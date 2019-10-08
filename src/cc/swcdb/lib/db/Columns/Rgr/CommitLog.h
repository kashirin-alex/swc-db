/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_CommitLog_h
#define swcdb_lib_db_Columns_Rgr_CommitLog_h

#include "swcdb/lib/core/Time.h"
#include "swcdb/lib/db/Files/CommitLogFragment.h"

namespace SWC { namespace server { namespace Rgr {


class CommitLog: public std::enable_shared_from_this<CommitLog> {
  public:

  typedef std::shared_ptr<CommitLog>  Ptr;

  inline static Ptr make(const DB::RangeBase::Ptr& range){
    return std::make_shared<CommitLog>(range);
  }

  CommitLog(const DB::RangeBase::Ptr& range) 
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

  virtual ~CommitLog(){}

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
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(blk_size != 0) 
        m_size_commit = blk_size;
      else 
        blk_size = m_size_commit;
    }
    
    do {
      DynamicBuffer cells;
      uint32_t cell_count = 0;
      DB::Cells::Interval::Ptr intval = std::make_shared<DB::Cells::Interval>();
      m_cells->write_and_free(cells, cell_count, intval, blk_size);
      if(cells.fill() == 0){
        std::lock_guard<std::mutex> lock(m_mutex);
        m_commiting = false;
        return;
      }

      Files::CommitLog::Fragment::Ptr frag 
        = std::make_shared<Files::CommitLog::Fragment>(
            get_log_fragment(Time::now_ns()));
      int err;
      frag->write(
        err, 
        schema->blk_replication, 
        schema->blk_encoding, 
        intval, cells, cell_count
      );
      if(err){ 
        // server already shutting down or major fs issue (PATH NOT FOUND)
        // write temp(local) file for recovery
      }
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!err)
          m_fragments.push_back(frag);
      }
    } while((finalize && m_cells->size() > 0 ) 
            || m_cells->size_bytes() >= m_size_commit);
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_commiting = false;
  }

  const std::string get_log_fragment(int64_t frag) {
    std::string s(m_range->get_path(DB::RangeBase::log_dir));
    s.append("/");
    s.append(std::to_string(frag));
    s.append(".frag");
    return s;
  }

  const std::string get_log_fragment(const std::string& frag) {
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

    Files::CommitLog::Fragment::Ptr frag;
    for(auto entry : fragments) {
      frag = std::make_shared<Files::CommitLog::Fragment>(
        get_log_fragment(entry.name));
      frag->load_header(err, true);

      if(err)
        m_fragments_error.push_back(frag);
      else
        m_fragments.push_back(frag);
    }
  }
  
  void load(DB::Specs::Interval::Ptr spec, 
            Files::CellStore::ReadersPtr cellstores,
            std::function<void(int)> cb) {
    std::vector<Files::CommitLog::Fragment::Ptr>  fragments;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      for(auto& frag : m_fragments) {  
        if(!frag->loaded() && frag->interval->includes(spec))
          fragments.push_back(frag);
      }
    }
    if(fragments.empty()){
      cb(Error::OK);
      return;
    }
    std::shared_ptr<AwatingLoad> state = std::make_shared<AwatingLoad>();
    state->count = fragments.size();
    for(auto& frag : fragments){
      frag->load_cells(
        cellstores, [state, cb](int err){
          if(!state->more()) cb(err);
        }
      );
    }
  }

  size_t cells_count() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t count = 0;
    for(auto frag : m_fragments)
      count+= frag->cells_count();
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
  
  struct AwatingLoad {
    std::mutex m_mutex;
    uint32_t count = 0;
    
    bool more() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return --count;
    }
  };

  std::mutex                  m_mutex;
  const DB::RangeBase::Ptr    m_range;

  DB::Cells::Mutable::Ptr     m_cells;
  uint32_t                    m_size_commit;
  bool                        m_commiting;

  std::vector<Files::CommitLog::Fragment::Ptr>  m_fragments;
  std::vector<Files::CommitLog::Fragment::Ptr>  m_fragments_error;
};


}}}
#endif