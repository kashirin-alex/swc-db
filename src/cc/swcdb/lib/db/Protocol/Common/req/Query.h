
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Query_h
#define swc_lib_db_protocol_common_req_Query_h

#include "swcdb/lib/db/Cells/SpecsScan.h"
#include "swcdb/lib/db/Cells/Serialized.h"

#include "swcdb/lib/db/Protocol/Rgr/req/RangeLocate.h"
#include "swcdb/lib/db/Protocol/Mngr/req/RgrGet.h"

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

};
struct Select{

};
}

class Update : public std::enable_shared_from_this<Update> {
  public:

  using Result = Result::Update;

  typedef std::shared_ptr<Update>                    Ptr;
  typedef std::function<void(Query::Result::Update)> Cb_t;
  
  Cb_t                        cb;
  DB::Cells::Serialized::Ptr  cells;


  Update(Cb_t cb=0)
        : cb(cb), cells(std::make_shared<DB::Cells::Serialized>()) { }

  Update(DB::Cells::Serialized::Ptr cells, Cb_t cb=0)
        : cells(cells), cb(cb) { }

  virtual ~Update(){ }


  void commit() {
    locate_ranger_master();
  }
  
  void locate_ranger_master(){
    //range-locator (master) : cid(1)+(cid[N]+Cell::Key) => cid(1), rid(N-master), rgr-endpoints 

    DB::Cells::Serialized::Pair pair;
    for(;;) {
      cells->pop(pair);
      if(pair.first == 0)
        break;

      Mngr::Params::RgrGetReq params(1, pair.second.interval);
      params.interval->key_start.insert(0, std::to_string(pair.first), Condition::GE);
      params.interval->key_finish.insert(0, std::to_string(pair.first), Condition::LE);
      //std::cout << params.interval->to_string() << "\n";
      
      Mngr::Req::RgrGet::request(
        params,
        [pair=pair, intval=params.interval, ptr=shared_from_this()]
        (Req::ConnQueue::ReqBase::Ptr req_ptr, Mngr::Params::RgrGetRsp rsp) {

          //std::cout << pair.second.interval->to_string() << "\n";

          if(rsp.err != Error::OK){
            if(rsp.err == Error::COLUMN_NOT_EXISTS ||  rsp.err == Error::RANGE_NOT_FOUND) {
              //std::cout << "NO-RETRY \n";
            } else {
              std::cout << "RETRYING " << rsp.to_string() << "\n";
              req_ptr->request_again();
              return;
            }
          }

          std::cout << "RgrGet: " << rsp.to_string() << "\n";
          // req. rgr(master-range) (cid+ci)
          // --> ci.keys_start = rsp.next_key

          Rgr::Params::RangeLocateReq params(rsp.cid, rsp.rid, intval);

          Rgr::Req::RangeLocate::request(
            params, rsp.endpoints, 
            [req_ptr]() {req_ptr->request_again();},
            [ptr] (Req::ConnQueue::ReqBase::Ptr req_ptr, Rgr::Params::RangeLocateRsp rsp) {
              std::cout << "Rgr::Req::RangeLocate: " << rsp.to_string() << "\n";
              Result r;
              ptr->cb(r);
            }
          );
          
          
        }
      );
    }

  }
  
  void locate_ranger_meta() {
  }

  void locate_ranger_data() {
  }
  
  void commit_data(EndPoints rgr) {

  }

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
