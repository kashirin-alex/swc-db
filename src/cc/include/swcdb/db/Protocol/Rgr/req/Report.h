/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_Report_h
#define swcdb_db_protocol_rgr_req_Report_h


#include "swcdb/db/Protocol/Rgr/params/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {



class Report: public client::ConnQueue::ReqBase {
  public:

  Report(const SWC::client::Clients::Ptr& clients,
         const EndPoints& endpoints,
         Params::Report::Function func,
         const uint32_t timeout);

  Report(const SWC::client::Clients::Ptr& clients,
         const EndPoints& endpoints,
         Params::Report::Function func,
         const Serializable& params,
         const uint32_t timeout);

  virtual ~Report() { }

  bool run() override;

  protected:
  SWC::client::Clients::Ptr clients;
  EndPoints                 endpoints;

};



class ReportRes: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspRes&)> Cb_t;
  typedef std::shared_ptr<ReportRes> Ptr;

  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    Ptr(new ReportRes(
      clients, endpoints, std::move(cb), timeout))->run();
  }

  ReportRes(const SWC::client::Clients::Ptr& clients,
            const EndPoints& endpoints,
            Cb_t&& cb,
            const uint32_t timeout);

  virtual ~ReportRes() { }

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t    cb;

};



class ReportCids: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspCids&)> Cb_t;
  typedef std::shared_ptr<ReportCids> Ptr;

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    Ptr(new ReportCids(
      clients, endpoints, std::move(cb), timeout))->run();
  }

  ReportCids(const SWC::client::Clients::Ptr& clients,
             const EndPoints& endpoints,
             Cb_t&& cb,
             const uint32_t timeout);

  virtual ~ReportCids() { }

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};



class ReportColumnRids: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspColumnRids&)> Cb_t;
  typedef std::shared_ptr<ReportColumnRids> Ptr;

  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      cid_t cid,
                      Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    Ptr(new ReportColumnRids(
      clients, endpoints, cid, std::move(cb), timeout))->run();
  }

  ReportColumnRids(const SWC::client::Clients::Ptr& clients,
                   const EndPoints& endpoints,
                   cid_t cid,
                   Cb_t&& cb,
                   const uint32_t timeout);

  virtual ~ReportColumnRids() { }

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};



class ReportColumnsRanges: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspColumnsRanges&)> Cb_t;
  typedef std::shared_ptr<ReportColumnsRanges> Ptr;

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    Ptr(new ReportColumnsRanges(
      clients, endpoints, std::move(cb), timeout))->run();
  }

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      cid_t cid,
                      Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    Ptr(new ReportColumnsRanges(
      clients, endpoints, cid, std::move(cb), timeout))->run();
  }


  ReportColumnsRanges(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      Cb_t&& cb,
                      const uint32_t timeout);

  ReportColumnsRanges(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      cid_t cid,
                      Cb_t&& cb,
                      const uint32_t timeout);

  virtual ~ReportColumnsRanges() { }

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};




}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/req/Report.cc"
#endif

#endif // swcdb_db_protocol_rgr_req_Report_h
