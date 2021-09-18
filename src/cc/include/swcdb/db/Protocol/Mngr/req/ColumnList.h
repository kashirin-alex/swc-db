/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_ColumnList_h
#define swcdb_db_protocol_mngr_req_ColumnList_h


#include "swcdb/db/Protocol/Mngr/req/ColumnList_Base.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


template<typename DataT>
class ColumnList final : public ColumnList_Base {
  public:

  typedef std::shared_ptr<ColumnList> Ptr;
  DataT                               data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::ColumnListReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new ColumnList(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::ColumnListReq& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  virtual ~ColumnList() noexcept { }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  ColumnList(
        const Params::ColumnListReq& params,
        const uint32_t timeout,
        DataArgsT&&... args)
      : ColumnList_Base(params, timeout),
        data(args...) {
  }

  SWC::client::Clients::Ptr& get_clients() noexcept override {
    return data.get_clients();
  }

  bool valid() override {
    return data.valid();
  }

  void callback(int err, const Params::ColumnListRsp& rsp) override {
    data.callback(req(), err, rsp);
  }

};


/** Functional_ColumnList - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Mngr::Req::Functional_ColumnList;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               int err,
               const Comm::Protocol::Mngr::Params::ColumnListRsp&) noexcept {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Mngr::Req::ColumnList<data_t>::request(
    params, timeout, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    int err,
    const Params::ColumnListRsp&
  )>
> Functional_ColumnList;



}}}}}



#endif // swcdb_db_protocol_mngr_req_ColumnList_h
