/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_CellsSelect_h
#define swcdb_db_protocol_bkr_req_CellsSelect_h


#include "swcdb/db/Protocol/Bkr/params/CellsSelect.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


template<typename DataT>
class CellsSelect final : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<CellsSelect> Ptr;
  DataT                                data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::CellsSelectReqRef& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new CellsSelect(params, timeout, args...));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::CellsSelectReqRef& params,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, timeout, args...)->run();
  }

  virtual ~CellsSelect() noexcept { }

  bool run() override {
    return data.get_clients()->brokers.put(req(), _bkr_idx);
  }

  bool valid() override {
    return data.valid() && !data.get_clients()->stopping();
  }

  void handle_no_conn() override {
    if(data.valid() && !_bkr_idx.turn_around(data.get_clients()->brokers)) {
      run();
    } else {
      Params::CellsSelectRsp rsp(Error::COMM_NOT_CONNECTED);
      data.callback(req(), rsp);
    }
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Params::CellsSelectRsp rsp(
      ev->error, ev->data.base, ev->data.size, ev->data_ext);
    data.callback(req(), rsp);
  }


  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  CellsSelect(const Params::CellsSelectReqRef& params,
              const uint32_t timeout,
              DataArgsT&&... args)
              : client::ConnQueue::ReqBase(
                  Buffers::make(params, 0, CELLS_SELECT, timeout)
                ),
                data(args...),
                _bkr_idx() {
  }

  private:
  SWC::client::Brokers::BrokerIdx _bkr_idx;

};






/** Functional_CellsSelect - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Bkr::Req::Functional_CellsSelect;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               Comm::Protocol::Bkr::Params::CellsSelectRsp&) {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Bkr::Req::CellsSelect<data_t>::request(
    params, timeout, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    Params::CellsSelectRsp&
  )>
> Functional_CellsSelect;



}}}}}



#endif // swcdb_db_protocol_bkr_req_CellsSelect_h
