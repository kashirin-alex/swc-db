/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeLoaded_h
#define swcdb_ranger_callbacks_RangeLoaded_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLoaded : public Comm::ResponseCallback {
  public:

  RangeLoaded(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev, 
              const cid_t cid, const rid_t rid)
            : Comm::ResponseCallback(conn, ev), cid(cid), rid(rid) {
    Env::Rgr::in_process(1);
  }

  virtual ~RangeLoaded() { }

  void response(int &err) override {
    if(!err && Env::Rgr::is_shuttingdown()) 
      err = Error::SERVER_SHUTTING_DOWN;

    RangePtr range;
    if(!err) {
      range =  Env::Rgr::columns()->get_range(err, cid, rid);
      if(err || !range || !range->is_loaded())
        err = Error::RGR_NOT_LOADED_RANGE;
    }
    if(err)
      goto send_error;
    

    try {
      Protocol::Rgr::Params::RangeLoaded params(range->cfg->key_seq);
      if((params.intval = range->type == DB::Types::Range::MASTER))
        range->get_interval(params.interval);
        
      auto cbp = Comm::Buffers::make(params, 4);
      cbp->header.initialize_from_request_header(m_ev->header);
      cbp->append_i32(err);
      m_conn->send_response(cbp);
      Env::Rgr::in_process(-1);
      return;

    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      err = Error::COMM_SEND_ERROR;
    }
    
    send_error:
      Env::Rgr::columns()->unload_range(err, cid, rid, 
        [berr=err, ptr=shared_from_this()] (int) {
          ptr->send_error(berr, "");
          Env::Rgr::in_process(-1);
        }
      );
    
  }

  private:
  const cid_t cid;
  const rid_t rid;

};


}
}}
#endif // swcdb_ranger_callbacks_RangeLoaded_h
