
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_db_protocol_rgr_req_ColumnCompact_h
#define swcdb_db_protocol_rgr_req_ColumnCompact_h

#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class ColumnCompact : public Comm::client::ConnQueue::ReqBase {
  public:

  ColumnCompact(cid_t cid);
  
  virtual ~ColumnCompact();
  
  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  void handle_no_conn() override;

};

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/req/ColumnCompact.cc"
#endif 

#endif // swcdb_db_protocol_rgr_req_ColumnCompact_h
