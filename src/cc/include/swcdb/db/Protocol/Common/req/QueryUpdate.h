
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_QueryUpdate_h
#define swc_lib_db_protocol_common_req_QueryUpdate_h

#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/db/Cells/MapMutable.h" 

#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQueryUpdate.h"
#include "swcdb/db/Types/Range.h"


namespace SWC { namespace Protocol { namespace Common { namespace Req { 
  
namespace Query {

using ReqBase = Req::ConnQueue::ReqBase;

/*
range-master: 
  req-mngr.   cid(1) + [n(cid), next_key_start]
              => cid(1) + rid + rgr(endpoints) + range_begin + range_end	
    req-rgr.  cid(1) + rid + [cid(n), next_key_start]
              => cid(2) + rid + range_begin + range_end
range-meta: 
  req-mngr.   cid(2) + rid                           
              => cid(2) + rid + rgr(endpoints)	
    req-rgr.  cid(2) + rid + [cid(n), next_key_start]
              => cid(n) + rid + range_begin + range_end
range-data: 
  req-mngr.   cid(n) + rid                           
              => cid(n) + rid + rgr(endpoints)	
    req-rgr.  cid(n) + rid + Specs::Interval         
              => results
*/
 
namespace Result{

struct Update final {
  typedef std::shared_ptr<Update> Ptr;
  int err;
  std::atomic<uint32_t> completion = 0;
  
  DB::Cells::MapMutable errored;
};

}
  

class Update : public std::enable_shared_from_this<Update> {
  public:

  using Result = Result::Update;

  typedef std::shared_ptr<Update>           Ptr;
  typedef std::function<void(Result::Ptr)>  Cb_t;
  
  Cb_t                        cb;
  DB::Cells::MapMutable::Ptr  columns;
  DB::Cells::MapMutable::Ptr  columns_onfractions;

  Result::Ptr                 result;

  uint32_t buff_sz          = 8000000;
  uint32_t timeout_commit   = 60000;
  uint32_t timeout_commit_bytes_ratio = 750;
  

  std::mutex                  m_mutex;
  std::condition_variable     cv;

  Update(Cb_t cb=0)
        : cb(cb),
          columns(std::make_shared<DB::Cells::MapMutable>()),
          columns_onfractions(std::make_shared<DB::Cells::MapMutable>()),
          result(std::make_shared<Result>()) { 
  }

  Update(DB::Cells::MapMutable::Ptr columns, 
         DB::Cells::MapMutable::Ptr columns_onfractions, 
         Cb_t cb=0)
        : cb(cb), 
          columns(columns), 
          columns_onfractions(columns_onfractions), 
          result(std::make_shared<Result>()) { 
  }

  virtual ~Update() { }
 
  void response(int err=Error::OK) {
    if(columns->size() || columns_onfractions->size()) {
      commit();
      return;
    }

    if(err)
      result->err=err;

    if(cb)
      cb(result);
    cv.notify_all();
  }

  void wait() {
    std::unique_lock<std::mutex> lock_wait(m_mutex);
    if(result->completion == 0)
      return;
    cv.wait(
      lock_wait, 
      [updater=shared_from_this()]() {
        return updater->result->completion==0;
      }
    );  
  }

  void commit() {
    DB::Cells::ColCells::Ptr col;
    for(size_t idx=0; (col=columns->get_idx(idx)) != nullptr; idx++)
      commit(col);
    for(size_t idx=0; (col=columns_onfractions->get_idx(idx)) != nullptr; idx++)
      commit_onfractions(col);
  }

  void commit(const int64_t cid) { 
    commit(columns->get_col(cid));
    commit_onfractions(columns_onfractions->get_col(cid));
  }

  void commit(DB::Cells::ColCells::Ptr col) {
    if(col != nullptr && !col->size())
      return;
    std::make_shared<Locator>(
      Types::Range::MASTER, 
      col->cid, col, col->get_first_key(), 
      shared_from_this()
    )->locate_on_manager();
  }

