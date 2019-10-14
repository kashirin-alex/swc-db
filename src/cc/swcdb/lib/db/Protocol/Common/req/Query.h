
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


namespace SWC { namespace Protocol { namespace Common { namespace Req { 
  
namespace Query {

/*
range-master: 
  req-mngr.   cid(1) + n(cid):Specs::Interval        => cid(1) + rid + rgr(endpoints) + ?next	
    req-rgr.  cid(1) + rid + cid(n):Specs::Interval  => cid(2) + rid + ?next	
range-meta: 
  req-mngr.   cid(2) + rid                           => cid(2) + rid + rgr(endpoints)	
    req-rgr.  cid(2) + rid + cid(n):Specs::Interval  => cid(n) + rid + ?next	
range-data: 
  req-mngr.   cid(n) + rid                           => cid(n) + rid + rgr(endpoints)	
    req-rgr.  cid(n) + rid + Specs::Interval         => results
*/
 
namespace Result{
struct Update{
  typedef std::shared_ptr<Update> Ptr;
  int err;
  std::atomic<uint32_t> completion = 0;
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
    DB::Cells::MapMutable::ColumnCells pair;
    for(size_t idx=0;columns_cells->get(idx, pair);idx++)
      locate_ranger_master(pair.first, pair.second);
  }
  
  void locate_ranger_master(const int64_t cid, DB::Cells::Mutable::Ptr cells) {
    //range-locator (master) : cid(1)+(cid[N]+Cell::Key) => cid(1), rid(N-master), rgr-endpoints 
    
    DB::Specs::Interval::Ptr intval = DB::Specs::Interval::make_ptr();
    DB::Cells::Cell cell;
    cells->get(0, cell);
    intval->key_start.set(cell.key, Condition::GE);
    cells->get(-1, cell);
    intval->key_finish.set(cell.key, Condition::LE);;

    DB::Specs::Interval::Ptr intval_cells = DB::Specs::Interval::make_ptr(intval);

    intval->key_start.insert(0, std::to_string(cid), Condition::GE);
    intval->key_finish.insert(0, std::to_string(cid), Condition::LE);
    locate_ranger_master(cid, cells, intval, intval_cells);
  }

  void locate_ranger_master(const int64_t cid, DB::Cells::Mutable::Ptr cells,
                            DB::Specs::Interval::Ptr intval, 
                            DB::Specs::Interval::Ptr intval_cells) {
    std::cout << intval->to_string() << "\n";
    
    result->completion++;

    Mngr::Req::RgrGet::request(
      Mngr::Params::RgrGetReq(1, intval),
      [cid, cells, intval, intval_cells, ptr=shared_from_this()]
      (Req::ConnQueue::ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {

        if(rsp.err != Error::OK){
          if(rsp.err == Error::COLUMN_NOT_EXISTS 
            || rsp.err == Error::RANGE_NOT_FOUND) {
            //std::cout << "NO-RETRY \n";
          } else {
            std::cout << "RETRYING " << rsp.to_string() << "\n";
            req_ptr->request_again();
            return;
          }
        }

        std::cout << "RgrGet: " << rsp.to_string() << " cid=" << cid  << " " << cells->to_string() << "\n";

        if(cid == 1) {
          if(cid != rsp.cid) {
            ptr->result->err=Error::NOT_ALLOWED;
            ptr->response();
            ptr->result->completion--;
            return;   
          }
          DB::Specs::Interval::Ptr ci = DB::Specs::Interval::make_ptr(intval_cells);
          ci->key_finish.set(rsp.next_key, Condition::LT);
          ptr->commit_data(rsp.endpoints, rsp.cid, rsp.rid, ci, cells);

        } else {
          ptr->result->completion++;
          Rgr::Params::RangeLocateReq params(rsp.cid, rsp.rid, intval);

          Rgr::Req::RangeLocate::request(
            params, rsp.endpoints, 
            [req_ptr]() {
              req_ptr->request_again();
            },
            [ptr] (Req::ConnQueue::ReqBase::Ptr req_ptr, Rgr::Params::RangeLocateRsp rsp) {
              // locate_ranger_meta(), mngr-> getRanger of rsp.cid+rsp.rid
              std::cout << "Rgr::Req::RangeLocate: " << rsp.to_string() << "\n";
              ptr->result->err=rsp.err;
              if(!--ptr->result->completion)
                ptr->response();          
              }
          );
        }

        if(!rsp.next_key.empty()) {
          // if cells -gt next_key
          DB::Specs::Interval::Ptr intval_nxt = DB::Specs::Interval::make_ptr(intval);
          intval_nxt->key_start.set(rsp.next_key, Condition::GE);

          DB::Specs::Interval::Ptr ci = DB::Specs::Interval::make_ptr();
          ci->key_start.set(rsp.next_key, Condition::LT, 1);
          ci->key_finish.copy(intval_cells->key_finish);
          ptr->locate_ranger_master(cid, cells, intval_nxt, ci);
        }
        
        ptr->result->completion--;
      }
    );
  }

  void locate_ranger_meta() {
  }

  void locate_ranger_data() {
  }
  
  void commit_data(EndPoints rgr, int64_t cid, uint64_t rid,
                   DB::Specs::Interval::Ptr ci, 
                   DB::Cells::Mutable::Ptr cells) {
    bool more;
    do {
   
      DynamicBufferPtr cells_buff = std::make_shared<DynamicBuffer>();     
      more = cells->write_and_free(*ci.get(), *cells_buff.get(), buff_sz);
      std::cout << "Query::Update commit_data: sz=" << cells_buff->fill() << " " << cells->to_string() << "\n"; 
      
      result->completion++;
    
      Rgr::Req::RangeQueryUpdate::request(
        Rgr::Params::RangeQueryUpdateReq(cid, rid, cells_buff->fill()), cells_buff, rgr, 
        [cid, cells, cells_buff, ptr=shared_from_this()]() {
          cells->add(*cells_buff.get());
          ptr->locate_ranger_master(cid, cells);
          --ptr->result->completion;
        },
        [ptr=shared_from_this()] 
        (Req::ConnQueue::ReqBase::Ptr req_ptr, 
         Rgr::Params::RangeQueryUpdateRsp rsp) {

          std::cout << "Rgr::Req::RangeQueryUpdate: " << rsp.to_string() << "\n";        
          ptr->result->err=rsp.err;
          if(!--ptr->result->completion)
            ptr->response();
        }
      );
    } while(more);
  }

  uint32_t buff_sz = 32000000;
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
          (Req::ConnQueue::ReqBase::Ptr req_ptr, 
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
            // --> ci.keys_start = rsp.next_key
          }
        );
      }
    }
  }

};
*/
}}}}}

#endif // swc_lib_db_protocol_req_Callbacks_h
