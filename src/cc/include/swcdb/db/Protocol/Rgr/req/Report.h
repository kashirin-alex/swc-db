
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_rgr_req_Report_h
#define swc_db_protocol_rgr_req_Report_h


#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


  
class Report: public Comm::client::ConnQueue::ReqBase {
  public:

  Report(const Comm::EndPoints& endpoints, Params::Report::Function func, 
         const uint32_t timeout);

  Report(const Comm::EndPoints& endpoints, Params::Report::Function func, 
         const Serializable& params, const uint32_t timeout);

  virtual ~Report();

  bool run() override;

  protected:

  Comm::EndPoints   endpoints;

};


  
class ReportRes: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&, 
                             const Params::Report::RspRes&)> Cb_t;

  static void request(const Comm::EndPoints& endpoints, const Cb_t& cb, 
                      const uint32_t timeout = 10000);

  ReportRes(const Comm::EndPoints& endpoints, const Cb_t& cb, 
            const uint32_t timeout);

  virtual ~ReportRes();

  void handle_no_conn() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t    cb;

};



class ReportCids: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&,
                             const Params::Report::RspCids&)> Cb_t;
 
  static void request(const Comm::EndPoints& endpoints, const Cb_t& cb, 
                      const uint32_t timeout = 10000);

  ReportCids(const Comm::EndPoints& endpoints, const Cb_t& cb, 
             const uint32_t timeout);

  virtual ~ReportCids();

  void handle_no_conn() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};



class ReportColumnRids: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&, 
                             const Params::Report::RspColumnRids&)> Cb_t;
 
  static void request(const Comm::EndPoints& endpoints, cid_t cid, const Cb_t& cb,
                      const uint32_t timeout = 10000);

  ReportColumnRids(const Comm::EndPoints& endpoints, cid_t cid, const Cb_t& cb,
                   const uint32_t timeout);

  virtual ~ReportColumnRids();

  void handle_no_conn() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};



class ReportColumnsRanges: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&, 
                             const Params::Report::RspColumnsRanges&)> Cb_t;
 
  static void request(const Comm::EndPoints& endpoints, 
                      const Cb_t& cb,
                      const uint32_t timeout = 10000);

  static void request(const Comm::EndPoints& endpoints, cid_t cid, 
                      const Cb_t& cb,
                      const uint32_t timeout = 10000);

  ReportColumnsRanges(const Comm::EndPoints& endpoints, 
                      const Cb_t& cb, 
                      const uint32_t timeout);

  ReportColumnsRanges(const Comm::EndPoints& endpoints, cid_t cid,
                      const Cb_t& cb, 
                      const uint32_t timeout);

  virtual ~ReportColumnsRanges();

  void handle_no_conn() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};




}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/req/Report.cc"
#endif 

#endif // swc_db_protocol_rgr_req_Report_h
