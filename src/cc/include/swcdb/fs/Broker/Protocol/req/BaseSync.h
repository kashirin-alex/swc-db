/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_BaseSync_h
#define swcdb_fs_Broker_Protocol_req_BaseSync_h


#include "swcdb/core/StateSynchronization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class BaseSync : private Core::StateSynchronization {
  public:
  using Core::StateSynchronization::wait;

  protected:
  using Core::StateSynchronization::acknowledge;

  ~BaseSync() noexcept { }
};



}}}}}


#endif // swcdb_fs_Broker_Protocol_req_BaseSync_h
