/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h
#define swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h


#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"
#include "swcdb/db/client/Query/Update/BrokerCommitter.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class Committer_CellsUpdate: public client::ConnQueue::ReqBase {
  public:

  static void request(
        const SWC::client::Query::Update::BrokerCommitter::Ptr& committer,
        DynamicBuffer&& buffer) {
    StaticBuffer snd_buf(buffer.base, buffer.fill(), false);
    std::make_shared<Committer_CellsUpdate>(
      committer,
      std::move(buffer),
      Buffers::make(
        Params::CellsUpdateReq(committer->colp->get_cid()),
        snd_buf, 0, CELLS_UPDATE,
        committer->hdlr->timeout + buffer.fill()/committer->hdlr->timeout_ratio
      )
    )->run();
  }

  typedef std::shared_ptr<Committer_CellsUpdate>    Ptr;
  SWC::client::Query::Update::BrokerCommitter::Ptr  committer;
  DynamicBuffer                                     buffer;
  SWC::client::Query::Profiling::Component::Start   profile;

  Committer_CellsUpdate(
        const SWC::client::Query::Update::BrokerCommitter::Ptr& committer,
        DynamicBuffer&& buffer,
        Buffers::Ptr&& cbp) noexcept
        : client::ConnQueue::ReqBase(std::move(cbp)),
          committer(committer), buffer(std::move(buffer)),
          profile(committer->hdlr->profile.bkr()) {
  }

  virtual ~Committer_CellsUpdate() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  SWC::client::Brokers::BrokerIdx _bkr_idx;
};


}}}}}



#endif // swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h
