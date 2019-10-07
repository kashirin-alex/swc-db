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

  CommitLog(const DB::RangeBase::Ptr& range) : m_range(range) {
    m_size_commit = Env::Schemas::get()->get(range->cid)->blk_size;
    if(m_size_commit == 0)
      m_size_commit = 16000000; // cfg.default.blk.size
   }

  virtual ~CommitLog(){}

  void load(DB::Specs::Interval::Ptr spec, std::function<void()> cb) {
    /*
    m_commit_log->load(
      req->spec, 
      [cs_ptr, req, ptr=shared()](){ptr->scan(cs_ptr, 0, req);}
    );
    */
  }
  

  void add(DB::Cells::Cell& cell) {
    m_cells->add(cell);

    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_cells->size_bytes() >= m_size_commit) {
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=shared_from_this()](){
          ptr->commit_new_fragment();
        }
      );
    }
  }
  
  void commit_new_fragment() {
    DB::SchemaPtr schema = Env::Schemas::get()->get(m_range->cid);
    uint32_t blk_size = schema->blk_size;

    DynamicBuffer cells;
    uint32_t cell_count = 0;
    DB::Cells::Interval::Ptr intval = std::make_shared<DB::Cells::Interval>();

    {
      std::lock_guard<std::mutex> lock(m_mutex);

      if(blk_size != 0) {
        m_size_commit = blk_size;
        if(m_cells->size_bytes() < m_size_commit)
          return;
      }
      m_cells->write_and_free(cells, cell_count, intval);
    }

    Files::CommitLog::Fragment::Ptr frag 
      = std::make_shared<Files::CommitLog::Fragment>(
          get_log_fragment(Time::now_ns()));

    Types::Encoding encoder = schema->blk_encoding;
    int err;
    do {
      frag->write(err, encoder, intval, cells, cell_count);
    } while(err);
    
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_fragments.push_back(frag);
    }
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

      if(!err)
        m_fragments_error.push_back(frag);
      else
        m_fragments.push_back(frag);
    }
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CommitLog(");
    
    s.append(" fragments=[");
    for(auto frag : m_fragments){
      s.append(frag->to_string());
      s.append(",");
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
  
  std::mutex                  m_mutex;
  const DB::RangeBase::Ptr    m_range;

  DB::Cells::Mutable::Ptr     m_cells;
  uint32_t                    m_size_commit;

  std::vector<Files::CommitLog::Fragment::Ptr>  m_fragments;
  std::vector<Files::CommitLog::Fragment::Ptr>  m_fragments_error;
};


}}}
#endif