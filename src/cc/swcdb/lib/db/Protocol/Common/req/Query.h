
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
    cv.wait(lock_wait, [](){return true;});
  }

  void commit() {
    DB::Cells::MapMutable::ColumnCells pair;
    for(size_t idx=0;columns_cells->get(idx, pair);idx++)
      locate_ranger_master(pair);
  }
  
  void locate_ranger_master(const DB::Cells::MapMutable::ColumnCells& pair) {
    //range-locator (master) : cid(1)+(cid[N]+Cell::Key) => cid(1), rid(N-master), rgr-endpoints 
    DB::Specs::Interval::Ptr cells_interval = DB::Specs::Interval::make_ptr();

    Mngr::Params::RgrGetReq params(1, DB::Specs::Interval::make_ptr());
    DB::Cells::Cell cell;
    
    pair.second->get(0, cell);
    cells_interval->key_start.set(cell.key, Condition::GE);
    params.interval->key_start.set(cell.key, Condition::GE);
    params.interval->key_start.insert(0, std::to_string(pair.first), Condition::GE);

    pair.second->get(-1, cell);
    params.interval->key_finish.set(cell.key, Condition::LE);
    params.interval->key_finish.insert(0, std::to_string(pair.first), Condition::LE);

    std::cout << params.interval->to_string() << "\n";
    
    result->completion++;
    Mngr::Req::RgrGet::request(
      params,
      [cells_interval, 
       pair = DB::Cells::MapMutable::ColumnCells(pair.first, pair.second), 
       intval_loc = params.interval, ptr = shared_from_this()]
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
        ptr->result->completion--;

        std::cout << "RgrGet: " << rsp.to_string() << " cid=" << pair.first  << " " << pair.second->to_string() << "\n";

        if(pair.first == 1) {
          cells_interval->key_finish.set(rsp.next_key, Condition::LT);
          ptr->commit_data(rsp.endpoints, rsp.cid, rsp.rid, cells_interval, pair);
          return;
        }

        ptr->result->completion++;
        Rgr::Params::RangeLocateReq params(rsp.cid, rsp.rid, intval_loc);

        Rgr::Req::RangeLocate::request(
          params, rsp.endpoints, 
          [req_ptr]() {req_ptr->request_again();},
          [ptr] (Req::ConnQueue::ReqBase::Ptr req_ptr, Rgr::Params::RangeLocateRsp rsp) {
            std::cout << "Rgr::Req::RangeLocate: " << rsp.to_string() << "\n";
            ptr->result->err=rsp.err;
            if(!--ptr->result->completion)
              ptr->response();          
            }
        );

      }
    );
  }

  void locate_ranger_meta() {
  }

  void locate_ranger_data() {
  }
  
  void commit_data(EndPoints rgr, int64_t cid, uint64_t rid,
                   DB::Specs::Interval::Ptr interval, 
                   const DB::Cells::MapMutable::ColumnCells& pair) {
    bool more;
    do {
   
      DynamicBufferPtr cells = std::make_shared<DynamicBuffer>();     
      more = pair.second->write_and_free(*interval.get(), *cells.get(), buff_sz);
      std::cout << "Query::Update commit_data: sz=" << cells->fill() << " " << pair.second->to_string() << "\n"; 
      result->completion++;
    
      Rgr::Req::RangeQueryUpdate::request(
        Rgr::Params::RangeQueryUpdateReq(cid, rid, cells->fill()), cells, rgr, 
        [pair, cells, ptr=shared_from_this()]() {
          pair.second->add(*cells.get());
          ptr->locate_ranger_master(pair);
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
