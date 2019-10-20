
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Query_h
#define swc_lib_db_protocol_common_req_Query_h

#include "swcdb/lib/db/Cells/SpecsScan.h"
#include "swcdb/lib/db/Cells/MapMutable.h" 

#include "swcdb/lib/db/Protocol/Mngr/req/RgrGet.h"
#include "swcdb/lib/db/Protocol/Rgr/req/RangeLocate.h"
#include "swcdb/lib/db/Protocol/Rgr/req/RangeQueryUpdate.h"
#include "swcdb/lib/db/Types/Range.h"


namespace SWC { namespace Protocol { namespace Common { namespace Req { 
  
namespace Query {

using ReqBase = Req::ConnQueue::ReqBase;

/*
range-master: 
  req-mngr.   cid(1) + n(cid):Specs::Interval        
              => cid(1) + rid + rgr(endpoints) + key_start + key_end + ?key_next	
    req-rgr.  cid(1) + rid + cid(n):Specs::Interval  
              => cid(2) + rid + key_start + key_end + ?key_next	
range-meta: 
  req-mngr.   cid(2) + rid                           
              => cid(2) + rid + rgr(endpoints)	
    req-rgr.  cid(2) + rid + cid(n):Specs::Interval  
              => cid(n) + rid + key_start + key_end + ?key_next	
range-data: 
  req-mngr.   cid(n) + rid                           
              => cid(n) + rid + rgr(endpoints)	
    req-rgr.  cid(n) + rid + Specs::Interval         
              => results
*/
 
namespace Result{

struct Update{
  typedef std::shared_ptr<Update> Ptr;
  int err;
  std::atomic<uint32_t> completion = 0;
  
  DB::Cells::MapMutable::Ptr  errored;
};

struct Select{

};
}
  

class Update : public std::enable_shared_from_this<Update> {
  public:

  using Result = Result::Update;

  typedef std::shared_ptr<Update>           Ptr;
  typedef std::function<void(Result::Ptr)>  Cb_t;
  
  Cb_t                        cb;
  DB::Cells::MapMutable::Ptr  columns_cells;
  Result::Ptr                 result;

  std::mutex                  m_mutex;
  std::condition_variable     cv;

  Update(Cb_t cb=0)
        : cb(cb),
          columns_cells(std::make_shared<DB::Cells::MapMutable>()),
          result(std::make_shared<Result>()) { }

  Update(DB::Cells::MapMutable::Ptr columns_cells, Cb_t cb=0)
        : cb(cb), columns_cells(columns_cells), 
          result(std::make_shared<Result>()) { }

  virtual ~Update(){ }
 
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

  void commit() {
    DB::Cells::Cell     cell;

    DB::Cells::MapMutable::ColumnCells pair;
    for(size_t idx=0;columns_cells->get(idx, pair);idx++) {
      if(pair.second->size() == 0) 
        continue;
      auto cid = pair.first;
      auto& cells = pair.second;

      cells->get(0, cell);
  
      std::make_shared<Locator>(
        Types::Range::MASTER,
        cid, cells, cid, 
        std::make_shared<DB::Cell::Key>(cell.key), 
        shared_from_this()
      )->locate_on_manager();
    }
  }

  class Locator : public std::enable_shared_from_this<Locator> {
    public:
    const Types::Range        type;
    const int64_t             cid;
    DB::Cells::Mutable::Ptr   cells;
    const int64_t             cells_cid;
    DB::Cell::Key::Ptr        key_start;
    Update::Ptr               updater;
    ReqBase::Ptr              parent_req;
    const int64_t             rid;
    
    Locator(const Types::Range type, const int64_t cid, 
            DB::Cells::Mutable::Ptr cells, const int64_t cells_cid,
            DB::Cell::Key::Ptr key_start,
            Update::Ptr updater,
            ReqBase::Ptr parent_req=nullptr, const int64_t rid=0) 
            : type(type), cid(cid), cells(cells), cells_cid(cells_cid), 
              key_start(key_start), 
              updater(updater), parent_req(parent_req), rid(rid) {
      std::cout << " LOCATOR, " << to_string() <<"\n";
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
      s.append(" cells_cid=");
      s.append(std::to_string(cells_cid));
      s.append(" ");
      s.append(cells->to_string());
      s.append(" ");
      s.append(key_start->to_string());
      return s;
    }

