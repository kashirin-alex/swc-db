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

  FS::Statistics::Metric::Tracker tracker;
  int                             error;
  Buffers::Ptr                    cbp;

  virtual ~Base() { }

  bool is_rsp(const Event::Ptr& ev, int cmd,
              const uint8_t **ptr, size_t *remain);

  void handle_no_conn() override {
    handle(nullptr, nullptr);
  }

  protected:

  Base(FS::Statistics& stats, FS::Statistics::Command cmd, Buffers::Ptr&& cbp)
      : tracker(stats.tracker(cmd)), error(Error::OK), cbp(std::move(cbp)) {
  }

  /* common (Sync & Async) handlers */

  void handle_write(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd);

  void handle_sync(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd);

  void handle_seek(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd);

  void handle_rmdir(const Event::Ptr& ev, const std::string& name);

  void handle_rename(const Event::Ptr& ev,
                     const std::string& from, const std::string& to);

  void handle_readdir(const Event::Ptr& ev, const std::string& name,
                      FS::DirentList& listing);

  void handle_remove(const Event::Ptr& ev, const std::string& name);

  void handle_read(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd,
                   size_t& amount);

  void handle_read_all(const Event::Ptr& ev, const std::string& name);

  void handle_combi_pread(const Event::Ptr& ev,
                          const FS::SmartFd::Ptr& smartfd);

  void handle_pread(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd,
                    size_t& amount);

  void handle_open(const FS::FileSystem::Ptr& fs, const Event::Ptr& ev,
                   FS::SmartFd::Ptr& smartfd);

  void handle_mkdirs(const Event::Ptr& ev, const std::string& name);

  void handle_length(const Event::Ptr& ev, const std::string& name,
                     size_t& length);

  void handle_flush(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd);

  void handle_exists(const Event::Ptr& ev, const std::string& name,
                     bool& state);

  void handle_create(const FS::FileSystem::Ptr& fs, const Event::Ptr& ev,
                     FS::SmartFd::Ptr& smartfd);

  void handle_close(const FS::FileSystem::Ptr& fs, const Event::Ptr& ev,
                    FS::SmartFd::Ptr& smartfd);

  void handle_append(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd,
                     size_t& amount);
};



}}}}}


#include "swcdb/fs/Broker/Protocol/req/BaseSync.h"


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/req/Base.cc"
#endif

#endif // swcdb_fs_Broker_Protocol_req_Base_h
