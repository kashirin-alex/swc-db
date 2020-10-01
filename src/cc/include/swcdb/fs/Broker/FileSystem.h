/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_FileSystem_h
#define swcdb_fs_Broker_FileSystem_h

#include "swcdb/fs/FileSystem.h"

#include "swcdb/core/comm/SerializedClient.h"


namespace SWC {


namespace FsBroker { namespace Protocol { namespace Req {

class Base;
typedef std::shared_ptr<Base> BasePtr;

}}}



namespace FS {

Configurables apply_broker();


class FileSystemBroker final : public FileSystem {
  public:

  static Comm::EndPoints get_endpoints();

  FileSystemBroker();

  virtual ~FileSystemBroker();

  void stop() override;

  Type get_type() override;

  std::string to_string() override;

  bool send_request(FsBroker::Protocol::Req::BasePtr hdlr);

  void send_request_sync(FsBroker::Protocol::Req::BasePtr hdlr, 
                         std::promise<void> res);

  /// File/Dir name actions

  bool exists(int& err, const std::string& name) override;

  void exists(const Callback::ExistsCb_t& cb, 
              const std::string& name) override;

  void remove(int& err, const std::string& name) override;

  void remove(const Callback::RemoveCb_t& cb,
              const std::string& name) override;
  
  size_t length(int& err, const std::string& name) override;

  void length(const Callback::LengthCb_t& cb, 
              const std::string& name) override;

  void mkdirs(int& err, const std::string& name) override;

  void mkdirs(const Callback::MkdirsCb_t& cb, 
              const std::string& name) override;

  void readdir(int& err, const std::string& name, 
               DirentList& results) override;

  void readdir(const Callback::ReaddirCb_t& cb, 
               const std::string& name) override;

  void rmdir(int& err, const std::string& name) override;

  void rmdir(const Callback::RmdirCb_t& cb, 
             const std::string& name) override;

  void rename(int& err, 
              const std::string& from, const std::string& to) override;

  void rename(const Callback::RenameCb_t& cb, 
              const std::string& from, const std::string& to)  override;

  /// SmartFd actions

  void write(int& err, SmartFd::Ptr& smartfd,
             uint8_t replication, int64_t blksz, 
             StaticBuffer& buffer) override;

  void write(const Callback::WriteCb_t& cb, SmartFd::Ptr& smartfd,
             uint8_t replication, int64_t blksz, 
             StaticBuffer& buffer) override;

  void read(int& err, const std::string& name, StaticBuffer* dst) override;

  void read(const Callback::ReadAllCb_t& cb, 
            const std::string& name) override;

  void create(int& err, SmartFd::Ptr& smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz) override;

  void create(const Callback::CreateCb_t& cb, SmartFd::Ptr& smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz) override;
  
  size_t append(int& err, SmartFd::Ptr& smartfd, 
                StaticBuffer& buffer, Flags flags) override;
   
  void append(const Callback::AppendCb_t& cb, SmartFd::Ptr& smartfd, 
              StaticBuffer& buffer, Flags flags) override;

  void open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz) override;

  void open(const Callback::OpenCb_t& cb, SmartFd::Ptr& smartfd, 
            int32_t bufsz) override;
  
  size_t read(int& err, SmartFd::Ptr& smartfd, 
              void *dst, size_t amount) override;
   
  size_t read(int& err, SmartFd::Ptr& smartfd, 
              StaticBuffer* dst, size_t amount) override;
   
  void read(const Callback::ReadCb_t& cb, SmartFd::Ptr& smartfd, 
            size_t amount) override;
  
  size_t pread(int& err, SmartFd::Ptr& smartfd, 
              uint64_t offset, void *dst, size_t amount) override;

  size_t pread(int& err, SmartFd::Ptr& smartfd, 
              uint64_t offset, StaticBuffer* dst, size_t amount) override;
   
  void pread(const Callback::ReadCb_t& cb, SmartFd::Ptr& smartfd, 
            uint64_t offset, size_t amount) override;

  void seek(int& err, SmartFd::Ptr& smartfd, size_t offset) override;
   
  void seek(const Callback::SeekCb_t& cb, SmartFd::Ptr& smartfd, 
            size_t offset) override;

  void flush(int& err, SmartFd::Ptr& smartfd) override;
  
  void flush(const Callback::FlushCb_t& cb, SmartFd::Ptr& smartfd) override;

  void sync(int& err, SmartFd::Ptr& smartfd) override;
  
  void sync(const Callback::SyncCb_t& cb, SmartFd::Ptr& smartfd) override;

  void close(int& err, SmartFd::Ptr& smartfd) override;

  void close(const Callback::CreateCb_t& cb, SmartFd::Ptr& smartfd) override;

  private:

  Comm::IoContext::Ptr          m_io;
  Comm::client::Serialized::Ptr m_service = nullptr;
  Type                          m_type_underlying;
  const Comm::EndPoints         m_endpoints;
  std::atomic<bool>             m_run;

  const Config::Property::V_GINT32::Ptr cfg_timeout;
  const Config::Property::V_GINT32::Ptr cfg_timeout_ratio;
};


}}


extern "C" {
SWC::FS::FileSystem* fs_make_new_broker();
void fs_apply_cfg_broker(SWC::Env::Config::Ptr env);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/FileSystem.cc"
#endif 


#endif // swcdb_fs_Broker_FileSystem_h