
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Query_h
#define swc_lib_db_protocol_common_req_Query_h

#include "swcdb/lib/db/Cells/Vector.h"

#include "QueryUpdate.h"
#include "swcdb/lib/db/Protocol/Rgr/req/RangeQuerySelect.h"

namespace SWC { namespace Protocol { namespace Common { namespace Req { 
  
namespace Query {
 
namespace Result{

struct Select{
  typedef std::shared_ptr<Select> Ptr;

  std::atomic<uint32_t> completion = 0;
  int err = Error::OK;

  std::unordered_map<int64_t, DB::Cells::Vector::Ptr> columns;

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

  uint32_t buff_sz          = 1000000;
  uint32_t timeout_commit   = 60000;

  std::mutex                  m_mutex;
  std::condition_variable     cv;

  Select(Cb_t cb=0)
        : cb(cb), result(std::make_shared<Result>()) { }

  Select(const DB::Specs::Scan& specs, Cb_t cb=0)
        : cb(cb), specs(specs), 
          result(std::make_shared<Result>()) { }

  virtual ~Select(){ }
 
  void response() {
    // if cells not empty commit-again
    if(cb)
      cb(result);
    cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lock_wait(m_mutex);
    cv.wait(
      lock_wait, 
      [ptr=shared_from_this()](){return ptr->result->completion==0;}
    );  
  }

  void scan() {
    for(auto &col : specs.columns)
      result->columns.insert(std::make_pair(col->cid, DB::Cells::Vector::make()));
    
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
    uint64_t                  offset;

    typedef std::shared_ptr<std::vector<std::function<void()>>> NextCalls;
    NextCalls next_calls;

    Scanner(const Types::Range type, 
            const int64_t cid, const int64_t cells_cid, 
            DB::Specs::Interval::Ptr interval,
            Select::Ptr selector,
            NextCalls next_calls=nullptr, 
            ReqBase::Ptr parent_req=nullptr, 
            const uint64_t offset=0, const int64_t rid=0)
          : type(type), cid(cid), cells_cid(cells_cid), 
            interval(interval), selector(selector),
            next_calls(
              next_calls == nullptr ? 
              std::make_shared<std::vector<std::function<void()>>>() : 
              next_calls
            ),
            parent_req(parent_req), rid(rid), offset(offset)  {
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
      s.append(" offset=");
      s.append(std::to_string(offset));
      s.append(" next_calls=");
      s.append(std::to_string(next_calls->size()));
      
      s.append(" ");
      s.append(interval->to_string());
      return s;
    }
    
    void locate_on_manager() {

      Mngr::Params::RgrGetReq params(1);
      params.interval.key_start.copy(interval->key_start);
      params.interval.key_start.set(-1, Condition::GE);
      if(cid > 2)
        params.interval.key_start.insert(
          0, std::to_string(cid), Condition::GE);
      if(cid >= 2)
        params.interval.key_start.insert(0, "2", Condition::EQ);
      if(cid == 1) 
        params.interval.key_start.set(0, Condition::EQ);
      
      params.interval.flags.offset = offset;
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
      if(rsp.rid == 0) {
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        (parent_req == nullptr ? base_req : parent_req)->request_again();
        return false;
      }

      if(type != Types::Range::DATA && rsp.next_key && !rsp.key_end.empty()) {
        //std::cout << "located_on_manager, NEXT-KEY: " 
        //          << Types::to_string(type) 
        //          << " " << rsp.key_end.to_string() << "\n";
        next_calls->push_back([scanner=std::make_shared<Scanner>(
          type, cid, cells_cid, interval, selector, next_calls,
          parent_req == nullptr ? base_req : parent_req,
          ++offset)] () { scanner->locate_on_manager(); });
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
          base_req, 0, rsp.rid
        )->locate_on_ranger(rsp.endpoints);

      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints) {
      HT_ASSERT(type != Types::Range::DATA);

      Rgr::Params::RangeLocateReq params(cid, rid);

      params.interval.key_start.copy(interval->key_start);
      params.interval.key_start.set(-1, Condition::GE);
      params.interval.key_start.insert(0, std::to_string(cells_cid),
        type == Types::Range::MASTER? Condition::GE : Condition::EQ);
      if(type == Types::Range::MASTER && cells_cid > 2) 
        params.interval.key_start.insert(0, "2", Condition::EQ);

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
      if(rsp.rid == 0 
      ||(type == Types::Range::DATA && rsp.cid != cells_cid)) {
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        parent_req->request_again();
        return false;
      }

      selector->result->err=rsp.err;
      if(rsp.err){

        return true;
      }

      auto current_offset = offset;
      if(rsp.next_key && !rsp.key_end.empty()
        && type != Types::Range::DATA 
        && interval->key_finish.is_matching(rsp.key_end)) {
              
        //std::cout << "located_on_ranger, NEXT-KEY: " 
        //  << Types::to_string(type) 
        //  << " " << rsp.key_end.to_string() << "\n";
          
        next_calls->push_back([endpoints, scanner=std::make_shared<Scanner>(
          type, cid, cells_cid, interval, selector, next_calls, parent_req, ++offset
        )] () { scanner->locate_on_ranger(endpoints); });
      }

      std::make_shared<Scanner>(
        type == Types::Range::MASTER
        ? Types::Range::META : Types::Range::DATA,
        rsp.cid, cells_cid, interval, selector, next_calls, 
        parent_req, current_offset, rsp.rid
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
        (ReqBase::Ptr req_ptr, Rgr::Params::RangeQuerySelectRsp rsp) {

          //std::cout << "select, Rgr::Req::RangeQuerySelect: "
          //  << rsp.to_string() 
          //  << " completion=" 
          //  << ptr->selector->result->completion.load() << "\n";

          if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
            base_req->request_again();
            --ptr->selector->result->completion;
            //std::cout << "RETRYING " << rsp.to_string() << "\n";
            return;
          }      
          auto col = ptr->selector->result->columns[ptr->cells_cid]; 
          if(rsp.size > 0) 
            col->add(&rsp.bufp, &rsp.size);
          
          //std::cout << col->cells.size() << "\n";

          if(ptr->interval->flags.limit == 0 
            || col->cells.size() < ptr->interval->flags.limit) {
            if(rsp.reached_limit) {
              auto qreq = std::dynamic_pointer_cast<Rgr::Req::RangeQuerySelect>(req_ptr);
              auto last = col->cells.back();
              if(ptr->interval->flags.offset) {
                ssize_t adj = ptr->interval->flags.offset-col->cells.size();
                ptr->interval->flags.offset = adj < 0 ? 0 : adj;
              }
              //std::cout << "LAST cell, " << last->to_string() << "\n";
              ptr->interval->offset_key.copy(last->key);
              ptr->interval->offset_rev = last->revision;
              ptr->select(qreq->endpoints, rid, base_req);
              --ptr->selector->result->completion;
              return;
            }
            if(!ptr->next_calls->empty()) {
              ptr->interval->offset_key.free();
              ptr->interval->offset_rev = 0;
              auto it = ptr->next_calls->begin();
              auto call = *it;
              ptr->next_calls->erase(it);
              call();
              --ptr->selector->result->completion;
              return;
            }
          }

          ptr->selector->result->err=rsp.err;
          if(!--ptr->selector->result->completion) {
            ptr->selector->response();
          }
        },
        selector->timeout_commit
      );
    }

  };
  
};

}}}}}

#endif // swc_lib_db_protocol_common_req_Query_h
