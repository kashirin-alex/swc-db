/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Range_h
#define swcdb_lib_db_Columns_Rgr_Range_h

#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Protocol/Rgr/req/RangeUnload.h"

#include "swcdb/lib/db/Cells/Mutable.h"
#include "swcdb/lib/db/Types/Range.h"
#include "swcdb/lib/db/Files/RangeData.h"
#include "swcdb/lib/db/Protocol/Common/req/Query.h"


namespace Query = SWC::Protocol::Common::Req::Query;

namespace SWC { namespace server { namespace Rgr {

class Range : public DB::RangeBase {

  public:
  
  typedef std::shared_ptr<Range>                    Ptr;

  struct ReqAdd {
    public:
    ReqAdd(const StaticBufferPtr& input, const ResponseCallbackPtr& cb) 
          : input(input), cb(cb) {}
    virtual ~ReqAdd() {}
    const StaticBufferPtr     input;
    const ResponseCallbackPtr cb;
  };
  enum State{
    NOTLOADED,
    LOADED,
    DELETED
  };

  const Types::Range  type;

  Range(const int64_t cid, const int64_t rid)
        : RangeBase(cid, rid), 
          m_state(State::NOTLOADED), 
          type(cid == 1 ? Types::Range::MASTER 
               :(cid == 2 ? Types::Range::META : Types::Range::DATA)),
          m_cellstores(std::make_shared<Files::CellStore::Readers>()) {
  }

  inline Ptr shared() {
    return std::dynamic_pointer_cast<Range>(shared_from_this());
  }

  inline static Ptr shared(const DB::RangeBase::Ptr& other){
    return std::dynamic_pointer_cast<Range>(other);
  }

  virtual ~Range(){}
  
  void set_state(State new_state) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = new_state;
  }

  bool is_loaded() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::LOADED;
  }

  bool deleted() { 
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::DELETED;
  }

  void get_next(size_t& cs_ptr, uint32_t& idx, 
                Files::CellStore::Read::Ptr& cs) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if(cs_ptr != (size_t)m_cellstores.get()) {
      cs_ptr = (size_t)m_cellstores.get();
      idx = 0;
    } 
    if(idx >= m_cellstores->size()) {
      cs = nullptr;
      return;
    } 
    cs = *(m_cellstores->begin()+(idx++));
  }

  void add(ReqAdd* req) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_q_adding.push(req);
    
