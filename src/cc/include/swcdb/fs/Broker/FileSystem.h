/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_FileSystem_h
#define swc_lib_fs_Broker_FileSystem_h

#include "swcdb/fs/FileSystem.h"

#include "swcdb/core/comm/SerializedClient.h"

namespace SWC{ namespace FS {

namespace Protocol { namespace Req {
class Base;
typedef std::shared_ptr<Base> BasePtr;
}}

bool apply_broker();


class FileSystemBroker: public FileSystem {
  public:

  static const EndPoints get_endpoints();

  FileSystemBroker();

  virtual ~FileSystemBroker();

  void stop() override;

  Types::Fs get_type() override;

  const std::string to_string() override;

  bool send_request(Protocol::Req::BasePtr hdlr);

  void send_request_sync(Protocol::Req::BasePtr hdlr, 
                         std::promise<void> res);

  /// File/Dir name actions

  bool exists(int &err, const std::string &name) override;

  void exists(Callback::ExistsCb_t cb, const std::string &name) override;

  void remove(int &err, const std::string &name) override;

  void remove(Callback::RemoveCb_t cb, const std::string &name) override;
  
  size_t length(int &err, const std::string &name) override;

  void length(Callback::LengthCb_t cb, const std::string &name) override;

  void mkdirs(int &err, const std::string &name) override;

  void mkdirs(Callback::MkdirsCb_t cb, const std::string &name) override;

  void readdir(int &err, const std::string &name, 
               DirentList &results) override;

  void readdir(Callback::ReaddirCb_t cb, const std::string &name) override;

  void rmdir(int &err, const std::string &name) override;

  void rmdir(Callback::RmdirCb_t cb, const std::string &name) override;

  void rename(int &err, 
              const std::string &from, const std::string &to) override;

  void rename(Callback::RenameCb_t cb, 
              const std::string &from, const std::string &to)  override;

  /// SmartFd actions

  void write(int &err, SmartFd::Ptr &smartfd,
             uint8_t replication, int64_t blksz, 
             StaticBuffer &buffer) override;

  void write(Callback::WriteCb_t cb, SmartFd::Ptr &smartfd,
             uint8_t replication, int64_t blksz, 
             StaticBuffer &buffer) override;

  void create(int &err, SmartFd::Ptr &smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz) override;

  void create(Callback::CreateCb_t cb, SmartFd::Ptr &smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz) override;
  
  size_t append(int &err, SmartFd::Ptr &smartfd, 
                StaticBuffer &buffer, Flags flags) override;
   
  void append(Callback::AppendCb_t cb, SmartFd::Ptr &smartfd, 
              StaticBuffer &buffer, Flags flags) override;

  void open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz) override;

  void open(Callback::OpenCb_t cb, SmartFd::Ptr &smartfd, 
            int32_t bufsz) override;
  
  size_t read(int &err, SmartFd::Ptr &smartfd, 
              void *dst, size_t amount) override;
   
  size_t read(int &err, SmartFd::Ptr &smartfd, 
              StaticBuffer* dst, size_t amount) override;
   
  void read(Callback::ReadCb_t cb, SmartFd::Ptr &smartfd, 
            size_t amount) override;
  
  size_t pread(int &err, SmartFd::Ptr &smartfd, 
              uint64_t offset, void *dst, size_t amount) override;

  size_t pread(int &err, SmartFd::Ptr &smartfd, 
              uint64_t offset, StaticBuffer* dst, size_t amount) override;
   
  void pread(Callback::ReadCb_t cb, SmartFd::Ptr &smartfd, 
            uint64_t offset, size_t amount) override;

  void seek(int &err, SmartFd::Ptr &smartfd, size_t offset) override;
   
  void seek(Callback::SeekCb_t cb, SmartFd::Ptr &smartfd, 
            size_t offset) override;

  void flush(int &err, SmartFd::Ptr &smartfd) override;
  
  void flush(Callback::FlushCb_t cb, SmartFd::Ptr &smartfd) override;

  void sync(int &err, SmartFd::Ptr &smartfd) override;
  
  void sync(Callback::SyncCb_t cb, SmartFd::Ptr &smartfd) override;

  void close(int &err, SmartFd::Ptr &smartfd) override;

  void close(Callback::CreateCb_t cb, SmartFd::Ptr &smartfd) override;

  private:

  IoContext::Ptr          m_io;
  client::Serialized::Ptr m_service = nullptr;
  Types::Fs               m_type_underlying;
  const EndPoints         m_endpoints;
  std::atomic<bool>       m_run;

  const gInt32tPtr cfg_timeout;
  const gInt32tPtr cfg_timeout_ratio;
};


}}


extern "C" {
SWC::FS::FileSystem* fs_make_new_broker();
void fs_apply_cfg_broker(SWC::Env::Config::Ptr env);
}

#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/fs/Broker/FileSystem.cc"
#endif 


#endif  // swc_lib_fs_Broker_FileSystem_h