/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_FileSystem_h
#define swcdb_fs_Broker_FileSystem_h

#include "swcdb/fs/FileSystem.h"

#include "swcdb/core/comm/SerializedClient.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker { namespace Req {

class Base;
typedef std::shared_ptr<Base> BasePtr;

}}}} // namespace Comm::Protocol::FsBroker::Req



namespace FS {


Configurables* apply_broker(Configurables* config);


class FileSystemBroker final : public FileSystem {
  public:

  static Comm::EndPoints get_endpoints(const Config::Settings::Ptr& settings);

  FileSystemBroker(Configurables* config);

  virtual ~FileSystemBroker() noexcept;

  void stop() override;

  Type SWC_CONST_FUNC get_type() const noexcept override;

  Type SWC_PURE_FUNC get_type_underlying() const noexcept override;

  std::string to_string() const override;

  bool send_request(Comm::Protocol::FsBroker::Req::BasePtr hdlr);

  /// File/Dir name actions

  bool exists(int& err, const std::string& name) override;

  void exists(Callback::ExistsCb_t&& cb,
              const std::string& name) override;

  void remove(int& err, const std::string& name) override;

  void remove(Callback::RemoveCb_t&& cb,
              const std::string& name) override;

  size_t length(int& err, const std::string& name) override;

  void length(Callback::LengthCb_t&& cb,
              const std::string& name) override;

  void mkdirs(int& err, const std::string& name) override;

  void mkdirs(Callback::MkdirsCb_t&& cb,
              const std::string& name) override;

  void readdir(int& err, const std::string& name,
               DirentList& results) override;

  void readdir(Callback::ReaddirCb_t&& cb,
               const std::string& name) override;

  void rmdir(int& err, const std::string& name) override;

  void rmdir(Callback::RmdirCb_t&& cb,
             const std::string& name) override;

  void rename(int& err,
              const std::string& from, const std::string& to) override;

  void rename(Callback::RenameCb_t&& cb,
              const std::string& from, const std::string& to)  override;

  /// SmartFd actions

  void write(int& err, SmartFd::Ptr& smartfd,
             uint8_t replication, StaticBuffer& buffer) override;

  void write(Callback::WriteCb_t&& cb, SmartFd::Ptr& smartfd,
             uint8_t replication, StaticBuffer& buffer) override;

  void read(int& err, const std::string& name, StaticBuffer* dst) override;

  void read(Callback::ReadAllCb_t&& cb,
            const std::string& name) override;

  void combi_pread(int& err, SmartFd::Ptr& smartfd,
                   uint64_t offset, uint32_t amount,
                   StaticBuffer* dst) override;

  void combi_pread(Callback::CombiPreadCb_t&& cb,
                   SmartFd::Ptr& smartfd,
                   uint64_t offset, uint32_t amount) override;

  void create(int& err, SmartFd::Ptr& smartfd, uint8_t replication) override;

  void create(Callback::CreateCb_t&& cb, SmartFd::Ptr& smartfd,
              uint8_t replication) override;

  size_t append(int& err, SmartFd::Ptr& smartfd,
                StaticBuffer& buffer, Flags flags) override;

  void append(Callback::AppendCb_t&& cb, SmartFd::Ptr& smartfd,
              StaticBuffer& buffer, Flags flags) override;

  void open(int& err, SmartFd::Ptr& smartfd) override;

  void open(Callback::OpenCb_t&& cb, SmartFd::Ptr& smartfd) override;

  size_t read(int& err, SmartFd::Ptr& smartfd,
              void *dst, size_t amount) override;

  size_t read(int& err, SmartFd::Ptr& smartfd,
              StaticBuffer* dst, size_t amount) override;

  void read(Callback::ReadCb_t&& cb, SmartFd::Ptr& smartfd,
            size_t amount) override;

  size_t pread(int& err, SmartFd::Ptr& smartfd,
              uint64_t offset, void *dst, size_t amount) override;

  size_t pread(int& err, SmartFd::Ptr& smartfd,
              uint64_t offset, StaticBuffer* dst, size_t amount) override;

  void pread(Callback::ReadCb_t&& cb, SmartFd::Ptr& smartfd,
            uint64_t offset, size_t amount) override;

  void seek(int& err, SmartFd::Ptr& smartfd, size_t offset) override;

  void seek(Callback::SeekCb_t&& cb, SmartFd::Ptr& smartfd,
            size_t offset) override;

  void flush(int& err, SmartFd::Ptr& smartfd) override;

  void flush(Callback::FlushCb_t&& cb, SmartFd::Ptr& smartfd) override;

  void sync(int& err, SmartFd::Ptr& smartfd) override;

  void sync(Callback::SyncCb_t&& cb, SmartFd::Ptr& smartfd) override;

  void close(int& err, SmartFd::Ptr& smartfd) override;

  void close(Callback::CloseCb_t&& cb, SmartFd::Ptr& smartfd) override;

  private:

  Comm::IoContextPtr                    m_io;
  Comm::client::Serialized::Ptr         m_service = nullptr;
  Type                                  m_type_underlying;
  const Comm::EndPoints                 m_endpoints;
  Core::QueueSafe<Comm::ConnHandlerPtr> m_connections;

  const Config::Property::Value_int32_g::Ptr cfg_timeout;
  const Config::Property::Value_int32_g::Ptr cfg_timeout_ratio;
};


}}


extern "C" {
SWC::FS::FileSystem* fs_make_new_broker(SWC::FS::Configurables* config);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/FileSystem.cc"
#endif


#endif // swcdb_fs_Broker_FileSystem_h
