
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

  const Manager::ColumnHealthCheck::RangerCheck::Ptr   checker;
  const Manager::Range::Ptr                            range;

  RangeIsLoaded(const Manager::ColumnHealthCheck::RangerCheck::Ptr& checker,
                const Manager::Range::Ptr& range, uint32_t timeout=60000)
                : client::ConnQueue::ReqBase(
                    false,
                    Buffers::make(
                      Params::RangeIsLoaded(range->cfg->cid, range->rid),
                      0,
                      RANGE_IS_LOADED, timeout
                    )
                  ),
                  checker(checker), range(range) {
  }

  virtual ~RangeIsLoaded() { }

  bool valid() override {
    return checker->rgr->state == DB::Types::MngrRangerState::ACK &&
           range->assigned();
  }

  void handle_no_conn() override {
    checker->handle(range, Error::COMM_CONNECT_ERROR);
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    checker->handle(range, ev->response_code());
  }

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_RangeIsLoaded_h
