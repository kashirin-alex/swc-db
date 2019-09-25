
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_Query_h
#define swc_lib_db_protocol_req_Query_h

#include "swcdb/lib/db/Protocol/req/MngrRsGet.h"
#include "swcdb/lib/db/Cells/SpecsScan.h"
#include "swcdb/lib/db/Cells/Serialized.h"

namespace SWC { namespace Protocol { namespace Req { namespace Query {

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
    locate_rangeservers_master();
  }
  
  void locate_rangeservers_master(){
    //range-locator (master) : cid(1)+(cid[N]+Cell::Key) => cid(1), rid(N-master), rs-endpoints 

    DB::Cells::Serialized::Pair pair;
    for(;;) {
      cells->pop(pair);
      if(pair.first == 0)
        break;

      Protocol::Params::MngrRsGetReq params(1, pair.second.interval);
      params.interval->key_start.insert(0, std::to_string(pair.first), Condition::GE);
      params.interval->key_finish.insert(0, std::to_string(pair.first), Condition::LE);
      std::cout << params.interval->to_string() << "\n";
      
      Protocol::Req::MngrRsGet::request(
        params,
        [pair=pair, ptr=shared_from_this()]
        (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr, Protocol::Params::MngrRsGetRsp rsp) {
          std::cout << "get RS-master " << rsp.to_string() << "\n";

          std::cout << pair.second.interval->to_string() << "\n";

          if(rsp.err != Error::OK){
            if(rsp.err == Error::COLUMN_NOT_EXISTS ||  rsp.err == Error::RANGE_NOT_FOUND) {
              std::cout << "NO-RETRY \n";
            } else {
              std::cout << "RETRYING \n";
              req_ptr->request_again();
              return;
            }
          }

          // req. rs(master-range) (cid+ci)
          // --> ci.keys_start = rsp.next_key
            
          Result r;
          ptr->cb(r);
        }
      );
    }

  }
  
  void locate_rangeservers_meta() {
  }

  void locate_rangeservers_data() {
  }
  
  void commit_data(EndPoints rs) {

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


  void get_rs_master(){
      //range-locator (master) : cid+cells_interval => cid(1), rid(master), rs-endpoints 

    for(auto &col : specs.columns){

      for(auto &cells_interval : col.cells_interval){
        Protocol::Req::MngrRsGet::request(
          col.cid, cells_interval, 
          [cid=col.cid, ci=cells_interval]
          (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr, 
          int err, Protocol::Params::MngrRsGetRsp rsp) {
            if(err != Error::OK){
              std::cout << "get RS-master err="<< err << "("<<Error::get_text(err) <<  ")";
              if(err == Error::COLUMN_NOT_EXISTS || 
                err == Error::RANGE_NOT_FOUND) {
                std::cout << "NO-RETRY \n";
              } else {
                std::cout << "RETRYING \n";
                req_ptr->request_again();
              }
              return;
            }
            // req. rs(master-range) (cid+ci)
            std::cout << "get RS-master " << rsp.to_string() << "\n";
            // --> ci.keys_start = rsp.next_key
          }
        );
      }
    }
  }

};
*/
}}}}

#endif // swc_lib_db_protocol_req_Callbacks_h
