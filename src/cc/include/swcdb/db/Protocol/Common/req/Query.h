
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

  struct Rsp final {
    public:
    typedef std::shared_ptr<Rsp> Ptr;

    const bool add_cells(const StaticBuffer& buffer, bool more, 
                         DB::Specs::Interval& interval) {
      std::scoped_lock lock(m_mutex);
      size_t recved = m_vec.add(buffer.base, buffer.size);
      m_counted += recved;
      m_size_bytes += buffer.size;

      if(interval.flags.limit && (interval.flags.limit -= recved) <= 0 )
        return false;
      
      interval.flags.offset = 0;
      //if(more) {
      auto last = m_vec.cells.back();
      interval.offset_key.copy(last->key);
      interval.offset_rev = last->revision;
      //}
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
                       bool more, DB::Specs::Interval& interval) {
    return m_columns[cid]->add_cells(buffer, more, interval);
  }

  void get_cells(const int64_t cid, DB::Cells::Vector& cells) {
    return m_columns[cid]->get_cells(cells);
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
  }

  void remove(const int64_t cid) {
    m_columns.erase(cid);
  }
  
  private:
  std::unordered_map<int64_t, Rsp::Ptr> m_columns;

};
}

class Select : public std::enable_shared_from_this<Select> {
  public:

  using Result = Result::Select;

  typedef std::shared_ptr<Select>           Ptr;
  typedef std::function<void(Result::Ptr)>  Cb_t;
  
  Cb_t                        cb;
  DB::Specs::Scan             specs;
  Result::Ptr                 result;

  uint32_t buff_sz          = 8000000;
  uint32_t timeout_select   = 600000;

  std::mutex                  m_mutex;
  bool                        rsp_partials;
  bool                        rsp_partial_runs = false;
  std::condition_variable     cv;

  Select(Cb_t cb=0, bool rsp_partials=false)
        : cb(cb), 
          result(std::make_shared<Result>()), 
          rsp_partials(cb && rsp_partials) { 
  }

  Select(const DB::Specs::Scan& specs, Cb_t cb=0, bool rsp_partials=false)
        : cb(cb), specs(specs), 
          result(std::make_shared<Result>()), 
          rsp_partials(cb && rsp_partials) {
  }

  virtual ~Select(){ }
 
  void response() {
    // if cells not empty commit-again
    if(cb)
      cb(result);
    cv.notify_all();
  }

  void response_partial() {
    if(!rsp_partials)
      return;
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      if(rsp_partial_runs)
        return;
      rsp_partial_runs = true;
    }
    
