/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_req_Base_h
#define swc_fs_Broker_Protocol_req_Base_h

#include "swcdb/fs/Broker/FileSystem.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include <future>

namespace SWC { namespace FS { namespace Protocol { namespace Req {

class Base : public DispatchHandler {

  public:

  using Ptr = BasePtr;

  CommBuf::Ptr  cbp;
  int           error;

  Base();

  virtual ~Base();

  bool is_rsp(const Event::Ptr& ev, int cmd, 
              const uint8_t **ptr, size_t *remain);

};



}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Base.cc"
#endif

#endif  // swc_fs_Broker_Protocol_req_Base_h