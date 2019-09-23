/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_manager_handlers_RangeGetRs_h
#define swc_app_manager_handlers_RangeGetRs_h

#include "swcdb/lib/db/Protocol/params/MngrRangeGetRs.h"


namespace SWC { namespace server { namespace Mngr {

namespace Handler {


class RangeGetRs : public AppHandler {
  public:

  RangeGetRs(ConnHandlerPtr conn, EventPtr ev)
            : AppHandler(conn, ev){ }

  void run() override {

    int err = Error::OK;
    
    try {

      const uint8_t *ptr = m_ev->payload;
      size_t remain = m_ev->payload_len;

      Protocol::Params::MngrRangeGetRsReq params;
      params.decode(&ptr, &remain);
      int cid = params.by==Protocol::Params::MngrRangeGetRsReq::By::RID ? 
                params.cid : 1;

      if(!Env::MngrRole::get()->is_active(cid)){
        std::cout << "MNGR NOT ACTIVE: cid=" << cid << "\n";
        err = Error::MNGR_NOT_ACTIVE;
        goto send_error;
      }
      auto col = Env::MngrColumns::get()->get_column(err, cid, false);
      if(err != Error::OK)
        goto send_error;

      col->state(err);
      if(err != Error::OK)
        goto send_error;
      
      DB::Specs::Key next_key;
      server::Mngr::RangePtr range;
      if(params.by == Protocol::Params::MngrRangeGetRsReq::By::RID){
        range = col->get_range(err, params.rid);

      } else {
        std::string col_id(std::to_string(params.cid)); 
        col_id.append(":");
        if(params.by == Protocol::Params::MngrRangeGetRsReq::By::INTERVALS){
          /*
          params.intervals.keys_start.keys[0] = ScanSpecs::Key(
            col_id.c_str(), col_id.length(), Condition::GE);
          params.intervals.keys_finish.keys[0] = ScanSpecs::Key(
            col_id.c_str(), col_id.length(), Condition::LE);
          */
          range = col->get_range(err, params.interval, next_key);

        } else {
          //keys.keys[0].key = col_id.c_str();
          //keys.keys[0].len = col_id.length();
          
          range = col->get_range(err, params.interval, next_key);
        }
      }

      if(range == nullptr) {
        err = Error::RANGE_NOT_FOUND;
        goto send_error;
      }

      EndPoints endpoints;
      Env::RangeServers::get()->rs_get(range->get_rs_id(), endpoints);
      if(endpoints.empty()) {
        err = Error::RS_NOT_READY;
        goto send_error;
      }
      
      Protocol::Params::MngrRangeGetRsRsp rsp_params(
        range->cid, range->rid, endpoints
      );
      if(cid==1 && !next_key.empty())
        rsp_params.next_key = next_key;
      
      CommHeader header;
      header.initialize_from_request_header(m_ev->header);
      CommBufPtr cbp = std::make_shared<CommBuf>(
        header, rsp_params.encoded_length()+4);
      cbp->append_i32(err);
      rsp_params.encode(cbp->get_data_ptr_address());
      m_conn->send_response(cbp);
      return;
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
      err = e.code();
    }
  
    send_error:
      try {
        CommHeader header;
        header.initialize_from_request_header(m_ev->header);
        CommBufPtr cbp = std::make_shared<CommBuf>(header, 4);
        cbp->append_i32(err);
        m_conn->send_response(cbp);
      } catch (Exception &e) {
        HT_ERROR_OUT << e << HT_END;
      }
    
  }

};
  
// flag(if cid==1) 
//      in(cid+intervals)  out(cid + rid + rs-endpoints + ?next) 
// else 
//      in(cid+rid)        out(cid + rid + rs-endpoints)

}}}}

#endif // swc_app_manager_handlers_RangeGetRs_h