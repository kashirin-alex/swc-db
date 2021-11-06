/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_handlers_Create_h
#define swcdb_fsbroker_handlers_Create_h

#include "swcdb/fs/Broker/Protocol/params/Create.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Handler {


void create(const ConnHandlerPtr& conn, const Event::Ptr& ev) {

  int err = Error::OK;
  int32_t fd = -1;
  try {

    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::CreateReq params;
    params.decode(&ptr, &remain);

    auto smartfd = FS::SmartFd::make_ptr(
      std::move(params.fname), params.flags);

    Env::FsInterface::fs()->create(
      err, smartfd, params.replication, params.blksz
    );

    if(smartfd->valid())
      fd = Env::FsBroker::fds().add(smartfd);

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  auto cbp = Buffers::make(ev, Params::OpenRsp(fd), 4);
  cbp->append_i32(err);
  conn->send_response(cbp);

}


}}}}}

#endif // swcdb_fsbroker_handlers_Create_h