    if(m_q_adding.size() == 1) {
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=shared()](){ ptr->run_add_queue(); }
      );
    }
  }

  void scan(DB::Cells::ReqScan::Ptr req) {
    size_t cs_ptr = 0;
    scan(cs_ptr, 0, req);
  }

  void scan(size_t cs_ptr, uint32_t idx, DB::Cells::ReqScan::Ptr req) {
    /*std::cout << "Range::Scan cid=" << cid 
              << " cs-idx=" << idx << " " 
              << req->spec->to_string() << "\n";*/
    
    Files::CellStore::Read::Ptr cs;
    for(;;) {
      get_next(cs_ptr, idx, cs);
      if(cs == nullptr)
        break;
      
      if(!cs->interval.includes(req->spec))
        continue;
      
      req->next_call = [cs_ptr, idx, req, ptr=shared()]() {
        ptr->scan(cs_ptr, idx, req);
      };
      cs->scan(req);
      return;
    }
    
    req->response(Error::OK);
  }

  void load(ResponseCallbackPtr cb) {
    bool is_loaded;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      is_loaded = m_state != State::NOTLOADED;
      m_state = State::LOADED;
    }
    int err = Env::RgrData::is_shuttingdown()
              ?Error::SERVER_SHUTTING_DOWN:Error::OK;
    if(is_loaded || err != Error::OK)
      return loaded(err, cb);

    HT_DEBUGF("LOADING RANGE %s", to_string().c_str());

    if(!Env::FsInterface::interface()->exists(err, get_path(""))){
      if(err != Error::OK)
        return loaded(err, cb);
      Env::FsInterface::interface()->mkdirs(err, get_path(log_dir));
      Env::FsInterface::interface()->mkdirs(err, get_path(cellstores_dir));
      if(err != Error::OK)
        return loaded(err, cb);
      
      take_ownership(err, cb);
    } else {
      last_rgr_chk(err, cb);
    }

  }

  void take_ownership(int &err, ResponseCallbackPtr cb) {
    if(err == Error::RS_DELETED_RANGE)
      return loaded(err, cb);

    Env::RgrData::get()->set_rgr(err, get_path(ranger_data_file));
    if(err != Error::OK)
      return loaded(err, cb);

    load(err, cb);
  }

  void on_change(int &err) { // change of range-interval
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if(type != Types::Range::MASTER) {
      uint8_t cid_typ = type == Types::Range::DATA ? 2 : 1;
      
      if(m_req_set_intval == nullptr) {
        DB::SchemaPtr schema = Env::Schemas::get()->get(cid_typ);
        if(schema == nullptr) {
          schema = Env::Clients::get()->schemas->get(err, cid_typ);
          if(err)
            return;
        }
        m_req_set_intval = std::make_shared<Query::Update>();
        m_req_set_intval->columns_cells->create(schema);
      }

      DB::Cells::Cell cell;
      cell.flag = DB::Cells::INSERT;
      cell.key.copy(m_interval.key_begin);
      cell.key.insert(0, std::to_string(cid));

      DB::Cell::Key key_end(m_interval.key_end);
      key_end.insert(0, std::to_string(cid));
        
      cell.own = true;
      cell.vlen = Serialization::encoded_length_vi64(rid) 
                + key_end.encoded_length();
      cell.value = new uint8_t[cell.vlen];
      uint8_t * ptr = cell.value;
      Serialization::encode_vi64(&ptr, rid);
      key_end.encode(&ptr);

      //m_req_set_intval->columns_cells->add(cid_typ, cell_del);
      m_req_set_intval->columns_cells->add(cid_typ, cell);
      m_req_set_intval->commit();
      m_req_set_intval->wait();
      err = m_req_set_intval->result->err;
      // INSERT master-range(col-1), key[cid+m_interval(data(cid)+key)], value[rid]
      // INSERT meta-range(col-2), key[cid+m_interval(key)], value[rid]
    } else {
      // update manager-root
    }
    // 
    Files::RangeData::save(err, shared_from_this(), m_cellstores);
    // 
  }

  void unload(Callback::RangeUnloaded_t cb, bool completely) {
    int err = Error::OK;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::DELETED){
        cb(err);
        return;
      }
    }

    if(m_commit_log != nullptr) 
      m_commit_log->commit_new_fragment(true);  
    m_cellstores->clear();
    // range.data (from compaction)


    
    if(completely) // whether to keep ranger_data_file
      Env::FsInterface::interface()->remove(err, get_path(ranger_data_file));


    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    set_state(State::NOTLOADED);
    
    HT_INFOF("UNLOADED RANGE cid=%d rid=%d err=%d(%s)", 
              cid, rid, err, Error::get_text(err));
    cb(err);
  }

  void remove(int &err) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = State::DELETED;
  
      for(auto& cs : *m_cellstores.get()){
        cs->remove(err);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      Env::FsInterface::interface()->rmdir(err, get_path(""));
    }
    HT_INFOF("REMOVED RANGE %s", to_string().c_str());
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string s("(");
    s.append(DB::RangeBase::to_string());
    s.append(" state=");
    s.append(std::to_string(m_state));
    
    s.append(" type=");
    s.append(Types::to_string(type));

    s.append(" ");
    s.append(m_interval.to_string());

    if(m_commit_log != nullptr) {
      s.append(" ");
      s.append(m_commit_log->to_string());
    }

    s.append(" cellstores=[");
    for(auto& cs : *m_cellstores.get()) {
      s.append(cs->to_string());
      s.append(", ");
    }
    s.append("] ");
    s.append(")");
    return s;
  }

  private:
  
  void loaded(int &err, ResponseCallbackPtr cb) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::DELETED)
        err = Error::RS_DELETED_RANGE;
    }
    cb->response(err);
  }

  void last_rgr_chk(int &err, ResponseCallbackPtr cb) {
    // ranger.data
    Files::RgrDataPtr rs_data = Env::RgrData::get();
    Files::RgrDataPtr rs_last = get_last_rgr(err);

    if(rs_last->endpoints.size() > 0 
      && !has_endpoint(rs_data->endpoints, rs_last->endpoints)){
      HT_DEBUGF("RANGER-LAST=%s RANGER-NEW=%s", 
                rs_last->to_string().c_str(), rs_data->to_string().c_str());
                
      Env::Clients::get()->rgr->get(rs_last->endpoints)->put(
        std::make_shared<Protocol::Rgr::Req::RangeUnload>(shared_from_this(), cb));
      return;
    }

    take_ownership(err, cb);
  }

  void load(int &err, ResponseCallbackPtr cb) {
    bool is_initial_column_range = false;
    Files::RangeData::load(err, shared_from_this(), m_cellstores);
    if(err) 
      (void)err;
      //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

    else if(m_cellstores->empty()) {
      // init 1st cs(for log_cells)
      DB::SchemaPtr schema = Env::Schemas::get()->get(cid);
      Files::CellStore::Read::Ptr cs 
        = Files::CellStore::create_init_read(
          err, schema->blk_encoding, shared_from_this());
      if(!err) {
        m_cellstores->push_back(cs);
        is_initial_column_range = true;
      }
    }
 
    if(!err) {
      m_commit_log = Files::CommitLog::Fragments::make(shared_from_this());
      m_commit_log->load(err);

      if(!err) {
        m_interval.free();
        for(auto& cs: *m_cellstores.get()) {
          m_interval.expand(cs->interval);
          cs->set(m_commit_log);
        }
        if(is_initial_column_range)
          on_change(err);
      }
    }

    loaded(err, cb); // RSP-LOAD-ACK

    if(err == Error::OK) {
      set_state(State::LOADED);
      if(is_loaded()) {
        HT_INFOF("LOADED RANGE %s", to_string().c_str());
        return;
      }
    }
    HT_WARNF("LOAD RANGE FAILED err=%d(%s) %s", 
             err, Error::get_text(err), to_string().c_str());
    return; 
  }

  void run_add_queue() {
    ReqAdd* req;

    int err;
    DB::Cells::Cell cell;
    const uint8_t* ptr;
    size_t remain;
    size_t cs_ptr;
    uint32_t cs_idx;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      cs_ptr = (size_t)m_cellstores.get();
    }
    
    Files::CellStore::Read::Ptr cs;

    for(;;) {
      err = Error::OK;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        req = m_q_adding.front();
      }

      std::cout << "Range::add cid="<<cid 
               << " sz=" << req->input->size << "\n";
      uint32_t count = 0;
      ptr = req->input->base;
      remain = req->input->size; 

      while(remain) {
        cell.read(&ptr, &remain);
        if(!m_interval.is_in_end(cell.key)) {
          // validate over range.interval match skip( with error)
          continue;
        }
        
        if(!(cell.control & DB::Cells::HAVE_TIMESTAMP)) {
          cell.set_timestamp(Time::now_ns());
          if(cell.control & DB::Cells::AUTO_TIMESTAMP)
            cell.control ^= DB::Cells::AUTO_TIMESTAMP;
        }
        if(!(cell.control & DB::Cells::HAVE_REVISION))
          cell.control |= DB::Cells::REV_IS_TS;

        count++;
        m_commit_log->add(cell);
      //std::cout << " " << cell.to_string() << "\n";

        cs_idx = 0;
        for(;;) {
          get_next(cs_ptr, cs_idx, cs);
          if(cs == nullptr)
            break;
          if(cs->add_logged(cell)) {
            if(!cell.on_fraction)
              break;
          }
        }
      }
      std::cout << "Range::added cid=" << cid 
               << " count=" << count << "\n";

      req->cb->response(err);

      delete req;
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_q_adding.pop();
        if(m_q_adding.empty())
          break;
      }
    }

  }

  State                             m_state;
   
  Files::CellStore::ReadersPtr      m_cellstores;
  std::queue<ReqAdd*>               m_q_adding;

  Files::CommitLog::Fragments::Ptr  m_commit_log;
  Query::Update::Ptr                m_req_set_intval;
};



}} // server namespace


void Protocol::Rgr::Req::RangeUnload::unloaded(int err, ResponseCallbackPtr cb) {
  server::Rgr::Range::shared(range)->take_ownership(err, cb);
}
bool Protocol::Rgr::Req::RangeUnload::valid() {
  return !server::Rgr::Range::shared(range)->deleted();
}

}
#endif