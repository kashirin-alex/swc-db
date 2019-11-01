/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLog_h
#define swcdb_db_Files_CommitLog_h

#include "swcdb/lib/core/Time.h"
#include "CommitLogFragment.h"

namespace SWC { namespace Files { namespace CommitLog {


class Fragments {
  
  /* file-format(dir-structure): 
    ../log/{N}.frag
  */

  public:

  typedef Fragments*  Ptr;

  inline static Ptr make(const DB::RangeBase::Ptr& range){
    return new Fragments(range);
  }

  const DB::RangeBase::Ptr range;

  Fragments(const DB::RangeBase::Ptr& range) 
            : range(range), m_commiting(false), m_deleting(false),
              cfg_blk_sz(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.Range.block.size")), 
              cfg_blk_enc(Env::Config::settings()->get_ptr<gEnumExt>(
                "swc.rgr.Range.block.encoding")) {
    
    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);
    m_size_commit = schema->blk_size ? schema->blk_size : cfg_blk_sz->get();
    
    m_cells = DB::Cells::Mutable::make(
      1, 
      schema->cell_versions, 
      schema->cell_ttl, 
      schema->col_type
    );
  }

  Ptr ptr() {
    return this;
  }

  virtual ~Fragments() {
    //std::cout << " ~CommitLog::Fragments\n";
    wait_processing();
    free();
  }

  void add(const DB::Cells::Cell& cell) {
    m_cells->add(cell);

    std::lock_guard<std::mutex> lock(m_mutex);
    if(!m_deleting && !m_commiting && m_cells->size_bytes() >= m_size_commit) {
      m_commiting = true;
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=ptr()](){
          ptr->commit_new_fragment();
        }
      );
    }
  }