    void locate_on_manager() {

      Mngr::Params::RgrGetReq params(1);
      params.interval.key_start.set(*key_start.get(), Condition::GE);
      if(cid > 2)
        params.interval.key_start.insert(0, std::to_string(cid), Condition::GE);
      if(cid >= 2)
        params.interval.key_start.insert(0, "2", Condition::EQ);
      if(cid == 1) 
        params.interval.key_start.set(0, Condition::EQ);
      
      std::cout << "locate_on_manager:\n " 
                << to_string() << "\n "
                << params.to_string() << "\n";

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
      std::cout << "resolve_on_manager:\n " << to_string() << "\n";
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

    bool located_on_manager(const ReqBase::Ptr& base_req, const Mngr::Params::RgrGetRsp& rsp) {

      std::cout << "located_on_manager:\n " << to_string() 
                << "\n " << rsp.to_string() << "\n";
      
      if(rsp.err != Error::OK){
        // err type? ~| parent_req->request_again();
        if(rsp.err == Error::COLUMN_NOT_EXISTS 
          || rsp.err == Error::RANGE_NOT_FOUND) {
          //std::cout << "NO-RETRY \n";
          return true;
        } else {
          std::cout << "RETRYING " << rsp.to_string() << "\n";
          base_req->request_again();
          return false;
        }
      }
      if(rsp.rid == 0) {
        std::cout << "RETRYING " << rsp.to_string() << "\n";
        (parent_req == nullptr ? base_req : parent_req)->request_again();
        return false;
      }

       // (rsp.key_start lacks importance, needs to be key_start)

      if(type == Types::Range::DATA) {
        // cells_cid != rsp.cid
        updater->commit_data(
          rsp.endpoints, cells_cid, rsp.rid, 
          *key_start.get(), rsp.key_end,  
          cells, base_req);
        return true;
      }

      if((type == Types::Range::MASTER && cells_cid == 1) ||
         (type == Types::Range::META   && cells_cid == 2 )) {
        if(cid != rsp.cid || cells_cid != cid) {
          updater->result->err = Error::NOT_ALLOWED;
          updater->response();
        } else {
          updater->commit_data(
            rsp.endpoints, cells_cid, rsp.rid, 
            *key_start.get(), rsp.key_end,
            cells, base_req);
        }
      } else {
        std::make_shared<Locator>(
          type, rsp.cid, cells, cells_cid, key_start, updater, base_req, rsp.rid
        )->locate_on_ranger(rsp.endpoints);
      }

      if(rsp.next_key) {
        auto key_next = get_key_next(rsp.key_end);
        if(key_next != nullptr) {
          std::make_shared<Locator>(
            type, cid, cells, cells_cid, key_next, updater, 
            parent_req == nullptr ? base_req : parent_req
          )->locate_on_manager();
        }
      }
      return true;
    }

    void locate_on_ranger(const EndPoints& endpoints) {
      HT_ASSERT(type != Types::Range::DATA);

      Rgr::Params::RangeLocateReq params(cid, rid);

      params.interval.key_start.set(*key_start.get(), Condition::GE);
      params.interval.key_start.insert(0, std::to_string(cells_cid),
        type == Types::Range::MASTER? Condition::GE : Condition::EQ);
      if(type == Types::Range::MASTER && cells_cid > 2) 
        params.interval.key_start.insert(0, "2", Condition::EQ);

      std::cout << "locate_on_ranger:\n "
                << to_string() << "\n "
                << params.to_string() << "\n";
      
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
      std::cout << "located_on_ranger:\n " << to_string() 
                << "\n " << rsp.to_string() << "\n";

      if(rsp.err == Error::RS_NOT_LOADED_RANGE
      || rsp.err == Error::RANGE_NOT_FOUND) {
        parent_req->request_again();
        return false;
      }
      if(rsp.rid == 0 
      ||(type == Types::Range::DATA && rsp.cid != cells_cid)) {
        std::cout << "RETRYING " << rsp.to_string() << "\n";
        parent_req->request_again();
        return false;
      }

      updater->result->err=rsp.err;
      if(rsp.err){

        return true;
      }

      /*
      auto key_next = get_key_next(key_start, true);
      if(key_next == nullptr) 
        return true;
      */

      std::make_shared<Locator>(
        type == Types::Range::MASTER
        ? Types::Range::META : Types::Range::DATA,
        rsp.cid, cells, cells_cid, key_start, updater, parent_req, rsp.rid
      )->resolve_on_manager();

      if(rsp.next_key) {
        auto key_next = get_key_next(rsp.key_end);
        if(key_next != nullptr) {
          std::make_shared<Locator>(
            type, cid, cells, cells_cid, key_next, updater, parent_req
          )->locate_on_ranger(endpoints);
        }
      }
      return true;
    }
    
    DB::Cell::Key::Ptr get_key_next(const DB::Cell::Key& eval_key, 
                                    bool start_key=false) {
      DB::Cell::Key key(eval_key);
      if(type != Types::Range::DATA) 
        key.remove(0);
      if(type == Types::Range::MASTER) 
        key.remove(0);
      DB::Cells::Cell cell;
      if(!cells->get(key, start_key? Condition::GE : Condition::GT, cell))
        return nullptr;
      return std::make_shared<DB::Cell::Key>(cell.key);
    }

  };


