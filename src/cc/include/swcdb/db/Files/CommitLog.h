/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLog_h
#define swcdb_db_Files_CommitLog_h

#include "swcdb/core/Time.h"
#include "swcdb/db/Files/CommitLogFragment.h"

namespace SWC { namespace Files { namespace CommitLog {


class Fragments final {
  
  /* file-format(dir-structure): 
    ../log/{N}.frag
  */

  public:

  typedef Fragments*  Ptr;

  DB::RangeBase::Ptr range;

  explicit Fragments() 
              : m_commiting(false), m_deleting(false),
                cfg_blk_sz(Env::Config::settings()->get_ptr<gInt32t>(
                  "swc.rgr.Range.block.size")), 
                cfg_blk_enc(Env::Config::settings()->get_ptr<gEnumExt>(
                  "swc.rgr.Range.block.encoding")) {
  }

  void init(DB::RangeBase::Ptr for_range) {
    range = for_range;

    HT_ASSERT(range != nullptr);
    DB::Schema::Ptr schema = Env::Schemas::get()->get(range->cid);
    m_size_commit = schema->blk_size ? schema->blk_size : cfg_blk_sz->get();
    
    m_cells = DB::Cells::Mutable(
      1, 
      schema->cell_versions, 
      schema->cell_ttl, 
      schema->col_type
    );
  }

  ~Fragments() { }

  Ptr ptr() {
    return this;
  }

  void add(const DB::Cells::Cell& cell) {
    size_t size_bytes;
    {
      std::scoped_lock lock(m_mutex_cells);
      m_cells.add(cell);
      size_bytes = m_cells.size_bytes;
    }

    if(!m_mutex.try_lock())
      return;

    if(!m_deleting && !m_commiting && size_bytes >= m_size_commit) {
      m_commiting = true;
      asio::post(
        *Env::IoCtx::io()->ptr(), 
        [ptr=ptr()](){ ptr->commit_new_fragment(); }
      );
    }
    m_mutex.unlock();
  }

  void commit_new_fragment(bool finalize=false) {
    DB::Schema::Ptr schema = Env::Schemas::get()->get(range->cid);
    uint32_t blk_size;
    {
      std::unique_lock lock_wait(m_mutex);
      if(finalize && m_commiting)
        m_cv.wait(lock_wait, [&commiting=m_commiting]
                             {return !commiting && (commiting = true);});
      m_size_commit = blk_size = schema->blk_size ? 
                      schema->blk_size : cfg_blk_sz->get();
    }

    auto blk_encoding = schema->blk_encoding != Types::Encoding::DEFAULT ?
                        schema->blk_encoding : 
                        (Types::Encoding)cfg_blk_enc->get();
    Fragment::Ptr frag; 
    uint32_t cell_count;
    int err;
    for(;;) {
      err = Error::OK;
      DynamicBuffer cells;
      cell_count = 0;
      frag = Fragment::make(
        get_log_fragment(Time::now_ns()), Fragment::State::WRITING);
      
      {
        std::scoped_lock lock(m_mutex);
        for(;;) {
          {
            std::scoped_lock lock2(m_mutex_cells);
            m_cells.write_and_free(
              cells, cell_count, frag->interval, blk_size);
          }
          if(cells.fill() >= blk_size)
            break;
          {
            std::shared_lock lock2(m_mutex_cells);
            if(!finalize && !m_cells.size)
              break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          {
            std::shared_lock lock2(m_mutex_cells);
            if(!m_cells.size)
              break;
          }
        }
        if(m_deleting || !cells.fill()) {
          delete frag;
          break;
        }
        m_fragments.push_back(frag);
      }
      
      frag->write(
        err, 
        schema->blk_replication, 
        blk_encoding, 
        cells, cell_count
      );

      {
        std::shared_lock lock(m_mutex_cells);
        if(!m_cells.size || (m_cells.size_bytes < blk_size && !finalize))
          break; 
      }   
    }
    
    {
      std::scoped_lock lock_wait(m_mutex);
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
    std::scoped_lock lock(m_mutex);
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
  
  void load_current_cells(int err, DB::Cells::Block::Ptr cells_block, 
                          int64_t after_ts = 0) {
    if(!err) {
      if(after_ts) {// + ? whether a commit_new_fragment happened
        load_cells(cells_block, after_ts);
        return;
      }
      std::shared_lock lock(m_mutex_cells);
      cells_block->load_cells(m_cells);
    }
    
    cells_block->loaded_logs(err);
  }
  
  void load_cells(DB::Cells::Block::Ptr cells_block, int64_t after_ts = 0) {
    {
      std::unique_lock lock_wait(m_mutex);
      if(m_commiting)
        m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting;});
    }

    int64_t ts;
    std::vector<Fragment::Ptr>  fragments;
    {
      std::shared_lock lock(m_mutex);
      ts = Time::now_ns();
      for(auto frag : m_fragments) {  
        if(after_ts < frag->ts && cells_block->is_consist(frag->interval))
          fragments.push_back(frag);
      }
    }
    if(fragments.empty()) {
      load_current_cells(Error::OK, cells_block);
      return;
    }
    //if(after_ts) {
    //  std::cout << " LOG::after_ts sz=" << fragments.size() << "\n";
    //}

    auto waiter = new AwaitingLoad(ts, fragments.size(), cells_block, ptr());
    for(auto frag : fragments)
      frag->load([frag, waiter](int err){ waiter->processed(err, frag); });
  }

  void get(std::vector<Fragment::Ptr>& fragments) {
    fragments.clear();
    
    std::shared_lock lock(m_mutex);
    fragments.assign(m_fragments.begin(), m_fragments.end());
  }

  const size_t release(size_t bytes) {   
    size_t released = 0;
    std::shared_lock lock(m_mutex);

    for(auto frag : m_fragments) {
      released += frag->release();
      if(bytes && released >= bytes)
        break;
    }
    return released;
  }

  void remove(int &err, std::vector<Fragment::Ptr>& fragments_old) {
    std::scoped_lock lock(m_mutex);

    for(auto old = fragments_old.begin(); old < fragments_old.end(); old++){
      for(auto it = m_fragments.begin(); it < m_fragments.end(); it++) {
        if(*it == *old) {
          (*it)->remove(err);
          delete *it;
          m_fragments.erase(it);
          break;
        }
      }
    }
  }

  void remove(int &err) {
    {
      std::unique_lock lock_wait(m_mutex);
      m_deleting = true;
      if(m_commiting)
        m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting;});
    }
    std::scoped_lock lock(m_mutex);
    for(auto frag : m_fragments) {
      frag->remove(err);
      delete frag;
    }
    m_fragments.clear();
    range = nullptr;
  }

