
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_req_Report_h
#define swc_db_protocol_req_Report_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/Report.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class Report: public client::ConnQueue::ReqBase {
  public:
  
  Report(const Serializable& params, Params::Report::Function func, 
         const uint32_t timeout);

  virtual ~Report();

  void handle_no_conn() override;

  protected:
  
  void clear_endpoints();

  EndPoints   endpoints;
};


  
class ColumnStatus: public Report {
  public:
  
  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&, 
                             const int&, 
                             const Params::Report::RspColumnStatus&)> Cb_t;
 
  static void request(cid_t cid, const Cb_t& cb, 
                      const uint32_t timeout = 10000);

  static void request(const Params::Report::ReqColumnStatus& params,
                      const Cb_t& cb, const uint32_t timeout = 10000);

  static Ptr make(const Params::Report::ReqColumnStatus& params,
                  const Cb_t& cb, const uint32_t timeout = 10000);

  ColumnStatus(const Params::Report::ReqColumnStatus& params, const Cb_t& cb, 
               const uint32_t timeout);

  virtual ~ColumnStatus();

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;
  cid_t       cid;

};




}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/Report.cc"
#endif 

#endif // swc_db_protocol_req_Report_h