  void commit_data(EndPoints endpoints, 
                   int64_t cid, uint64_t rid,
                   const DB::Cell::Key& key_start, 
                   const DB::Cell::Key& key_end, 
                   DB::Cells::Mutable::Ptr cells,
                   const ReqBase::Ptr& base_req) {
    bool more;
    do {
   
      DynamicBufferPtr cells_buff = std::make_shared<DynamicBuffer>();     
      more = cells->write_and_free(key_start, key_end, *cells_buff.get(), buff_sz);
      std::cout << "Query::Update commit_data:\n sz=" << cells_buff->fill() 
                << " more=" << more << " cid=" << cid << " rid=" << rid
                << " key_start=" << key_start.to_string() << " key_end=" << key_end.to_string() 
                << " " << cells->to_string() << "\n"; 
      
      result->completion++;
    
      Rgr::Req::RangeQueryUpdate::request(
        Rgr::Params::RangeQueryUpdateReq(cid, rid, cells_buff->fill()), 
        cells_buff, endpoints, 
        [cells, cells_buff, base_req, ptr=shared_from_this()]() {
          cells->add(*cells_buff.get());
          base_req->request_again();
          --ptr->result->completion;
            std::cout << "RETRYING NO-CONN\n";
        },
        [cells, cells_buff, base_req, ptr=shared_from_this()] 
        (ReqBase::Ptr req_ptr, Rgr::Params::RangeQueryUpdateRsp rsp) {

          std::cout << "commit_data, Rgr::Req::RangeQueryUpdate: " << rsp.to_string() 
                    << " completion=" << ptr->result->completion.load() << "\n";
          if(rsp.err == Error::RS_NOT_LOADED_RANGE) {
            cells->add(*cells_buff.get());
            base_req->request_again();
            --ptr->result->completion;
            std::cout << "RETRYING " << rsp.to_string() << "\n";
            return;
          }        
          ptr->result->err=rsp.err;
          if(!--ptr->result->completion)
            ptr->response();
        },
        timeout_commit
      );
    } while(more);
  }

  uint32_t buff_sz = 8000000;
  uint32_t timeout_commit = 60000;
};






/*
class Select : std::enable_shared_from_this<Select> {
  public:
  
  typedef std::shared_ptr<Select>                    Ptr;
  typedef std::function<void(Query::Result::Select)> Cb_t;
  DB::Specs::Scan                                    specs;

  Select() { }

  Select(const DB::Specs::Scan &specs): specs(specs) { }

  virtual ~Select(){ }


  void get_rgr_master(){
      //range-locator (master) : cid+cells_interval => cid(1), rid(master), rgr-endpoints 

    for(auto &col : specs.columns){

      for(auto &cells_interval : col.cells_interval){
        Mngr::Req::RgrGet::request(
          col.cid, cells_interval, 
          [cid=col.cid, ci=cells_interval]
          (ReqBase::Ptr req_ptr, 
          int err, Mngr::Params::RgrGetRsp rsp) {
            if(err != Error::OK){
              std::cout << "get Ranger-master err="<< err << "("<<Error::get_text(err) <<  ")";
              if(err == Error::COLUMN_NOT_EXISTS || 
                err == Error::RANGE_NOT_FOUND) {
                std::cout << "NO-RETRY \n";
              } else {
                std::cout << "RETRYING \n";
                req_ptr->request_again();
              }
              return;
            }
            // req. rgr(master-range) (cid+ci)
            std::cout << "get Ranger-master " << rsp.to_string() << "\n";
            // --> ci.keys_start = rsp.key_next
          }
        );
      }
    }
  }

};
*/
}}}}}

#endif // swc_lib_db_protocol_req_Callbacks_h
