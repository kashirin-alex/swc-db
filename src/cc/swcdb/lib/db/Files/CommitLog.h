/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLog_h
#define swcdb_db_Files_CommitLog_h

#include "swcdb/lib/core/Time.h"
#include "CommitLogFragment.h"

namespace SWC { namespace Files { 
  
namespace CommitLog {


class Fragments: public std::enable_shared_from_this<Fragments> {
  
  /* file-format(dir-structure): 
    ../log/{N}.frag
  */

  public:

  typedef std::shared_ptr<Fragments>  Ptr;

  inline static Ptr make(const DB::RangeBase::Ptr& range){
    return std::make_shared<Fragments>(range);
  }

  Fragments(const DB::RangeBase::Ptr& range) 
            : m_range(range), m_commiting(false), m_deleting(false) {
    
    DB::SchemaPtr schema = Env::Schemas::get()->get(m_range->cid);

    m_size_commit = schema->blk_size;
    if(m_size_commit == 0)
      m_size_commit = 16000000; // cfg.default.blk.size
    
    m_cells = DB::Cells::Mutable::make(
      1, 
      schema->cell_versions, 
      schema->cell_ttl, 
      schema->col_type
    );

  }

  virtual ~Fragments(){}

  void add(DB::Cells::Cell& cell) {
    m_cells->add(cell);

    std::lock_guard<std::mutex> lock(m_mutex);
    if(!m_deleting && !m_commiting && m_cells->size_bytes() >= m_size_commit) {
      m_commiting = true;
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=shared_from_this()](){
          ptr->commit_new_fragment();
        }
      );
    }
  }

  void commit_new_fragment(bool finalize=false) {
    DB::SchemaPtr schema = Env::Schemas::get()->get(m_range->cid);
    uint32_t blk_size = schema->blk_size;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(blk_size != 0) 
        m_size_commit = blk_size;
      else 
        blk_size = m_size_commit;
    }

    if(finalize && m_commiting){
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting && (commiting = true);});
    }
    
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

      frag = std::make_shared<Fragment>(get_log_fragment(Time::now_ns()));
      frag->write(
        err, 
        schema->blk_replication, 
        schema->blk_encoding, 
        intval, cells, cell_count
      );

      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(err) { 
          m_fragments_error.push_back(frag);
          // server already shutting down or major fs issue (PATH NOT FOUND)
          // write temp(local) file for recovery
        } else {
          m_fragments.push_back(frag);
        }
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
    std::string s(m_range->get_path(DB::RangeBase::log_dir));
    s.append("/");
    s.append(std::to_string(frag));
    s.append(".frag");
    return s;
  }

  const std::string get_log_fragment(const std::string& frag) const {
    std::string s(m_range->get_path(DB::RangeBase::log_dir));
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
      err, m_range->get_path(DB::RangeBase::log_dir), fragments);
    if(err)
      return;

    Fragment::Ptr frag;
    for(auto entry : fragments) {
      frag = std::make_shared<Fragment>(
        get_log_fragment(entry.name));
      frag->load_header(err, true);

      if(err)
        m_fragments_error.push_back(frag);
      else
        m_fragments.push_back(frag);
    }
    
    if(m_fragments_error.size() > 0) {
      std::cerr << "Error-Fragments: \n";
      for(auto frag : m_fragments_error)
        std::cerr << " " << frag->to_string() << "\n";
      exit(1);
    }
  }
  
  void load_to_block(CellStore::Block::Read::Ptr blk, 
                     std::function<void(int)> cb) {
    if(m_commiting){
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting;});
    }
    
    std::vector<Fragment::Ptr>  fragments;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto& frag : m_fragments) {  
        if(!frag->interval.includes(blk->interval))
          continue;
        fragments.push_back(frag);
      }
    }

    if(fragments.empty()){
      load_cells(blk->interval, blk->cells);
      blk->state = CellStore::Block::Read::State::LOGS_LOADED;
      blk->pending_logs_load();
      cb(Error::OK);
      return;
    }

    AwaitingLoad::Ptr state = std::make_shared<AwaitingLoad>(
      fragments.size(), blk, cb, shared_from_this());
    for(auto& frag : fragments) 
      frag->load([frag, state](int err){ state->processed(err, frag); });
  }

  void load_cells(const DB::Cells::Interval& intval, 
                  DB::Cells::Mutable& cells) {
    // + ? whether a commit_new_fragment happened 
    m_cells->scan(intval, cells);
  }

  size_t cells_count() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t count = 0;
    for(auto frag : m_fragments)
      count += frag->cells_count();
    return count;
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
    s.append("] errors=[");
    for(auto frag : m_fragments_error){
      s.append(frag->to_string());
      s.append(",");
    }
    s.append("]");

    s.append(")");
    return s;
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
      m_fragments.clear();
      m_fragments_error.clear();
    }
    
    m_cells->free();
  }

  bool deleting() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_deleting;
  }

  private:
  
  struct AwaitingLoad {
    public:
    typedef std::function<void(int)>      Cb_t;
    typedef std::shared_ptr<AwaitingLoad> Ptr;
    
    AwaitingLoad(int32_t count, CellStore::Block::Read::Ptr blk, 
                  const Cb_t& cb, Fragments::Ptr log) 
                : m_count(count), blk(blk), cb(cb), log(log) {
    }

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
          frag->load_cells(blk->interval, blk->cells);
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_pending.pop();
          if(m_pending.empty()) 
            break;
        }
      }
      if(m_count == 0) {
        log->load_cells(blk->interval, blk->cells);
        blk->state = CellStore::Block::Read::State::LOGS_LOADED;
        blk->pending_logs_load();
        std::cout << "AwaitingLoad CALL\n";
        cb(err);
      }
    }

    std::mutex                    m_mutex;
    int32_t                       m_count = 0;
    std::queue<Fragment::Ptr>     m_pending;
    CellStore::Block::Read::Ptr   blk;
    const Cb_t                    cb;
    Fragments::Ptr                log;
  };

  std::mutex                  m_mutex;
  const DB::RangeBase::Ptr    m_range;

  DB::Cells::Mutable::Ptr     m_cells;
  uint32_t                    m_size_commit;
  std::atomic<bool>           m_commiting;
  bool                        m_deleting;
  
  std::condition_variable     m_cv;

  std::vector<Fragment::Ptr>  m_fragments;
  std::vector<Fragment::Ptr>  m_fragments_error;
};


}}}
#endif