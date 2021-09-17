/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_RangeIsLoaded_h
#define swcdb_manager_Protocol_rgr_req_RangeIsLoaded_h

#include "swcdb/db/Protocol/Rgr/params/RangeIsLoaded.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class RangeIsLoaded : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<RangeIsLoaded> Ptr;

  const Manager::ColumnHealthCheck::RangerCheck::Ptr   checker;
  const Manager::Range::Ptr                            range;

  SWC_CAN_INLINE
  RangeIsLoaded(const Manager::ColumnHealthCheck::RangerCheck::Ptr& a_checker,
                const Manager::Range::Ptr& a_range, uint32_t timeout=60000)
                : client::ConnQueue::ReqBase(
                    Buffers::make(
                      Params::RangeIsLoadedReq(
                        a_range->cfg->cid, a_range->rid
                      ),
                      0,
                      RANGE_IS_LOADED, timeout
                    )
                  ),
                  checker(a_checker), range(a_range) {
  }

  virtual ~RangeIsLoaded() noexcept { }

  bool valid() override {
    return checker->rgr->state == DB::Types::MngrRangerState::ACK &&
           range->assigned() &&
           Env::Mngr::rangers()->running();
  }

  void handle_no_conn() override {
    checker->handle(range, Error::COMM_CONNECT_ERROR, 0);
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Params::RangeIsLoadedRsp rsp_params(ev->error);
    if(!rsp_params.err) {
      try {
        const uint8_t *ptr = ev->data.base;
        size_t remain = ev->data.size;
        rsp_params.decode(&ptr, &remain);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
        rsp_params.err = e.code();
      }
    }
    checker->handle(range, rsp_params.err, rsp_params.flags);
  }

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_RangeIsLoaded_h
