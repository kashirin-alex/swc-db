/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_SeekSync_h
#define swcdb_fs_Broker_Protocol_req_SeekSync_h

#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/Protocol/params/Seek.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class SeekSync final : public BaseSync, public Base {
  public:

  SeekSync(FS::Statistics& stats,
           uint32_t timeout, FS::SmartFd::Ptr& smartfd, size_t offset)
          : Base(
              stats, FS::Statistics::SEEK_SYNC,
              Buffers::make(
                Params::SeekReq(smartfd->fd(), offset),
                0,
                FUNCTION_SEEK, timeout
              )
            ),
            smartfd(smartfd) {
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    Base::handle_seek(ev, smartfd);
    BaseSync::acknowledge();
  }

  private:
  FS::SmartFd::Ptr&   smartfd;

};


}}}}}


#endif // swcdb_fs_Broker_Protocol_req_SeekSync_h