  void commit_onfractions(DB::Cells::ColCells::Ptr col) {
    if(col != nullptr && !col->size())
      return;
    // query cells on fractions -EQ && rest(NONE-true)
    // update cell with + ts,flag,value and add cell to 'columns'
  }

  class Locator : public std::enable_shared_from_this<Locator> {
    public:
    const Types::Range        type;
    const int64_t             cid;
    DB::Cells::ColCells::Ptr  col;
    DB::Cell::Key::Ptr        key_start;
    Update::Ptr               updater;
    ReqBase::Ptr              parent;
    const int64_t             rid;
    const DB::Cell::Key       key_finish;
    
    Locator(const Types::Range type, const int64_t cid, 
            DB::Cells::ColCells::Ptr col, DB::Cell::Key::Ptr key_start,
            Update::Ptr updater, ReqBase::Ptr parent=nullptr, 
            const int64_t rid=0, const DB::Cell::Key* key_finish=nullptr) 
            : type(type), cid(cid), col(col), key_start(key_start), 
              updater(updater), parent(parent), 
              rid(rid), 
              key_finish(key_finish ? *key_finish : DB::Cell::Key()) {
    }

    virtual ~Locator() { }

    const std::string to_string() {
      std::string s("Locator(type=");
      s.append(Types::to_string(type));
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));
      s.append(" completion=");
      s.append(std::to_string(updater->result->completion.load()));
      s.append(" Start");
      s.append(key_start->to_string());
      s.append(" Finish");
      s.append(key_finish.to_string());
      s.append(" ");
      s.append(col->to_string());
      return s;
    }

    void locate_on_manager() {
      updater->result->completion++;

      Mngr::Params::RgrGetReq params(1);
      params.range_begin.copy(*key_start.get());
      if(cid > 2)
        params.range_begin.insert(0, std::to_string(cid));
      if(cid >= 2)
        params.range_begin.insert(0, "2");

      SWC_LOGF(LOG_INFO, "LocateRange-onMngr %s", params.to_string().c_str());

      Mngr::Req::RgrGet::request(
        params,
        [locator=shared_from_this()]
        (ReqBase::Ptr req, Mngr::Params::RgrGetRsp rsp) {
          if(locator->located_on_manager(req, rsp))
            locator->updater->result->completion--;
        }
      );
    }

    private:

    bool located_on_manager(const ReqBase::Ptr& base, 
                            const Mngr::Params::RgrGetRsp& rsp) {  
      SWC_LOGF(LOG_INFO, "LocatedRange-onMngr %s", rsp.to_string().c_str());
      
      if(rsp.err == Error::COLUMN_NOT_EXISTS) {
        updater->response(rsp.err);
        return true;
      }
      if(rsp.err || !rsp.rid) {
        SWC_LOGF(LOG_DEBUG, "Located-onMngr RETRYING %s", 
                              rsp.to_string().c_str());
        base->request_again();
        return false;
      }
      
      auto locator = std::make_shared<Locator>(
        Types::Range::MASTER, 
        rsp.cid, col, key_start, 
        updater, base, 
        rsp.rid
      );
      if(col->cid == 1)
        locator->commit_data(rsp.endpoints, base);
      else 
        locator->locate_on_ranger(rsp.endpoints);

      if(!rsp.range_end.empty()) {
        auto next_key_start = col->get_key_next(rsp.range_end);
        if(next_key_start != nullptr) {
          std::make_shared<Locator>(
            Types::Range::MASTER, 
            col->cid, col, next_key_start, 
            updater
          )->locate_on_manager();
        }
      }
      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints) {
      updater->result->completion++;

      Rgr::Params::RangeLocateReq params(cid, rid);
      params.flags |= Rgr::Params::RangeLocateReq::COMMIT;

      params.range_begin.copy(*key_start.get());
      if(col->cid >= 2) {
        params.range_begin.insert(0, std::to_string(col->cid));
        if(type == Types::Range::MASTER && col->cid > 2) 
          params.range_begin.insert(0, "2");
      }
      
      SWC_LOGF(LOG_INFO, "LocateRange-onRgr %s", params.to_string().c_str());

      Rgr::Req::RangeLocate::request(
        params, endpoints, 
        [locator=shared_from_this()]() {
          locator->parent->request_again();
        },
        [endpoints, locator=shared_from_this()] 
        (ReqBase::Ptr req, Rgr::Params::RangeLocateRsp rsp) {
          if(locator->located_on_ranger(endpoints, req, rsp))
            locator->updater->result->completion--;
        }
      );
    }

