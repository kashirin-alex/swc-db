/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_Base_h
#define swcdb_fs_Broker_Protocol_req_Base_h

#include "swcdb/fs/Broker/FileSystem.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include <future>

namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {

class Base : public Comm::DispatchHandler {

  public:

  using Ptr = BasePtr;

  Comm::Buffers::Ptr  cbp;
  int                 error;

  Base();

  virtual ~Base();

  bool is_rsp(const Comm::Event::Ptr& ev, int cmd, 
              const uint8_t **ptr, size_t *remain);

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Base.cc"
#endif

#endif // swcdb_fs_Broker_Protocol_req_Base_h