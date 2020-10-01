
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_Report_h
#define swcdb_db_protocol_req_Report_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Mngr/params/Report.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Req {

  
class Report: public Comm::client::ConnQueue::ReqBase {
  public:
  
  Report(Params::Report::Function func, const uint32_t timeout);

  Report(const Comm::EndPoints& endpoints, Params::Report::Function func, 
         const uint32_t timeout);

  Report(const Comm::Serializable& params, Params::Report::Function func, 
         const uint32_t timeout);

  virtual ~Report();

  void handle_no_conn() override;

  protected:
  
  void clear_endpoints();

  Comm::EndPoints   endpoints;
};




class ClusterStatus: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&)> Cb_t;
 
  static void request(const Comm::EndPoints& endpoints, 
                      const Cb_t& cb, const uint32_t timeout = 10000);

  static Ptr make(const Comm::EndPoints& endpoints, 
                  const Cb_t& cb, const uint32_t timeout = 10000);

  ClusterStatus(const Comm::EndPoints& endpoints, 
                const Cb_t& cb, const uint32_t timeout);

  virtual ~ClusterStatus();

  bool run() override;

  void handle_no_conn() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};



  
class ColumnStatus: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
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

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;
  cid_t       cid;

};




class RangersStatus: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&, 
                             const Params::Report::RspRangersStatus&)> Cb_t;
 
  static void request(cid_t cid, const Cb_t& cb, 
                      const uint32_t timeout = 10000);

  static Ptr make(cid_t cid, const Cb_t& cb, 
                  const uint32_t timeout = 10000);

  RangersStatus(cid_t cid, const Cb_t& cb, 
                const uint32_t timeout);

  virtual ~RangersStatus();

  bool run() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;
  cid_t       cid;

};




class ManagersStatus: public Report {
  public:
  
  typedef std::function<void(const Comm::client::ConnQueue::ReqBase::Ptr&, 
                             const int&, 
                             const Params::Report::RspManagersStatus&)> Cb_t;
 
  static void request(const Comm::EndPoints& endpoints, const Cb_t& cb, 
                      const uint32_t timeout = 10000);

  static Ptr make(const Comm::EndPoints& endpoints, const Cb_t& cb, 
                  const uint32_t timeout = 10000);

  ManagersStatus(const Comm::EndPoints& endpoints, const Cb_t& cb, 
                 const uint32_t timeout);

  virtual ~ManagersStatus();

  bool run() override;

  void handle_no_conn() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  private:

  const Cb_t  cb;

};




}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/req/Report.cc"
#endif 

#endif // swcdb_db_protocol_req_Report_h
