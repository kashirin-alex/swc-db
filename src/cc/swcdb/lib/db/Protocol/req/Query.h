
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_Query_h
#define swc_lib_db_protocol_req_Query_h

#include "swcdb/lib/db/Cells/Serialized.h"
#include "swcdb/lib/db/Cells/SpecsScan.h"

namespace SWC { namespace Protocol { namespace Req { namespace Query {

namespace Result{
struct Update{

};
struct Select{

};
}

class Update : std::enable_shared_from_this<Update> {
  public:
  
  typedef std::shared_ptr<Update>                    Ptr;
  typedef std::function<void(Query::Result::Update)> Cb_t;
  Cells::Serialized::Ptr                             cells;

  Update(Cb_t cb=0): cb(cb) { }

  Update(Cells::Serialized::Ptr cells, Cb_t cb=0):  cells(cells), cb(cb) { }

  virtual ~Update(){ }

  void commit() {
    locate_rangeservers_master();
  }
  
  void locate_rangeservers_master(){
    //range-locator (master) : cid+cells_interval => cid(1), rid(master), rs-endpoints 

    DB::Cells::Serialized::Pair pair;
    DB::Cell::Key key;
    int8_t* ptr;
    int8_t* ptr_key;
    size_t len_key;
    DB::Specs::Key sskey;
    for(;;) {
      cells.pop(pair);
      if(pair.first == 0)
        break;

      ptr = pair.second.base;
      ptr_end = pair.second.ptr
      pair.second.ptr = pair.second.base;

      while(ptr < ptr_end){
        key = DB::Cell::get_keys_and_move(pair.second);

        Protocol::Req::MngrRangeGetRs::request(
          pair.first, key,
          [cid=pair.first, key=key]
          (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr, 
          int err, Protocol::Params::MngrRangeGetRsRsp rsp) {
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
  
  void locate_rangeservers_meta() {
  }

  void locate_rangeservers_data() {
  }
  
  void commit_data(EndPoints rs) {

  }

};

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
        Protocol::Req::MngrRangeGetRs::request(
          col.cid, cells_interval, 
          [cid=col.cid, ci=cells_interval]
          (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr, 
          int err, Protocol::Params::MngrRangeGetRsRsp rsp) {
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

}}}}

#endif // swc_lib_db_protocol_req_Callbacks_h