    bool located_on_ranger(const EndPoints& endpoints, 
                           const ReqBase::Ptr& base, 
                           const Rgr::Params::RangeLocateRsp& rsp) {
      SWC_LOGF(LOG_INFO, "LocatedRange-onRgr %s", rsp.to_string().c_str());

      if(rsp.err == Error::RS_NOT_LOADED_RANGE || 
         rsp.err == Error::RANGE_NOT_FOUND) {
        SWC_LOGF(LOG_DEBUG, "Located-onRgr RETRYING %s", 
                              rsp.to_string().c_str());
        Env::Clients::get()->rangers.remove(cid, rid);
        parent->request_again();
        return false;
      }
      if(!rsp.rid || rsp.err) {
        SWC_LOGF(LOG_DEBUG, "Located-onRgr HALT %s", 
                              rsp.to_string().c_str());
        Env::Clients::get()->rangers.remove(cid, rid);
        quick_exit(1);
        base->request_again();
        return false;
      }

      std::make_shared<Locator>(
        type == Types::Range::MASTER ? Types::Range::META : Types::Range::DATA,
        rsp.cid, col, key_start, 
        updater, base, 
        rsp.rid, &rsp.range_end
      )->resolve_on_manager();

      if(!rsp.range_end.empty()) {
        auto next_key_start = col->get_key_next(rsp.range_end);
        if(next_key_start != nullptr) {
          std::make_shared<Locator>(
            type, 
            cid, col, next_key_start, 
            updater, base, 
            rid//, &key_finish
          )->locate_on_ranger(endpoints);
        }
      }
      return true;
    }

    void resolve_on_manager() {

      auto req = Mngr::Req::RgrGet::make(
        Mngr::Params::RgrGetReq(cid, rid),
        [locator=shared_from_this()]
        (ReqBase::Ptr req, Mngr::Params::RgrGetRsp rsp) {
          if(locator->located_ranger(req, rsp))
            locator->updater->result->completion--;
        }
      );
      if(cid != 1) {
        Mngr::Params::RgrGetRsp rsp;
        rsp.cid = cid;
        rsp.rid = rid;
        if(Env::Clients::get()->rangers.get(cid, rid, rsp.endpoints)) {
          SWC_LOGF(LOG_INFO, "Cache hit %s", rsp.to_string().c_str());
          if(proceed_on_ranger(req, rsp))
            return; 
          Env::Clients::get()->rangers.remove(cid, rid);
        } else
          SWC_LOGF(LOG_INFO, "Cache miss %s", rsp.to_string().c_str());
      }
      updater->result->completion++;
      req->run();
    }

