
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
        : notify(notify), m_cv(cv) { }

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

      if(interval.flags.limit && (interval.flags.limit -= recved) <= 0 )
        return false;
      
      //if(interval.flags.offset >= recved)
      //  interval.flags.offset -= recved;
      //else
      interval.flags.offset = 0;

      if(reached_limit) {
        auto last = m_vec.cells.back();
        interval.offset_key.copy(last->key);
        interval.offset_rev = last->revision;
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
    m_columns.insert(std::make_pair(cid, std::make_shared<Rsp>()));
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
    std::vector<int64_t> list;
    for(const auto& col : m_columns)
      list.push_back(col.first);
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

  void response_partial() {
    if(!rsp_partials)
      return;
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
      [ptr=shared_from_this()] () {
        return ptr->result->completion == 0 && 
              !ptr->m_rsp_partial_runs;
      }
    );  
  }

  void wait_on_partials_run() {
    if(!rsp_partials || result->get_size_bytes() < buff_sz * 3)
      return;

    std::unique_lock<std::mutex> lock(m_mutex);
    if(!m_rsp_partial_runs)
      return;
    m_cv.wait(
      lock, 
      [ptr=shared_from_this()] () { 
        return !ptr->m_rsp_partial_runs || 
                ptr->result->get_size_bytes() < ptr->buff_sz * 3; 
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
        if(final)
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

    ReqBase::Ptr              parent_req;
    const int64_t             rid;
    DB::Cell::Key             range_begin;

    Scanner(const Types::Range type, const int64_t cid, ScannerColumn::Ptr col,
            ReqBase::Ptr parent_req=nullptr, 
            const DB::Cell::Key* range_begin=nullptr, const int64_t rid=0)
          : type(type), cid(cid), col(col), 
            parent_req(parent_req), 
            range_begin(range_begin ? *range_begin : DB::Cell::Key()), 
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
      s.append(" RangeBegin");
      s.append(range_begin.to_string());
      s.append(" ");
      s.append(col->to_string());
      s.append(")");
      return s;
    }
    
    void locate_on_manager(bool next_range=false) {
      col->selector->result->completion++;

      Mngr::Params::RgrGetReq params(1, 0, next_range);

      if(!range_begin.empty()) {
        params.range_begin.copy(range_begin);
        col->interval.apply_possible_range_end(params.range_end); 
      } else {
        col->interval.apply_possible_range(params.range_begin, params.range_end); 
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
        [next_range, ptr=shared_from_this()]
        (ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {
          if(ptr->located_on_manager(req_ptr, rsp, next_range))
            ptr->col->selector->result->completion--;
        }
      );
    }

    void resolve_on_manager() {
      col->selector->result->completion++;

      Mngr::Req::RgrGet::request(
        Mngr::Params::RgrGetReq(cid, rid),
        [ptr=shared_from_this()]
        (ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {
          if(ptr->located_on_manager(req_ptr, rsp))
            ptr->col->selector->result->completion--;
        }
      );
    }

    bool located_on_manager(const ReqBase::Ptr& base_req, 
                            const Mngr::Params::RgrGetRsp& rsp, 
                            bool next_range=false) {
      if(rsp.cid == 1)
        SWC_LOGF(LOG_INFO, "LocatedRange-onMngr %s", rsp.to_string().c_str());
      if(rsp.err) {
        if(rsp.err == Error::COLUMN_NOT_EXISTS) {
          //std::cout << "NO-RETRY \n";
          col->selector->response(rsp.err);
          return true;

        } else if(rsp.err == Error::RANGE_NOT_FOUND) {
          if(next_range) {
            col->next_call(true);
            return true;
          }
          //std::cout << "RETRYING " << rsp.to_string() << "\n";
          (parent_req == nullptr ? base_req : parent_req)->request_again();
          return false;
        } else {
          //std::cout << "RETRYING " << rsp.to_string() << "\n";
          base_req->request_again();
          return false;
        }
      }
      if(!rsp.rid) {
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        (parent_req == nullptr ? base_req : parent_req)->request_again();
        return false;
      }

      if(type != Types::Range::DATA) {
        col->add_call(
          [scanner=std::make_shared<Scanner>(
            type, cid, col,
            parent_req == nullptr ? base_req : parent_req,
            &rsp.range_begin
          )] () { scanner->locate_on_manager(true); }
        );
      }

      if(type == Types::Range::DATA || 
        (type == Types::Range::MASTER && col->cid == 1) ||
        (type == Types::Range::META   && col->cid == 2 )) {
        if(cid != rsp.cid || col->cid != cid) {
          (parent_req == nullptr ? base_req : parent_req)->request_again();
          return false;
          //col->selector->response(Error::NOT_ALLOWED);
          //return true;
        }
        select(rsp.endpoints, rsp.rid, base_req);

      } else {
        std::make_shared<Scanner>(
          type, rsp.cid, col,
          base_req, 
          &rsp.range_begin, 
          rsp.rid
        )->locate_on_ranger(rsp.endpoints, false);
      }

      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints, bool next_range=false) {
      col->selector->result->completion++;

      Rgr::Params::RangeLocateReq params(cid, rid, next_range);
      if(!range_begin.empty()) {
        params.range_begin.copy(range_begin);
        col->interval.apply_possible_range_end(params.range_end); 
      } else {
        col->interval.apply_possible_range(params.range_begin, params.range_end); 
      }

      params.range_begin.insert(0, std::to_string(col->cid));
      if(type == Types::Range::MASTER && col->cid > 2)
        params.range_begin.insert(0, "2");
      
      params.range_end.insert(0, std::to_string(col->cid));
      if(type == Types::Range::MASTER && col->cid > 2)
        params.range_end.insert(0, "2");
      
      SWC_LOGF(LOG_INFO, "LocateRange-onRgr %s", params.to_string().c_str());

      Rgr::Req::RangeLocate::request(
        params, endpoints, 
        [ptr=shared_from_this()]() {
          ptr->parent_req->request_again();
        },
        [endpoints, next_range, ptr=shared_from_this()] 
        (ReqBase::Ptr req_ptr, Rgr::Params::RangeLocateRsp rsp) {
          if(ptr->located_on_ranger(endpoints, req_ptr, rsp, next_range))
            ptr->col->selector->result->completion--;
        }
      );
    }

    bool located_on_ranger(const EndPoints& endpoints, 
                           const ReqBase::Ptr& base_req, 
                           const Rgr::Params::RangeLocateRsp& rsp, 
                           bool next_range=false) {
      SWC_LOGF(LOG_INFO, "LocatedRange-onRgr %s", rsp.to_string().c_str());
      if(rsp.err) {
        if(rsp.err == Error::RANGE_NOT_FOUND) {
          if(next_range) {
            col->next_call(true);
            return true;
          }
          parent_req->request_again();
          return false;
        }
        if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
          parent_req->request_again();
          return false;
        } // err conn - parent_req
        base_req->request_again();
        return false;
      }

      if(!rsp.rid || (type == Types::Range::DATA && rsp.cid != col->cid)) {
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        parent_req->request_again();
        return false;
      }

      if(rsp.err) {
        col->selector->response(rsp.err);
        return true;
      }

      if(type != Types::Range::DATA) {
        col->add_call(
          [endpoints, scanner=std::make_shared<Scanner>(
            type, cid, col,
            parent_req,
            &rsp.range_begin,
            rid
          )] () { scanner->locate_on_ranger(endpoints, true); }
        );
      }

      std::make_shared<Scanner>(
        type == Types::Range::MASTER ? Types::Range::META : Types::Range::DATA,
        rsp.cid, col, 
        parent_req, 
        &rsp.range_begin, 
        rsp.rid
      )->resolve_on_manager();

      return true;
    }

    void select(EndPoints endpoints, uint64_t rid, 
                const ReqBase::Ptr& base_req) {
      col->selector->result->completion++;

      //std::cout << "Query::Select select, cid=" << col->cid 
      //          << " rid=" << rid << ""  << col->interval.to_string() << "\n"; 
      
      Rgr::Req::RangeQuerySelect::request(
        Rgr::Params::RangeQuerySelectReq(
          col->cid, rid, col->interval, col->selector->buff_sz
        ), 
        endpoints, 
        [base_req, ptr=shared_from_this()]() {
          base_req->request_again();
          //std::cout << "RETRYING NO-CONN\n";
        },
        [rid, base_req, ptr=shared_from_this()] 
        (ReqBase::Ptr req_ptr, const Rgr::Params::RangeQuerySelectRsp& rsp) {

          if(rsp.err) {
            if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
              base_req->request_again();
              //std::cout << "RETRYING " << rsp.to_string() << "\n";
            } else {
              //std::cout << "RETRYING " << rsp.to_string() << "\n";
              req_ptr->request_again();
            }
            return;
          }
          ptr->col->selector->result->err = rsp.err;

          bool more = true;
          if(rsp.data.size) {
            more = ptr->col->add_cells(rsp.data, rsp.reached_limit);
          }

          if(more) {
            ptr->col->selector->wait_on_partials_run();

            if(rsp.reached_limit) {
              auto qreq = std::dynamic_pointer_cast<
                Rgr::Req::RangeQuerySelect>(req_ptr);
              ptr->select(qreq->endpoints, rid, base_req);
            } else {
              ptr->col->next_call();
            }
          }

          if(rsp.data.size)
            ptr->col->selector->response_partial();

          if(!--ptr->col->selector->result->completion)
            ptr->col->selector->response();

        },
        col->selector->timeout_select
      );
    }

  };
  
};

}}}}}

#endif // swc_lib_db_protocol_common_req_Query_h
