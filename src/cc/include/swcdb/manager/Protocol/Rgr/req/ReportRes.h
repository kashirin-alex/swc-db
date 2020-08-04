
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_ReportRes_h
#define swc_manager_Protocol_rgr_req_ReportRes_h

#include "swcdb/db/Protocol/Rgr/params/Report.h"

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

class ReportRes : public client::ConnQueue::ReqBase {
  public:

  ReportRes(const Manager::Ranger::Ptr& rgr);
  
  virtual ~ReportRes();

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;
  
  void handle_no_conn() override;

  private:

  Manager::Ranger::Ptr   rgr;
   
};

}}}}

#endif // swc_manager_Protocol_rgr_req_ReportRes_h
