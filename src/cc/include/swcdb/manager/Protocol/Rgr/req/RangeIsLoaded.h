
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_RangeIsLoaded_h
#define swc_manager_Protocol_rgr_req_RangeIsLoaded_h

#include "swcdb/db/Protocol/Rgr/params/RangeIsLoaded.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeIsLoaded : public Comm::client::ConnQueue::ReqBase {
  public:
  
  const Manager::ColumnHealthCheck::RangerCheck::Ptr   checker;
  const Manager::Range::Ptr                            range;

  RangeIsLoaded(const Manager::ColumnHealthCheck::RangerCheck::Ptr& checker,
                const Manager::Range::Ptr& range, uint32_t timeout=60000)
                : Comm::client::ConnQueue::ReqBase(false), 
                  checker(checker), range(range) { 
    cbp = Comm::Buffers::make(Params::RangeIsLoaded(range->cfg->cid, range->rid));
    cbp->header.set(RANGE_IS_LOADED, timeout);
  }
  
  virtual ~RangeIsLoaded() { }
  
  bool valid() override {
    return checker->rgr->state == Types::MngrRanger::State::ACK && 
           range->assigned();
  }

  void handle_no_conn() {
    checker->handle(range, Error::COMM_CONNECT_ERROR);
  }

  void handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) override {
    checker->handle(range, ev->response_code());
  }

};

}}}}

#endif // swc_manager_Protocol_rgr_req_RangeIsLoaded_h