  void unload() {
    {
      std::unique_lock lock_wait(m_mutex);
      if(m_commiting)
        m_cv.wait(lock_wait, [&commiting=m_commiting]{return !commiting;});
    }
    std::scoped_lock lock(m_mutex);
    for(auto frag : m_fragments)
      delete frag;
    m_fragments.clear();
    range = nullptr;
  }

  const bool deleting() {
    std::shared_lock lock(m_mutex);
    return m_deleting;
  }

  const size_t cells_count() {
    size_t count = 0;
    {
      std::shared_lock lock(m_mutex_cells);
      count += m_cells.size;
    }
    std::shared_lock lock(m_mutex);
    for(auto frag : m_fragments)
      count += frag->cells_count();
    return count;
  }

  const size_t size() {
    std::shared_lock lock(m_mutex);
    return m_fragments.size()+1;
  }

  const size_t size_bytes(bool only_loaded=false) {
    return _size_bytes(only_loaded);
  }

  const bool processing() {
    std::shared_lock lock(m_mutex);
    return _processing();
  }

  const std::string to_string() {
    size_t count = cells_count();
    std::shared_lock lock(m_mutex);

    std::string s("CommitLog(count=");
    s.append(std::to_string(count));
    
    s.append(" cells=");
    {
      std::shared_lock lock(m_mutex_cells);
      s.append(m_cells.to_string());
    }

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

    s.append(" used/actual=");
    s.append(std::to_string(_size_bytes(true)));
    s.append("/");
    s.append(std::to_string(_size_bytes()));

    s.append(")");
    return s;
  }

  const gInt32tPtr            cfg_blk_sz;
  const gEnumExtPtr           cfg_blk_enc;

  private:

  const bool _processing() {
    if(m_commiting)
      return true;
    for(auto frag : m_fragments)
      if(frag->processing())
        return true;
    return false;
  }

  const size_t _size_bytes(bool only_loaded=false) {
    size_t size = 0;
    {
      std::shared_lock lock(m_mutex_cells);
      size += m_cells.size_bytes;
    }
    for(auto frag : m_fragments)
      size += frag->size_bytes(only_loaded);
    return size;
  }

  struct AwaitingLoad final {
    public:
    
    AwaitingLoad(int64_t ts, int32_t count, DB::Cells::Block::Ptr cells_block, 
                  Fragments::Ptr log) 
                : ts(ts), m_count(count), cells_block(cells_block), log(log) {
    }

    ~AwaitingLoad() { }

    void processed(int err, Fragment::Ptr frag) {
      { 
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_count--;
        std::cout << " Fragments::AwaitingLoad count=" << m_count << "\n";
        m_pending.push(frag);
        if(m_pending.size() > 1)
          return;
      }
      for(;;) {
        {
          std::scoped_lock<std::mutex> lock(m_mutex);
          frag = m_pending.front();
        }

        frag->load_cells(cells_block);

        {
          std::scoped_lock<std::mutex> lock(m_mutex);
          m_pending.pop();
          if(m_pending.empty()) {
            if(m_count)
              return;
            break;
          }
        }
      }
      log->load_current_cells(err, cells_block, ts);
      delete this;
    }

    const int64_t                 ts;
    std::mutex                    m_mutex;
    int32_t                       m_count;
    DB::Cells::Block::Ptr         cells_block;
    Fragments::Ptr                log;
    std::queue<Fragment::Ptr>     m_pending;
  };

  std::shared_mutex           m_mutex_cells;
  DB::Cells::Mutable          m_cells;

  std::shared_mutex           m_mutex;
  uint32_t                    m_size_commit;
  bool                        m_commiting;
  bool                        m_deleting;
  std::condition_variable_any m_cv;
  std::vector<Fragment::Ptr>  m_fragments;

};


}}}
#endif