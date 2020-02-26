
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Query_h
#define swc_lib_db_protocol_common_req_Query_h

#include "swcdb/db/Cells/Vector.h"

#include "swcdb/db/Protocol/Common/req/QueryUpdate.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQuerySelect.h"

namespace SWC { namespace Protocol { namespace Common { namespace Req { 
  
namespace Query {
 
namespace Result{

struct Select final {
  typedef std::shared_ptr<Select> Ptr;

  std::atomic<uint32_t>  completion = 0;
  std::atomic<int>       err = Error::OK;
  std::atomic<bool>      notify;

  Select(std::condition_variable& cv, bool notify) 
        : notify(notify), m_cv(cv) { 
  }

  ~Select() { }

  struct Rsp final {
    public:
    typedef std::shared_ptr<Rsp> Ptr;

    Rsp() { }

    ~Rsp() { }

    const bool add_cells(const StaticBuffer& buffer, bool reached_limit, 
                         DB::Specs::Interval& interval) {
      std::scoped_lock lock(m_mutex);
      size_t recved = m_vec.add(buffer.base, buffer.size);
      m_counted += recved;
      m_size_bytes += buffer.size;

      if(interval.flags.limit) {
        if(interval.flags.limit <= recved) {
          interval.flags.limit = 0;
          return false;
        } 
        interval.flags.limit -= recved;
      }

      if(reached_limit) {
        auto last = m_vec.cells.back();
        interval.offset_key.copy(last->key);
        interval.offset_rev = last->get_revision();
      }
      return true;
    }  
    
    void get_cells(DB::Cells::Vector& vec) {
      std::scoped_lock lock(m_mutex);
      vec.add(m_vec);
      m_vec.cells.clear();
      m_size_bytes = 0;
    }

    const size_t get_size() {
      std::scoped_lock lock(m_mutex);
      return m_counted;
    }

    const size_t get_size_bytes() {
      std::scoped_lock lock(m_mutex);
      return m_size_bytes;
    }

    void free() {
      std::scoped_lock lock(m_mutex);
      m_vec.free();
      m_size_bytes = 0;
    }
  
    private:
    std::mutex         m_mutex;
    DB::Cells::Vector  m_vec;
    size_t             m_counted = 0;
    size_t             m_size_bytes = 0;
  };


  void add_column(const int64_t cid) {
    m_columns.emplace(cid, std::make_shared<Rsp>());
  }
  
  const bool add_cells(const int64_t cid, const StaticBuffer& buffer, 
                       bool reached_limit, DB::Specs::Interval& interval) {
    return m_columns[cid]->add_cells(buffer, reached_limit, interval);
  }

  void get_cells(const int64_t cid, DB::Cells::Vector& cells) {
    m_columns[cid]->get_cells(cells);
    if(notify)
      m_cv.notify_all();
  }

  const size_t get_size(const int64_t cid) {
    return m_columns[cid]->get_size();
  }

  const size_t get_size_bytes() {
    size_t sz = 0;
    for(const auto& col : m_columns)
      sz += col.second->get_size_bytes();
    return sz;
  }

  const std::vector<int64_t> get_cids() const {
    std::vector<int64_t> list(m_columns.size());
    int i = 0;
    for(const auto& col : m_columns)
      list[i++] = col.first;
    return list;
  }

  const void free(const int64_t cid) {
    m_columns[cid]->free();
    if(notify)
      m_cv.notify_all();
  }

  void remove(const int64_t cid) {
    m_columns.erase(cid);
  }
  
  private:

  std::unordered_map<int64_t, Rsp::Ptr> m_columns;
  std::condition_variable&              m_cv;

};

}

class Select : public std::enable_shared_from_this<Select> {
  public:

  using Result = Result::Select;

  typedef std::shared_ptr<Select>           Ptr;
  typedef std::function<void(Result::Ptr)>  Cb_t;
  
  uint32_t buff_sz          = 8000000;
  uint32_t partial_rate     = 3;
  uint32_t timeout_select   = 600000;

  Cb_t                        cb;
  DB::Specs::Scan             specs;
  Result::Ptr                 result;

  Select(Cb_t cb=0, bool rsp_partials=false)
        : cb(cb), 
          rsp_partials(cb && rsp_partials), 
          result(std::make_shared<Result>(m_cv, rsp_partials)) { 
  }

  Select(const DB::Specs::Scan& specs, Cb_t cb=0, bool rsp_partials=false)
        : cb(cb), specs(specs), 
          rsp_partials(cb && rsp_partials),
          result(std::make_shared<Result>(m_cv, rsp_partials)) {
  }

  virtual ~Select() { 
    result->notify.store(false);
  }
 
