/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Base_h
#define swcdb_fs_Broker_Protocol_req_Base_h

#include "swcdb/fs/Broker/FileSystem.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class Base : public DispatchHandler {
  public:

  using Ptr = BasePtr;

  Buffers::Ptr  cbp;
  int           error;

  Base(const Buffers::Ptr& cbp = nullptr)
      : cbp(cbp), error(Error::OK) {
  }

  virtual ~Base() { }

  bool is_rsp(const Event::Ptr& ev, int cmd, 
              const uint8_t **ptr, size_t *remain);

};



}}}}}


#include "swcdb/fs/Broker/Protocol/req/BaseSync.h"


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Base.cc"
#endif

#endif // swcdb_fs_Broker_Protocol_req_Base_h