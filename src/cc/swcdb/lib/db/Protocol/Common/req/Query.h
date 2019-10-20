
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_common_req_Query_h
#define swc_lib_db_protocol_common_req_Query_h

#include "QueryUpdate.h"


namespace SWC { namespace Protocol { namespace Common { namespace Req { 
  
namespace Query {
 
namespace Result{

struct Select{

};
}

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

#endif // swc_lib_db_protocol_common_req_Query_h
