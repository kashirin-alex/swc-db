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
            : m_range(range), m_commiting(false) {
    
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
    if(!m_commiting && m_cells->size_bytes() >= m_size_commit) {
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
    bool commiting;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(blk_size != 0) 
        m_size_commit = blk_size;
      else 
        blk_size = m_size_commit;
      commiting = m_commiting;
    }

    if(finalize && commiting){
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
      m_cells->write_and_free(cells, cell_count, intval, blk_size);
      if(cells.fill() == 0)
        break;

      frag = std::make_shared<Fragment>(
        get_log_fragment(Time::now_ns()));
      frag->write(
        err, 
        schema->blk_replication, 
        schema->blk_encoding, 
        intval, cells, cell_count
      );

      if(err) { 
        // server already shutting down or major fs issue (PATH NOT FOUND)
        // write temp(local) file for recovery
      } else {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_fragments.push_back(frag);
      }
    } while((finalize && m_cells->size() > 0 ) 
            || m_cells->size_bytes() >= m_size_commit);
   
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_commiting = false;
    }
    m_cv.notify_one();
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
  }
  
  void load_to_block(CellStore::Block::Read::Ptr blk, 
                     std::function<void(int)> cb) {
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
      blk->state = CellStore::Block::Read::State::LOGS_LOADED;
      load_cells(blk->interval, blk->cells);
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

  void remove(int &err) {}


  private:
  
  struct AwaitingLoad {
    public:
    typedef std::function<void(int)>      Cb_t;
    typedef std::shared_ptr<AwaitingLoad> Ptr;
    
    AwaitingLoad(int32_t count, CellStore::Block::Read::Ptr blk, 
                  const Cb_t& cb, Fragments::Ptr log) 
                : count(count), blk(blk), cb(cb), log(log) {
    }

    void processed(int err, Fragment::Ptr frag) {
      bool call;
      bool good;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        good = --count == 0;
        std::cout << "AwaitingLoad count="<<count << "\n";
        call = good || err;
        if(err) 
          count = 0;
      }
      if(!err) {
        frag->load_cells(blk->interval, blk->cells);
        if(good) {
          blk->state = CellStore::Block::Read::State::LOGS_LOADED;
          log->load_cells(blk->interval, blk->cells);
          blk->pending_logs_load();
        }
      }

      if(call)
        cb(err);
    }

    std::mutex                m_mutex;
    int32_t                   count = 0;
    DB::Cells::ReqScan::Ptr   req;
    CellStore::Block::Read::Ptr          blk;
    const Cb_t                cb;
    Fragments::Ptr            log;
  };

  std::mutex                  m_mutex;
  const DB::RangeBase::Ptr    m_range;

  DB::Cells::Mutable::Ptr     m_cells;
  uint32_t                    m_size_commit;
  bool                        m_commiting;
  
  std::condition_variable     m_cv;

  std::vector<Fragment::Ptr>  m_fragments;
  std::vector<Fragment::Ptr>  m_fragments_error;
};


}}}
#endif