  void commit_new_fragment(bool finalize=false) {

    if(finalize && m_commiting){
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting && (commiting = true);});
    }

    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);
    uint32_t blk_size;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_size_commit = blk_size = schema->blk_size ? 
                      schema->blk_size : cfg_blk_sz->get();
    }
    auto blk_encoding = schema->blk_encoding != Types::Encoding::DEFAULT ?
                        schema->blk_encoding : 
                        (Types::Encoding)cfg_blk_enc->get();
    
    Fragment::Ptr frag; 
    uint32_t cell_count;
    int err;
    do {
      err = Error::OK;
      DynamicBuffer cells;
      cell_count = 0;
      DB::Cells::Interval intval;
      for(;;) {
        m_cells->write_and_free(cells, cell_count, intval, blk_size);
        if(cells.fill() >= blk_size)
          break;
        if(finalize && m_cells->size() == 0)
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(m_cells->size() == 0)
          break;
      }
      if(cells.fill() == 0)
        break;

      for(;;) {
        frag = Fragment::make(get_log_fragment(Time::now_ns()));
        frag->write(
          err, 
          schema->blk_replication, 
          blk_encoding, 
          intval, cells, cell_count
        );
        if(!err)
          break;
        // err == ? 
        // server already shutting down or major fs issue (PATH NOT FOUND)
        // write temp(local) file for recovery
      }

      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fragments.push_back(frag);
        if(m_deleting)
          break;
      }
    } while((finalize && m_cells->size() > 0 ) 
            || m_cells->size_bytes() >= blk_size);
    
    {
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_commiting = false;
    }
    m_cv.notify_all();
  }

  const std::string get_log_fragment(const int64_t frag) const {
    std::string s(range->get_path(DB::RangeBase::log_dir));
    s.append("/");
    s.append(std::to_string(frag));
    s.append(".frag");
    return s;
  }

  const std::string get_log_fragment(const std::string& frag) const {
    std::string s(range->get_path(DB::RangeBase::log_dir));
    s.append("/");
    s.append(frag);
    return s;
  }

  void load(int &err) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // fragments header OR log.data >> file.frag(intervals)

    err = Error::OK;
    FS::DirentList fragments;
    Env::FsInterface::interface()->readdir(
      err, range->get_path(DB::RangeBase::log_dir), fragments);
    if(err)
      return;

    Fragment::Ptr frag;
    for(auto entry : fragments) {
      frag = Fragment::make(get_log_fragment(entry.name));
      frag->load_header(true);
      m_fragments.push_back(frag);
    }
  }
  
  void load_cells(CellsBlock::Ptr cells_block) {
    // + ? whether a commit_new_fragment happened 
    m_cells->scan(cells_block->interval, cells_block->cells);
  }
  
  void load_cells(CellsBlock::Ptr cells_block, std::function<void(int)> cb) {
    //std::cout << "CommitLog::load_cells\n";
    if(m_commiting){
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting;});
    }
    
    std::vector<Fragment::Ptr>  fragments;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto& frag : m_fragments) {  
        if(frag->errored() || !frag->interval.includes(cells_block->interval))
          continue;
        fragments.push_back(frag);
      }
    }

    if(fragments.empty()){
      load_cells(cells_block);
      cb(Error::OK);
      return;
    }

    auto waiter = new AwaitingLoad(fragments.size(), cells_block, cb, ptr());
    for(auto& frag : fragments) {
      frag->processing++;
      frag->load([frag, waiter](int err){ waiter->processed(err, frag); });
    }
  }

  void get(std::vector<Fragment::Ptr>& fragments) {
    fragments.clear();
    
    std::lock_guard<std::mutex> lock(m_mutex);
    fragments.assign(m_fragments.begin(), m_fragments.end());
  }

  const size_t release(size_t bytes) {    
    size_t released = 0;
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto& frag : m_fragments) {
      if(!frag->loaded())
        continue;
      released += frag->release();
      if(released >= bytes)
        break;
    }
    return released;
  }

  void remove(int &err, std::vector<Fragment::Ptr>& fragments_old) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto it = m_fragments.begin(); it < m_fragments.end(); it++){
      if(std::find_if(fragments_old.begin(), fragments_old.end(), 
        [frag=*it](Fragment::Ptr& other){return frag->is_equal(other);})
        == fragments_old.end())
        continue;
      (*it)->remove(err);
      delete *it;
      m_fragments.erase(it);
    }
  }

  void remove(int &err) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_deleting = true;
    }

    if(m_commiting) {
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting;});
    }
    
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto frag : m_fragments)
        frag->remove(err);
    }

    wait_processing();

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      free();
    }
    m_cells->free();
  }

  bool deleting() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_deleting;
  }

  const size_t cells_count() {
    size_t count = m_cells->size();
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto frag : m_fragments)
      count += frag->cells_count();
    return count;
  }

  const size_t size_bytes() {
    size_t size = m_cells->size_bytes();

    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& frag : m_fragments)
      size += frag->size_bytes();
    return size;
  }

  const size_t processing() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _processing() + m_commiting.load();
  }

  void wait_processing() {
    while(processing() > 0)  {
      //std::cout << "wait_processing: " << to_string() << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  const std::string to_string() {
    size_t count = cells_count();
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CommitLog(count=");
    s.append(std::to_string(count));
    
    s.append(" cells=");
    s.append(m_cells->to_string());

    s.append(" fragments=");
    s.append(std::to_string(m_fragments.size()));

    s.append(" [");
    for(auto frag : m_fragments){
      s.append(frag->to_string());
      s.append(", ");
    }
    s.append("]");

    s.append(" processing=");
    s.append(std::to_string(_processing()));

    s.append(")");
    return s;
  }

  const gInt32tPtr            cfg_blk_sz;
  const gEnumExtPtr           cfg_blk_enc;

  private:
  
  
  void free() {  
    for(auto frag : m_fragments)
      delete frag;
    m_fragments.clear();
  }

  const size_t _processing() {
    size_t size = 0;
    for(auto& frag : m_fragments)
      size += frag->processing;
    return size;
  }


  struct AwaitingLoad {
    public:
    typedef std::function<void(int)>  Cb_t;
    
    AwaitingLoad(int32_t count, CellsBlock::Ptr cells_block, const Cb_t& cb,
                  Fragments::Ptr log) 
                : m_count(count), cells_block(cells_block), cb(cb), 
                  log(log) {
    }

    virtual ~AwaitingLoad() { }

    void processed(int err, Fragment::Ptr frag) {
      { 
        std::lock_guard<std::mutex> lock(m_mutex);
        --m_count;
        m_pending.push(frag);
        if(m_pending.size() > 1)
          return;
      }
      for(;;) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          frag = m_pending.front();
        }

        if(log->deleting())
          err = Error::COLUMN_MARKED_REMOVED;
        else
          frag->load_cells(cells_block);
        
        frag->processing--;
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_pending.pop();
          if(m_pending.empty()) 
            break;
        }
      }
      if(m_count == 0){
        log->load_cells(cells_block);
        cb(err);
        delete this;
      }
    }

    std::mutex                    m_mutex;
    int32_t                       m_count = 0;
    CellsBlock::Ptr               cells_block;
    const Cb_t                    cb;
    Fragments::Ptr                log;
    std::queue<Fragment::Ptr>     m_pending;
  };

  std::mutex                  m_mutex;

  DB::Cells::Mutable::Ptr     m_cells;
  uint32_t                    m_size_commit;
  std::atomic<bool>           m_commiting;
  bool                        m_deleting;
  
  std::condition_variable     m_cv;

  std::vector<Fragment::Ptr>  m_fragments;

};


}}}
#endif