    cb(result);

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      rsp_partial_runs = false;
    }
    cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    cv.wait(
      lock, 
      [ptr=shared_from_this()] () {
        return ptr->result->completion == 0 && !ptr->rsp_partial_runs;
      }
    );  
  }

  void wait_on_partials_run() {
    if(!rsp_partials || result->get_size_bytes() < buff_sz * 3)
      return;

    std::unique_lock<std::mutex> lock(m_mutex);
    if(!rsp_partial_runs)
      return;
    cv.wait(
      lock, 
      [ptr=shared_from_this()] () { return !ptr->rsp_partial_runs; }
    );  
  }

  void scan() {
    for(auto &col : specs.columns)
      result->add_column(col->cid);
    
    for(auto &col : specs.columns){
      //std::cout << "Select::scan, " << col->to_string() << "\n";
      for(auto &intval : col->intervals){
        std::make_shared<Scanner>(
          Types::Range::MASTER,
          col->cid, col->cid, 
          DB::Specs::Interval::make_ptr(intval),
          shared_from_this()
        )->locate_on_manager();
      }
    }
  }


  class Scanner: public std::enable_shared_from_this<Scanner> {
    public:
    const Types::Range        type;
    const int64_t             cid;
    const int64_t             cells_cid;
    DB::Specs::Interval::Ptr  interval;
    Select::Ptr               selector;
    ReqBase::Ptr              parent_req;
    const int64_t             rid;

    typedef std::shared_ptr<std::vector<std::function<void()>>> NextCalls;
    NextCalls next_calls;

    Scanner(const Types::Range type, 
            const int64_t cid, const int64_t cells_cid, 
            DB::Specs::Interval::Ptr interval,
            Select::Ptr selector,
            NextCalls next_calls=nullptr, 
            ReqBase::Ptr parent_req=nullptr, 
            const int64_t rid=0)
          : type(type), cid(cid), cells_cid(cells_cid), 
            interval(interval), selector(selector),
            next_calls(
              next_calls == nullptr ? 
              std::make_shared<std::vector<std::function<void()>>>() : 
              next_calls
            ),
            parent_req(parent_req), rid(rid) {
    }

    virtual ~Scanner() {}

    const std::string to_string() {
      std::string s("Scanner(type=");
      s.append(Types::to_string(type));
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));
      s.append(" completion=");
      s.append(std::to_string(selector->result->completion.load()));
      s.append(" cells_cid=");
      s.append(std::to_string(cells_cid));
      s.append(" next_calls=");
      s.append(std::to_string(next_calls->size()));
      
      s.append(" ");
      s.append(interval->to_string());
      return s;
    }
    
    void locate_on_manager() {

      Mngr::Params::RgrGetReq params(
        1, 
        interval->offset_key.empty() ? 
          interval->range_begin : interval->offset_key,
        interval->range_end
      );

      if(cid > 2)
        params.range_begin.insert(0, std::to_string(cid));
      if(cid >= 2)
        params.range_begin.insert(0, "2");
      
      if(!params.range_end.empty()) {
        if(cid > 2) 
          params.range_end.insert(0, std::to_string(cid));
        if(cid >= 2) 
          params.range_end.insert(0, "2");
      }
        
      //std::cout << "locate_on_manager:\n " 
      //          << to_string() << "\n "
      //          << params.to_string() << "\n";

      selector->result->completion++;

      Mngr::Req::RgrGet::request(
        params,
        [ptr=shared_from_this()]
        (ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {
          if(ptr->located_on_manager(req_ptr, rsp))
            ptr->selector->result->completion--;
        }
      );
    }

    void resolve_on_manager() {
      //std::cout << "resolve_on_manager:\n " << to_string() << "\n";
      // if cid, rid >> cache rsp

      selector->result->completion++;

      Mngr::Req::RgrGet::request(
        Mngr::Params::RgrGetReq(cid, rid),
        [ptr=shared_from_this()]
        (ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {
          if(ptr->located_on_manager(req_ptr, rsp))
            ptr->selector->result->completion--;
        }
      );
    }

    bool located_on_manager(const ReqBase::Ptr& base_req, 
                            const Mngr::Params::RgrGetRsp& rsp) {

      //std::cout << "located_on_manager:\n " << to_string() 
      //          << "\n " << rsp.to_string() << "\n";
      
      if(rsp.err != Error::OK){
        // err type? ~| parent_req->request_again();
        if(rsp.err == Error::COLUMN_NOT_EXISTS) {
          //std::cout << "NO-RETRY \n";
          return true;
        } else if(rsp.err == Error::RANGE_NOT_FOUND) {
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

      if(type != Types::Range::DATA 
         && rsp.next_range && !rsp.range_end.empty()) {
        //std::cout << "located_on_manager, NEXT-KEY: " 
        //          << Types::to_string(type) 
        //          << " " << rsp.range_end.to_string() << "\n";
        next_calls->push_back([scanner=std::make_shared<Scanner>(
          type, cid, cells_cid, interval, selector, next_calls,
          parent_req == nullptr ? base_req : parent_req)] () { 
            scanner->locate_on_manager(); 
          }
        );
      }


      if(type == Types::Range::DATA || 
        (type == Types::Range::MASTER && cells_cid == 1) ||
        (type == Types::Range::META   && cells_cid == 2 )) {
        if(cid != rsp.cid || cells_cid != cid) {
          (parent_req == nullptr ? base_req : parent_req)->request_again();
          return false;
          //selector->result->err = Error::NOT_ALLOWED;
          //selector->response();
        }
        select(rsp.endpoints, rsp.rid, base_req);

      } else 
        std::make_shared<Scanner>(
          type, rsp.cid, cells_cid, interval, selector, next_calls,
          base_req, rsp.rid
        )->locate_on_ranger(rsp.endpoints);

      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints) {
      HT_ASSERT(type != Types::Range::DATA);

      // applicable(! LT/E RE) Specs::Key at ranges empty
      Rgr::Params::RangeLocateReq params(
        cid, rid, 
        interval->offset_key.empty() ? 
          interval->range_begin : interval->offset_key,
        interval->range_end
      );

      params.range_begin.insert(0, std::to_string(cells_cid));
      if(type == Types::Range::MASTER && cells_cid > 2)
        params.range_begin.insert(0, "2");
      
      if(!params.range_end.empty()) {
        params.range_end.insert(0, std::to_string(cells_cid));
        if(type == Types::Range::MASTER && cells_cid > 2)
          params.range_end.insert(0, "2");
      }

      //std::cout << "locate_on_ranger:\n "
      //          << to_string() << "\n "
      //          << params.to_string() << "\n";
      
      selector->result->completion++;

      Rgr::Req::RangeLocate::request(
        params, 
        endpoints, 
        [ptr=shared_from_this()]() {
          ptr->parent_req->request_again();
        },
        [endpoints, ptr=shared_from_this()] 
        (ReqBase::Ptr req_ptr, Rgr::Params::RangeLocateRsp rsp) {
          if(ptr->located_on_ranger(endpoints, req_ptr, rsp))
            ptr->selector->result->completion--;
        }
      );
    }

    bool located_on_ranger(const EndPoints& endpoints, 
                           const ReqBase::Ptr& base_req, 
                           const Rgr::Params::RangeLocateRsp& rsp) {
      //std::cout << "located_on_ranger:\n " << to_string() 
      //          << "\n " << rsp.to_string() << "\n";

      if(rsp.err == Error::RS_NOT_LOADED_RANGE
      || rsp.err == Error::RANGE_NOT_FOUND) {
        parent_req->request_again();
        return false;
      }
      if(!rsp.rid
      ||(type == Types::Range::DATA && rsp.cid != cells_cid)) {
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        parent_req->request_again();
        return false;
      }

      selector->result->err=rsp.err;
      if(rsp.err){

        return true;
      }

      if(rsp.next_range && !rsp.range_end.empty()
        && type != Types::Range::DATA 
        && interval->key_finish.is_matching(rsp.range_end)) {
              
        //std::cout << "located_on_ranger, NEXT-KEY: " 
        //  << Types::to_string(type) 
        //  << " " << rsp.range_end.to_string() << "\n";
          
        next_calls->push_back([endpoints, scanner=std::make_shared<Scanner>(
          type, cid, cells_cid, interval, selector, next_calls, parent_req)]
          () { scanner->locate_on_ranger(endpoints); });
      }

      std::make_shared<Scanner>(
        type == Types::Range::MASTER
        ? Types::Range::META : Types::Range::DATA,
        rsp.cid, cells_cid, interval, selector, next_calls, 
        parent_req, rsp.rid
      )->resolve_on_manager();

      return true;
    }

    void select(EndPoints endpoints, uint64_t rid, 
                const ReqBase::Ptr& base_req) {
      //std::cout << "Query::Select select, cid=" << cells_cid 
      //          << " rid=" << rid << ""  << interval->to_string() << "\n"; 
      selector->result->completion++;

      Rgr::Req::RangeQuerySelect::request(
        Rgr::Params::RangeQuerySelectReq(cells_cid, rid, *interval.get(), selector->buff_sz), 
        endpoints, 
        [base_req, ptr=shared_from_this()]() {
          base_req->request_again();
          --ptr->selector->result->completion;
          //std::cout << "RETRYING NO-CONN\n";
        },
        [rid, base_req, ptr=shared_from_this()] 
        (ReqBase::Ptr req_ptr, const Rgr::Params::RangeQuerySelectRsp& rsp) {

          if(rsp.err) {
            if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
              base_req->request_again();
              --ptr->selector->result->completion;
              //std::cout << "RETRYING " << rsp.to_string() << "\n";
            } else {
              std::cout << "RETRYING " << rsp.to_string() << "\n";
              req_ptr->request_again();
            }
            return;
          }

          bool more = true;
          if(rsp.data.size) {
            more = ptr->selector->result->add_cells(
              ptr->cells_cid, 
              rsp.data, rsp.reached_limit, *ptr->interval.get()
            );
          }

          if(more) {
            ptr->selector->wait_on_partials_run();

            if(rsp.reached_limit) {
              auto qreq = std::dynamic_pointer_cast<Rgr::Req::RangeQuerySelect>(req_ptr);
              ptr->select(qreq->endpoints, rid, base_req);

            } else if(!ptr->next_calls->empty()) {
              //ptr->interval->offset_key.free();
              //ptr->interval->offset_rev = 0;
              auto it = ptr->next_calls->begin();
              auto call = *it;
              ptr->next_calls->erase(it);
              call();
            }
          }

          ptr->selector->result->err = rsp.err;
          if(rsp.data.size)
            ptr->selector->response_partial();

          if(!--ptr->selector->result->completion)
            ptr->selector->response();

        },
        selector->timeout_select
      );
    }

  };
  
};

}}}}}

#endif // swc_lib_db_protocol_common_req_Query_h
