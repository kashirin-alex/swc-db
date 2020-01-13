
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
  req-mngr.   cid(1) + n(cid):range_begin+range_end     
              => cid(1) + rid + rgr(endpoints) + key_end + ?range_next	
    req-rgr.  cid(1) + rid + cid(n):Specs::Interval  
              => cid(2) + rid + key_end + ?range_next	
range-meta: 
  req-mngr.   cid(2) + rid                           
              => cid(2) + rid + rgr(endpoints)	
    req-rgr.  cid(2) + rid + cid(n):Specs::Interval  
              => cid(n) + rid + key_end + ?range_next	
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

  std::mutex                  m_mutex;
  std::condition_variable     cv;

  Update(Cb_t cb=0)
        : cb(cb),
          columns(std::make_shared<DB::Cells::MapMutable>()),
          columns_onfractions(std::make_shared<DB::Cells::MapMutable>()),
          result(std::make_shared<Result>()) { }

  Update(DB::Cells::MapMutable::Ptr columns, 
         DB::Cells::MapMutable::Ptr columns_onfractions, 
         Cb_t cb=0)
        : cb(cb), 
          columns(columns), 
          columns_onfractions(columns_onfractions), 
          result(std::make_shared<Result>()) { }

  virtual ~Update(){ }
 
  void response() {
    if(columns->size() || columns_onfractions->size()) {
      commit();
      return;
    }

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
      [ptr=shared_from_this()]() {
        return ptr->result->completion==0;
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
      col->cid, col, 
      col->get_first_key(), 
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
    DB::Cells::ColCells::Ptr  col_cells;
    DB::Cell::Key::Ptr        key_start;
    Update::Ptr               updater;
    ReqBase::Ptr              parent_req;
    const int64_t             rid;
    
    Locator(const Types::Range type, const int64_t cid, 
            DB::Cells::ColCells::Ptr col_cells,
            DB::Cell::Key::Ptr key_start,
            Update::Ptr updater,
            ReqBase::Ptr parent_req=nullptr, const int64_t rid=0) 
            : type(type), cid(cid), col_cells(col_cells), 
              key_start(key_start), 
              updater(updater), parent_req(parent_req), rid(rid) {
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
      s.append(" ");
      s.append(col_cells->to_string());
      s.append(" ");
      s.append(key_start->to_string());
      return s;
    }

    void locate_on_manager() {

      Mngr::Params::RgrGetReq params(1);
      
      params.range_begin.copy(*key_start.get());
      if(cid > 2)
        params.range_begin.insert(0, std::to_string(cid));
      if(cid >= 2)
        params.range_begin.insert(0, "2");

      //std::cout << "locate_on_manager:\n " 
      //          << to_string() << "\n "
      //          << params.to_string() << "\n";

      updater->result->completion++;

      Mngr::Req::RgrGet::request(
        params,
        [ptr=shared_from_this()]
        (ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {
          if(ptr->located_on_manager(req_ptr, rsp))
            ptr->updater->result->completion--;
        }
      );
    }

    void resolve_on_manager() {
      //std::cout << "resolve_on_manager:\n " << to_string() << "\n";
      // if cid, rid >> cache rsp

      updater->result->completion++;

      Mngr::Req::RgrGet::request(
        Mngr::Params::RgrGetReq(cid, rid),
        [ptr=shared_from_this()]
        (ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {
          if(ptr->located_on_manager(req_ptr, rsp))
            ptr->updater->result->completion--;
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

      if(type == Types::Range::DATA || 
        (type == Types::Range::MASTER && col_cells->cid == 1) ||
        (type == Types::Range::META   && col_cells->cid == 2 )) {
        if(cid != rsp.cid || col_cells->cid != cid) {
          (parent_req == nullptr ? base_req : parent_req)->request_again();
          return false;
          //updater->result->err = Error::NOT_ALLOWED;
          //updater->response();
        }
        commit_data(rsp.endpoints, rsp.rid, rsp.range_end, base_req);
        if(type == Types::Range::DATA)
          return true;
      
      } else 
        std::make_shared<Locator>(
          type, rsp.cid, col_cells, key_start, updater, 
          base_req, rsp.rid
        )->locate_on_ranger(rsp.endpoints);

      if(!rsp.range_end.empty() && !rsp.next_range_begin.empty()) {
        //std::cout << "located_on_manager, NEXT-KEY: " 
        //          << Types::to_string(type) 
        //          << " " << rsp.range_end.to_string() << "\n";

        auto next_key_start = col_cells->get_key_next(rsp.range_end);
        if(next_key_start != nullptr) {
          std::make_shared<Locator>(
            type, cid, col_cells, next_key_start, updater, 
            parent_req == nullptr ? base_req : parent_req
          )->locate_on_manager();
        }
      }
      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints) {
      HT_ASSERT(type != Types::Range::DATA);

      Rgr::Params::RangeLocateReq params(cid, rid);

      params.range_begin.copy(*key_start.get());
      params.range_begin.insert(0, std::to_string(col_cells->cid));
      if(type == Types::Range::MASTER && col_cells->cid > 2) 
        params.range_begin.insert(0, "2");

      //std::cout << "locate_on_ranger:\n "
      //          << to_string() << "\n "
      //          << params.to_string() << "\n";
      
      updater->result->completion++;

      Rgr::Req::RangeLocate::request(
        params, 
        endpoints, 
        [ptr=shared_from_this()]() {
          ptr->parent_req->request_again();
        },
        [endpoints, ptr=shared_from_this()] 
        (ReqBase::Ptr req_ptr, Rgr::Params::RangeLocateRsp rsp) {
          if(ptr->located_on_ranger(endpoints, req_ptr, rsp))
            ptr->updater->result->completion--;
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
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        parent_req->request_again();
        return false;
      }
      if(!rsp.rid 
      ||(type == Types::Range::DATA && rsp.cid != col_cells->cid)) {
        //std::cout << "RETRYING " << rsp.to_string() << "\n";
        parent_req->request_again();
        return false;
      }

      updater->result->err=rsp.err;
      if(rsp.err){

        return true;
      }

      /*
      auto range_next = get_key_next(key_start, true);
      if(range_next == nullptr) 
        return true;
      */

      std::make_shared<Locator>(
        type == Types::Range::MASTER
        ? Types::Range::META : Types::Range::DATA,
        rsp.cid, col_cells, key_start, updater, parent_req, rsp.rid
      )->resolve_on_manager();

      if(!rsp.range_end.empty() && !rsp.next_range_begin.empty()) {
        //std::cout << "located_on_ranger, NEXT-KEY: " 
        //  << Types::to_string(type) 
        //   << " " << rsp.range_end.to_string() << "\n";

        auto next_key_start = col_cells->get_key_next(rsp.range_end);
        if(next_key_start != nullptr) {
          std::make_shared<Locator>(
            type, cid, col_cells, next_key_start, updater, parent_req
          )->locate_on_ranger(endpoints);
        }
      }
      return true;
    }

    void commit_data(EndPoints endpoints, uint64_t rid,
                     const DB::Cell::Key& key_end,
                     const ReqBase::Ptr& base_req) {
      //std::cout << "Query::Update commit_data " 
      //          << col_cells->to_string() << " rid=" << rid << "\n"; 
              
      bool more = true;
      DynamicBuffer::Ptr cells_buff;
      while(more && 
           (cells_buff = col_cells->get_buff(
             *key_start.get(), key_end, updater->buff_sz, more)) != nullptr) {
        
        updater->result->completion++;
        /*
        std::cout << "Query::Update commit_data:\n sz=" << cells_buff->fill() 
                  << " more=" << more 
                  << " " << col_cells->to_string() << " rid=" << rid
                  << " key_start=" << key_start->to_string() 
                  << " key_end=" << key_end.to_string() 
                  << "\n"; 
        */
        Rgr::Req::RangeQueryUpdate::request(
          Rgr::Params::RangeQueryUpdateReq(col_cells->cid, rid), 
          cells_buff, 
          endpoints, 
          [cells_buff, base_req, ptr=shared_from_this()]() {
            ptr->col_cells->add(*cells_buff.get());
            base_req->request_again();
            --ptr->updater->result->completion;
            //std::cout << "RETRYING NO-CONN\n";
          },
          [cells_buff, base_req, ptr=shared_from_this()] 
          (ReqBase::Ptr req_ptr, Rgr::Params::RangeQueryUpdateRsp rsp) {

            //std::cout << "commit_data, Rgr::Req::RangeQueryUpdate: "
            //  << rsp.to_string() 
            //  << " completion=" 
            //  << ptr->updater->result->completion.load() << "\n";

            if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
              ptr->col_cells->add(*cells_buff.get());
              base_req->request_again();
              --ptr->updater->result->completion;
              //std::cout << "RETRYING " << rsp.to_string() << "\n";
              return;
            }        
             // ? if(rsp.range_split) 

            // cb(col) at !cells 
            ptr->updater->result->err=rsp.err;
            if(!--ptr->updater->result->completion)
              ptr->updater->response();
          },
          updater->timeout_commit
        );
      } while(more);
    }

  };

};


}}}}}

#endif // swc_lib_db_protocol_common_req_QueryUpdate_h
