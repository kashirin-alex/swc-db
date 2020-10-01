
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_manager_Protocol_rgr_req_ReportRes_h
#define swcdb_manager_Protocol_rgr_req_ReportRes_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {

class ReportRes : public client::ConnQueue::ReqBase {
  public:

  ReportRes(const Manager::Ranger::Ptr& rgr);
  
  virtual ~ReportRes();

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  Manager::Ranger::Ptr   rgr;
   
};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_ReportRes_h
