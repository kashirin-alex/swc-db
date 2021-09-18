/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_protocol_common_req_handler_data_h
#define swcdb_db_protocol_common_req_handler_data_h


#include "swcdb/db/client/Clients.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Common { namespace Req {


template<typename CbT>
struct function {
  typedef function* Ptr;

  static Ptr cast(void* _datap) noexcept {
    return reinterpret_cast<Ptr>(_datap);
  }

  SWC::client::Clients::Ptr clients;
  CbT                       cb;

  SWC_CAN_INLINE
  function(SWC::client::Clients::Ptr& a_clients, const CbT& a_cb)
          : clients(a_clients), cb(a_cb) {
  }

  SWC_CAN_INLINE
  function(SWC::client::Clients::Ptr& a_clients, CbT&& a_cb) noexcept
          : clients(a_clients), cb(std::move(a_cb)) {
  }

  ~function() noexcept { }

  SWC_CAN_INLINE
  SWC::client::Clients::Ptr& get_clients() noexcept {
    return clients;
  }

  SWC_CAN_INLINE
  bool valid() noexcept {
    return true;
  }

  template<typename... ArgsT>
  SWC_CAN_INLINE
  void callback(ArgsT&&... args) {
    cb(this, args...);
  }

};


}}}}}


#endif // swcdb_db_protocol_common_req_handler_data_h