  void response(int err=Error::OK) {
    if(err)
      result->err = err;
    // if cells not empty commit-again
    if(cb)
      cb(result);

    m_cv.notify_all();
  }

  void response_partials() {
    if(!rsp_partials)
      return;
    
    response_partial();
    
    if(!wait_on_partials())
      return;

    std::unique_lock<std::mutex> lock(m_mutex);
    if(!m_rsp_partial_runs)
      return;
    m_cv.wait(
      lock, 
      [selector=shared_from_this()] () { 
        return !selector->m_rsp_partial_runs || !selector->wait_on_partials();
      }
    );  
  }

  const bool wait_on_partials() const {
    return result->get_size_bytes() > buff_sz * partial_rate;
  } 

  void response_partial() {
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      if(m_rsp_partial_runs)
        return;
      m_rsp_partial_runs = true;
    }
    
    cb(result);

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_rsp_partial_runs = false;
    }
    m_cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(
      lock, 
      [selector=shared_from_this()] () {
        return selector->result->completion == 0 && 
              !selector->m_rsp_partial_runs;
      }
    );  
  }

  void scan() {
    for(auto &col : specs.columns)
      result->add_column(col->cid);
    
    for(auto &col : specs.columns) {
      for(auto &intval : col->intervals) {
        std::make_shared<ScannerColumn>(
          col->cid, *intval.get(), shared_from_this()
        )->run();
      }
    }
  }

  private:
  
  const bool                  rsp_partials;
  std::mutex                  m_mutex;
  bool                        m_rsp_partial_runs = false;
  std::condition_variable     m_cv;


  class ScannerColumn : public std::enable_shared_from_this<ScannerColumn> {
    public:

    typedef std::shared_ptr<ScannerColumn>  Ptr;
    const int64_t                           cid;
    DB::Specs::Interval                     interval;
    Select::Ptr                             selector;

    ScannerColumn(const int64_t cid, DB::Specs::Interval& interval,
                  Select::Ptr selector)
                  : cid(cid), interval(interval), selector(selector) {
    }

    virtual ~ScannerColumn() { }

    void run() {
      std::make_shared<Scanner>(
        Types::Range::MASTER, cid, shared_from_this()
      )->locate_on_manager(false);
    }

    const bool add_cells(const StaticBuffer& buffer, bool reached_limit) {
      return selector->result->add_cells(cid, buffer, reached_limit, interval);
    }

    void next_call(bool final=false) {
      if(next_calls.empty()) {
        if(final && !selector->result->completion)
          selector->response(Error::OK);
        return;
      }
      interval.offset_key.free();
      interval.offset_rev = 0;

      auto call = next_calls.back();
      next_calls.pop_back();
      call();
    }

    void add_call(const std::function<void()>& call) {    
      next_calls.push_back(call);
    }

    const std::string to_string() {
      std::string s("ScannerColumn(");
      s.append("cid=");
      s.append(std::to_string(cid));
      s.append(" ");
      s.append(interval.to_string());
      s.append(" completion=");
      s.append(std::to_string(selector->result->completion.load()));
      s.append(" next_calls=");
      s.append(std::to_string(next_calls.size()));
      s.append(")");
      return s;
    }

    private:

    std::vector<std::function<void()>> next_calls;

  };

  class Scanner: public std::enable_shared_from_this<Scanner> {
    public:
    const Types::Range        type;
    const int64_t             cid;
    ScannerColumn::Ptr        col;

    ReqBase::Ptr              parent;
    const int64_t             rid;
    DB::Cell::Key             range_offset;

    Scanner(const Types::Range type, const int64_t cid, ScannerColumn::Ptr col,
            ReqBase::Ptr parent=nullptr, 
            const DB::Cell::Key* range_offset=nullptr, const int64_t rid=0)
          : type(type), cid(cid), col(col), 
            parent(parent), 
            range_offset(range_offset ? *range_offset : DB::Cell::Key()), 
            rid(rid) {
    }

    virtual ~Scanner() {}

    const std::string to_string() {
      std::string s("Scanner(type=");
      s.append(Types::to_string(type));
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));
      s.append(" RangeOffset");
      s.append(range_offset.to_string());
      s.append(" ");
      s.append(col->to_string());
      s.append(")");
      return s;
    }
    
