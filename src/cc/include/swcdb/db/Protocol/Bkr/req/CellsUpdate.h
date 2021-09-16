/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h
#define swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h


#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"
#include "swcdb/db/Protocol/Commands.h"
#include "swcdb/db/Protocol/Common/req/handler_data.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


template<typename DataT>
class CellsUpdate final : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<CellsUpdate> Ptr;
  DataT                                data;

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::CellsUpdateReq& params,
        StaticBuffer& snd_buf,
        const uint32_t timeout,
        DataArgsT&&... args) {
    return Ptr(new CellsUpdate(
      Buffers::make(params, snd_buf, 0, CELLS_UPDATE, timeout),
      args...
    ));
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static Ptr make(
        const Params::CellsUpdateReq& params,
        const DynamicBuffer& buffer,
        const uint32_t timeout,
        DataArgsT&&... args) {
    StaticBuffer snd_buf(buffer.base, buffer.fill(), false);
    return make(params, snd_buf, timeout, args...);
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::CellsUpdateReq& params,
        StaticBuffer& snd_buf,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, snd_buf, timeout, args...)->run();
  }

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  static void request(
        const Params::CellsUpdateReq& params,
        const DynamicBuffer& buffer,
        const uint32_t timeout,
        DataArgsT&&... args) {
    make(params, buffer, timeout, args...)->run();
  }

  virtual ~CellsUpdate() { }

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
      data.callback(req(), Params::CellsUpdateRsp(Error::COMM_NOT_CONNECTED));
    }
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    data.callback(
      req(),
      Params::CellsUpdateRsp(ev->error, ev->data.base, ev->data.size)
    );
  }

  protected:

  template<typename... DataArgsT>
  SWC_CAN_INLINE
  CellsUpdate(Buffers::Ptr&& a_cbp, DataArgsT&&... args)
              : client::ConnQueue::ReqBase(std::move(a_cbp)),
                data(args...) {
  }

  private:
  SWC::client::Brokers::BrokerIdx _bkr_idx;

};



/** Functional_CellsUpdate - a default CbT DataT STL
  ```
  using data_t = Comm::Protocol::Bkr::Req::Functional_CellsUpdate;
  auto cb = [](void* datap,
               const Comm::client::ConnQueue::ReqBase::Ptr&,
               const Comm::Protocol::Bkr::Params::CellsUpdateRsp&) {
    data_t::Ptr datap = data_t::cast(_datap);
    (void)(datap->clients);
    ...;
  };
  Comm::Protocol::Bkr::Req::CellsUpdate<data_t>::request(
    params, buffer, timeout, clients, std::move(cb));
  ```
*/

typedef Common::Req::function<
  std::function<void(
    void*,
    const client::ConnQueue::ReqBase::Ptr&,
    const Params::CellsUpdateRsp&
  )>
> Functional_CellsUpdate;



}}}}}



#endif // swcdb_db_protocol_bkr_req_Committer_CellsUpdate_h