    bool located_ranger(const ReqBase::Ptr& base, 
                        const Mngr::Params::RgrGetRsp& rsp) {  
      SWC_LOGF(LOG_INFO, "LocatedRanger-onMngr %s", rsp.to_string().c_str());

      if(rsp.err) {
        if(rsp.err == Error::COLUMN_NOT_EXISTS) {
          updater->response(rsp.err);
          return true;
        }
        SWC_LOGF(LOG_DEBUG, "LocatedRanger-onMngr RETRYING %s", 
                              rsp.to_string().c_str());
        SWC_LOGF(LOG_DEBUG, " %s", to_string().c_str());
        if(rsp.err == Error::RANGE_NOT_FOUND) {
          (parent == nullptr ? base : parent)->request_again();
        } else {
          base->request_again();
        }
        return false;
      }

      if(!rsp.rid || rsp.endpoints.empty()) {
        SWC_LOGF(LOG_DEBUG, "LocatedRanger-onMngr RETRYING(no rid) %s", 
                            rsp.to_string().c_str());
        (parent == nullptr ? base : parent)->request_again();
        return false;
      }

      if(rsp.cid != 1)
        Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);

      return proceed_on_ranger(base, rsp);
    }

    bool proceed_on_ranger(const ReqBase::Ptr& base, 
                           const Mngr::Params::RgrGetRsp& rsp) {
      if(type == Types::Range::DATA || 
        (type == Types::Range::MASTER && col->cid == 1) ||
        (type == Types::Range::META   && col->cid == 2 )) {
        if(cid != rsp.cid || col->cid != cid) {
          SWC_LOGF(LOG_DEBUG, "LocatedRanger-onMngr RETRYING(cid no match) %s", 
                                rsp.to_string().c_str());
          (parent == nullptr ? base : parent)->request_again();          
          //updater->response(Error::NOT_ALLOWED);
          return false;
        }
        commit_data(rsp.endpoints, base);

      } else {
        std::make_shared<Locator>(
          type, 
          rsp.cid, col, key_start, 
          updater, base, 
          rsp.rid, 
          cid == 1 ? &rsp.range_end : nullptr
        )->locate_on_ranger(rsp.endpoints);
      }
      return true;
    }

    void commit_data(EndPoints endpoints, const ReqBase::Ptr& base) {
      bool more = true;
      DynamicBuffer::Ptr cells_buff;
      while(more && 
           (cells_buff = col->get_buff(
             *key_start.get(), key_finish, updater->buff_sz, more)) 
                                                            != nullptr) {
        updater->result->completion++;

        Rgr::Req::RangeQueryUpdate::request(
          Rgr::Params::RangeQueryUpdateReq(col->cid, rid), 
          cells_buff, 
          endpoints, 

          [cells_buff, base, locator=shared_from_this()]() {
            locator->col->add(*cells_buff.get());
            SWC_LOG(LOG_DEBUG, "Commit RETRYING no-conn");
            base->request_again();
            --locator->updater->result->completion;
          },

          [cells_buff, base, locator=shared_from_this()] 
          (ReqBase::Ptr req, Rgr::Params::RangeQueryUpdateRsp rsp) {
            if(rsp.err) {
              SWC_LOGF(LOG_DEBUG, "Commit RETRYING %s", rsp.to_string().c_str());

              if(rsp.err == Error::REQUEST_TIMEOUT) {
                SWC_LOGF(LOG_DEBUG, " %s", req->to_string().c_str());
                req->request_again();
                return;
              }

              if(rsp.err == Error::RANGE_END_EARLIER) {
                locator->col->add(*cells_buff.get(), rsp.range_end);
                auto next_key_start = locator->col->get_key_next(rsp.range_end);
                if(next_key_start != nullptr) {
                  std::make_shared<Locator>(
                    Types::Range::MASTER, 
                    locator->col->cid, locator->col, next_key_start, 
                    locator->updater
                  )->locate_on_manager();
                 }
              } else {
                locator->col->add(*cells_buff.get());
                base->request_again();
              }
            }
            
            if(!--locator->updater->result->completion)
              locator->updater->response();
          },

          updater->timeout_commit += 
            cells_buff->fill()/updater->timeout_commit_bytes_ratio
        );

      } while(more);
    }

  };

};


}}}}}

#endif // swc_lib_db_protocol_common_req_QueryUpdate_h
