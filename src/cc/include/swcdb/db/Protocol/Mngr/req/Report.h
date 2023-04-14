/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_Report_h
#define swcdb_db_protocol_req_Report_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class Report: public client::ConnQueue::ReqBase {
  public:

  Report(const SWC::client::Clients::Ptr& clients,
         Params::Report::Function func, const uint32_t timeout);

  Report(const SWC::client::Clients::Ptr& clients,
         const EndPoints& endpoints, Params::Report::Function func,
         const uint32_t timeout);

  Report(const SWC::client::Clients::Ptr& clients,
         const Serializable& params, Params::Report::Function func,
         const uint32_t timeout);

  Report(Report&&) = delete;
  Report(const Report&) = delete;
  Report& operator=(Report&&) = delete;
  Report& operator=(const Report&) = delete;

  virtual ~Report() noexcept { }

  void handle_no_conn() override;

  protected:

  void clear_endpoints();

  SWC::client::Clients::Ptr clients;
  EndPoints                 endpoints;
};




class ClusterStatus: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&)> Cb_t;
  typedef std::shared_ptr<ClusterStatus> Ptr;

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints,
                      Cb_t&& cb, const uint32_t timeout = 10000) {
    make(clients, endpoints, std::move(cb), timeout)->run();
  }

  SWC_CAN_INLINE
  static Ptr make(const SWC::client::Clients::Ptr& clients,
                  const EndPoints& endpoints,
                  Cb_t&& cb, const uint32_t timeout = 10000) {
    return Ptr(new ClusterStatus(clients, endpoints, std::move(cb), timeout));
  }

  ClusterStatus(const SWC::client::Clients::Ptr& clients,
                const EndPoints& endpoints,
                Cb_t&& cb, const uint32_t timeout);

  virtual ~ClusterStatus() noexcept { }

  bool run() override;

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};




class ColumnStatus: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspColumnStatus&)> Cb_t;
  typedef std::shared_ptr<ColumnStatus> Ptr;

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    request(
      clients, Params::Report::ReqColumnStatus(cid), std::move(cb), timeout);
  }

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      const Params::Report::ReqColumnStatus& params,
                      Cb_t&& cb, const uint32_t timeout = 10000) {
    make(clients, params, std::move(cb), timeout)->run();
  }

  SWC_CAN_INLINE
  static Ptr make(const SWC::client::Clients::Ptr& clients,
                  const Params::Report::ReqColumnStatus& params,
                  Cb_t&& cb, const uint32_t timeout = 10000) {
    return Ptr(new ColumnStatus(clients, params, std::move(cb), timeout));
  }

  ColumnStatus(const SWC::client::Clients::Ptr& clients,
               const Params::Report::ReqColumnStatus& params,
               Cb_t&& cb, const uint32_t timeout);

  virtual ~ColumnStatus() noexcept { }

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;
  cid_t       cid;

};




class RangersStatus: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspRangersStatus&)> Cb_t;
  typedef std::shared_ptr<RangersStatus> Ptr;

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      cid_t cid, Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    make(clients, cid, std::move(cb), timeout)->run();
  }

  SWC_CAN_INLINE
  static Ptr make(const SWC::client::Clients::Ptr& clients,
                  cid_t cid, Cb_t&& cb,
                  const uint32_t timeout = 10000) {
    return Ptr(new RangersStatus(clients, cid, std::move(cb), timeout));
  }

  RangersStatus(const SWC::client::Clients::Ptr& clients,
                cid_t cid, Cb_t&& cb,
                const uint32_t timeout);

  virtual ~RangersStatus() noexcept { }

  bool run() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;
  cid_t       cid;

};




class ManagersStatus: public Report {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             const int&,
                             const Params::Report::RspManagersStatus&)> Cb_t;
  typedef std::shared_ptr<ManagersStatus> Ptr;

  SWC_CAN_INLINE
  static void request(const SWC::client::Clients::Ptr& clients,
                      const EndPoints& endpoints, Cb_t&& cb,
                      const uint32_t timeout = 10000) {
    make(clients, endpoints, std::move(cb), timeout)->run();
  }

  SWC_CAN_INLINE
  static Ptr make(const SWC::client::Clients::Ptr& clients,
                  const EndPoints& endpoints, Cb_t&& cb,
                  const uint32_t timeout = 10000) {
    return Ptr(new ManagersStatus(
      clients, endpoints, std::move(cb), timeout));
  }

  ManagersStatus(const SWC::client::Clients::Ptr& clients,
                 const EndPoints& endpoints, Cb_t&& cb,
                 const uint32_t timeout);

  virtual ~ManagersStatus() noexcept { }

  bool run() override;

  void handle_no_conn() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};




}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/Report.cc"
#endif

#endif // swcdb_db_protocol_req_Report_h