    void locate_on_manager(bool next_range=false) {
      col->selector->result->completion++;

      Mngr::Params::RgrGetReq params(1, 0, next_range);

      if(!range_offset.empty()) {
        params.range_begin.copy(range_offset);
        col->interval.apply_possible_range_end(params.range_end); 
      } else {
        col->interval.apply_possible_range(
          params.range_begin, params.range_end);
      }

      if(cid > 2)
        params.range_begin.insert(0, std::to_string(cid));
      if(cid >= 2)
        params.range_begin.insert(0, "2");
      
      if(cid > 2) 
        params.range_end.insert(0, std::to_string(cid));
      if(cid >= 2) 
        params.range_end.insert(0, "2");

      SWC_LOGF(LOG_INFO, "LocateRange-onMngr %s", params.to_string().c_str());

      Mngr::Req::RgrGet::request(
        params,
        [next_range, scanner=shared_from_this()]
        (ReqBase::Ptr req, Mngr::Params::RgrGetRsp rsp) {
          if(scanner->located_on_manager(req, rsp, next_range))
            scanner->col->selector->result->completion--;
        }
      );
    }

    void resolve_on_manager() {

      auto req = Mngr::Req::RgrGet::make(
        Mngr::Params::RgrGetReq(cid, rid),
        [scanner=shared_from_this()]
        (ReqBase::Ptr req, Mngr::Params::RgrGetRsp rsp) {
          if(scanner->located_on_manager(req, rsp))
            scanner->col->selector->result->completion--;
        }
      );
      if(cid != 1) {
        Mngr::Params::RgrGetRsp rsp(cid, rid);
        if(Env::Clients::get()->rangers.get(cid, rid, rsp.endpoints)) {
          SWC_LOGF(LOG_INFO, "Cache hit %s", rsp.to_string().c_str());
          if(proceed_on_ranger(req, rsp))
            return; 
          Env::Clients::get()->rangers.remove(cid, rid);
        } else
          SWC_LOGF(LOG_INFO, "Cache miss %s", rsp.to_string().c_str());
      }
      col->selector->result->completion++;
      req->run();
    }

    bool located_on_manager(const ReqBase::Ptr& base, 
                            const Mngr::Params::RgrGetRsp& rsp, 
                            bool next_range=false) {
      SWC_LOGF(LOG_INFO, "LocatedRange-onMngr %s", rsp.to_string().c_str());

      if(rsp.err) {
        if(rsp.err == Error::COLUMN_NOT_EXISTS) {
          col->selector->response(rsp.err);
          return true;
        }
        if(next_range && rsp.err == Error::RANGE_NOT_FOUND) {
          col->selector->result->completion--;
          col->next_call(true);
          return false;
        }

        SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING %s", 
                              rsp.to_string().c_str());
        if(rsp.err == Error::RANGE_NOT_FOUND) {
          (parent == nullptr ? base : parent)->request_again();
        } else {
          base->request_again();
        }
        return false;
      }
      if(!rsp.rid) {
        SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING(no rid) %s", 
                            rsp.to_string().c_str());
        (parent == nullptr ? base : parent)->request_again();
        return false;
      }

      if(type == Types::Range::MASTER) {
        col->add_call(
          [scanner=std::make_shared<Scanner>(
            type, cid, col,
            parent == nullptr ? base : parent, &rsp.range_begin
          )] () { scanner->locate_on_manager(true); }
        );
      } else if(rsp.cid != 1) {
        Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);
      }

