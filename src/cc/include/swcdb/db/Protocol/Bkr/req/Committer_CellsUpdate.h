/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h
#define swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h


#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"
#include "swcdb/db/client/Query/Update/BrokerCommitter.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class Committer_CellsUpdate: public client::ConnQueue::ReqBase {
  public:

  static void request(
        const SWC::client::Query::Update::BrokerCommitter::Ptr& committer,
        const DynamicBuffer::Ptr& buffer) {
    std::make_shared<Committer_CellsUpdate>(committer, buffer)->run();
  }

  typedef std::shared_ptr<Committer_CellsUpdate>    Ptr;
  SWC::client::Query::Update::BrokerCommitter::Ptr  committer;
  DynamicBuffer::Ptr                                buffer;
  SWC::client::Query::Profiling::Component::Start   profile;

  Committer_CellsUpdate(
        const SWC::client::Query::Update::BrokerCommitter::Ptr& committer,
        const DynamicBuffer::Ptr& buffer);

  virtual ~Committer_CellsUpdate() { }

  void handle_no_conn() override;

  bool run() override;

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override;

  private:
  SWC::client::Brokers::BrokerIdx _bkr_idx;
};


}}}}}



#endif // swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h
