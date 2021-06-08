/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_ColumnCompact_h
#define swcdb_db_protocol_rgr_req_ColumnCompact_h

#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class ColumnCompact : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<ColumnCompact> Ptr;

  ColumnCompact(cid_t cid)
                : client::ConnQueue::ReqBase(
                    Buffers::make(
                      Params::ColumnCompactReq(cid), 0, COLUMN_COMPACT, 60000)
                  ) { }

  virtual ~ColumnCompact() { }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    if(Params::ColumnCompactRsp(ev->error, ev->data.base, ev->data.size).err)
      return request_again();
  }

  void handle_no_conn() override { }

};

}}}}}


#endif // swcdb_db_protocol_rgr_req_ColumnCompact_h