      return proceed_on_ranger(base, rsp);
    }
    
    bool proceed_on_ranger(const ReqBase::Ptr& base, 
                           const Mngr::Params::RgrGetRsp& rsp) {
      if(type == Types::Range::DATA || 
        (type == Types::Range::MASTER && col->cid == 1) ||
        (type == Types::Range::META   && col->cid == 2 )) {

        if(cid != rsp.cid || col->cid != cid) {
          SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING(cid no match) %s", 
                                rsp.to_string().c_str());
          (parent == nullptr ? base : parent)->request_again();
          return false;
          //col->selector->response(Error::NOT_ALLOWED);
          //return true;
        }
        select(rsp.endpoints, rsp.rid, base);

      } else {
        std::make_shared<Scanner>(
          type, rsp.cid, col,
          base, nullptr, rsp.rid
        )->locate_on_ranger(rsp.endpoints, false);
      }
      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints, bool next_range=false) {
      col->selector->result->completion++;

      Rgr::Params::RangeLocateReq params(cid, rid);
      if(next_range) {
        params.flags |= Rgr::Params::RangeLocateReq::NEXT_RANGE;
        params.range_offset.copy(range_offset);
        params.range_offset.insert(0, std::to_string(col->cid));
        if(type == Types::Range::MASTER && col->cid > 2)
          params.range_offset.insert(0, "2");
      }

      col->interval.apply_possible_range(params.range_begin, params.range_end);
      params.range_begin.insert(0, std::to_string(col->cid));
      if(type == Types::Range::MASTER && col->cid > 2)
        params.range_begin.insert(0, "2");
      params.range_end.insert(0, std::to_string(col->cid));
      if(type == Types::Range::MASTER && col->cid > 2)
        params.range_end.insert(0, "2");
      
      SWC_LOGF(LOG_INFO, "LocateRange-onRgr %s", params.to_string().c_str());

      Rgr::Req::RangeLocate::request(
        params, endpoints, 
        [scanner=shared_from_this()]() {
          SWC_LOG(LOG_DEBUG, "LocateRange RETRYING no-conn");
          scanner->parent->request_again();
        },
        [endpoints, next_range, scanner=shared_from_this()] 
        (ReqBase::Ptr req, Rgr::Params::RangeLocateRsp rsp) {
          if(scanner->located_on_ranger(endpoints, req, rsp, next_range))
            scanner->col->selector->result->completion--;
        }
      );
    }

    bool located_on_ranger(const EndPoints& endpoints, 
                           const ReqBase::Ptr& base, 
                           const Rgr::Params::RangeLocateRsp& rsp, 
                           bool next_range=false) {
      SWC_LOGF(LOG_INFO, "Located-onRgr %s", rsp.to_string().c_str());
      if(rsp.err) {
        if(rsp.err == Error::RANGE_NOT_FOUND && 
           (next_range || type == Types::Range::META)) {
          col->selector->result->completion--;
          col->next_call(true);
          return false;
        }

        SWC_LOGF(LOG_DEBUG, "Located-oRgr RETRYING %s", 
                            rsp.to_string().c_str());                      
        if(rsp.err == Error::RS_NOT_LOADED_RANGE || 
           rsp.err == Error::RANGE_NOT_FOUND ) {
          Env::Clients::get()->rangers.remove(cid, rid);
          parent->request_again();
        } else {
          base->request_again();
        }
        return false;
      }
      if(!rsp.rid) {
        SWC_LOGF(LOG_DEBUG, "Located-onRgr RETRYING(no rid) %s", 
                            rsp.to_string().c_str());
        Env::Clients::get()->rangers.remove(cid, rid);
        parent->request_again();
        return false;
      }
      if(type == Types::Range::DATA && rsp.cid != col->cid) {
        SWC_LOGF(LOG_DEBUG, "Located-onRgr RETRYING(cid no match) %s",
                 rsp.to_string().c_str());        
        Env::Clients::get()->rangers.remove(cid, rid);
        parent->request_again();
        return false;
      }

      if(type != Types::Range::DATA) {
        col->add_call([endpoints, scanner=std::make_shared<Scanner>(
          type, cid, col,
          parent, &rsp.range_begin, rid
          )] () { scanner->locate_on_ranger(endpoints, true); }
        );
      }

      std::make_shared<Scanner>(
        type == Types::Range::MASTER ? Types::Range::META : Types::Range::DATA,
        rsp.cid, col, 
        parent, nullptr, rsp.rid
      )->resolve_on_manager();

      return true;
    }

    void select(EndPoints endpoints, uint64_t rid, const ReqBase::Ptr& base) {
      col->selector->result->completion++;

      Rgr::Req::RangeQuerySelect::request(
        Rgr::Params::RangeQuerySelectReq(
          col->cid, rid, col->interval, col->selector->buff_sz
        ), 
        endpoints, 

        [base]() {
          SWC_LOG(LOG_DEBUG, "Select RETRYING no-conn");
          base->request_again();
        },

        [rid, base, scanner=shared_from_this()] 
        (ReqBase::Ptr req, const Rgr::Params::RangeQuerySelectRsp& rsp) {
          if(rsp.err) {
            SWC_LOGF(LOG_DEBUG, "Select RETRYING %s", rsp.to_string().c_str());
            if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
              Env::Clients::get()->rangers.remove(scanner->col->cid, rid);
              base->request_again();
            } else {
              req->request_again();
            }
            return;
          }
          auto& col = scanner->col;

          if(col->interval.flags.offset)
            col->interval.flags.offset = rsp.offset;

          if(!rsp.data.size || col->add_cells(rsp.data, rsp.reached_limit)) {
            if(rsp.reached_limit) {
              scanner->select(
                std::dynamic_pointer_cast<Rgr::Req::RangeQuerySelect>(req)
                  ->endpoints, 
                rid, base
              );
            } else {
              col->next_call();
            }
          }

          if(rsp.data.size)
            col->selector->response_partials();
          
          if(!--col->selector->result->completion)
            col->selector->response();
        },

        col->selector->timeout_select
      );
    }

  };
  
};

}}}}}

#endif // swc_lib_db_protocol_common_req_Query_h
