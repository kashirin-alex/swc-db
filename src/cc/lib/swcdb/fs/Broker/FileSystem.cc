/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/core/comm/ConnHandler.h"

#include "swcdb/fs/Broker/FileSystem.h"
#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/AppContext.h"
 
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include "swcdb/fs/Broker/Protocol/req/Exists.h"
#include "swcdb/fs/Broker/Protocol/req/Remove.h"
#include "swcdb/fs/Broker/Protocol/req/Length.h"
#include "swcdb/fs/Broker/Protocol/req/Mkdirs.h"
#include "swcdb/fs/Broker/Protocol/req/Readdir.h"
#include "swcdb/fs/Broker/Protocol/req/Rmdir.h"
#include "swcdb/fs/Broker/Protocol/req/Rename.h"
#include "swcdb/fs/Broker/Protocol/req/Write.h"
#include "swcdb/fs/Broker/Protocol/req/Create.h"
#include "swcdb/fs/Broker/Protocol/req/Append.h"
#include "swcdb/fs/Broker/Protocol/req/Open.h"
#include "swcdb/fs/Broker/Protocol/req/Read.h"
#include "swcdb/fs/Broker/Protocol/req/Pread.h"
#include "swcdb/fs/Broker/Protocol/req/Seek.h"
#include "swcdb/fs/Broker/Protocol/req/Sync.h"
#include "swcdb/fs/Broker/Protocol/req/Flush.h"
#include "swcdb/fs/Broker/Protocol/req/Close.h"


namespace SWC{ namespace FS {


bool apply_broker() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.broker.OnFileChange.cfg", str(), "Dyn-config file")
    ("swc.fs.broker.host", str(), "FsBroker host (default by hostname)") 
    ("swc.fs.broker.port", i32(17000), "FsBroker port")
    ("swc.fs.broker.handlers", i32(48), "Handlers for broker tasks")
    ("swc.fs.broker.timeout", g_i32(30000), "Default request timeout in ms")
    ("swc.fs.broker.timeout.bytes.ratio", g_i32(1000), 
     "Timeout ratio to bytes, bytes/ratio=ms added to default timeout")
  ;
  Env::Config::settings()->parse_file(
    Env::Config::settings()->get<std::string>("swc.fs.broker.cfg", ""),
    Env::Config::settings()->get<std::string>("swc.fs.broker.OnFileChange.cfg", "")
  );
  return true;
}



const EndPoints FileSystemBroker::get_endpoints() {
  std::string host = Env::Config::settings()->get<std::string>(
    "swc.fs.broker.host", "");
  if(host.empty()) {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }
  Strings addr;
  return Resolver::get_endpoints(
    Env::Config::settings()->get<int32_t>("swc.fs.broker.port"),
    addr, host, true
  );
}

FileSystemBroker::FileSystemBroker()
  : FileSystem(apply_broker()),
    m_io(std::make_shared<IoContext>("FsBroker",
      Env::Config::settings()->get<int32_t>("swc.fs.broker.handlers"))),
    m_service(std::make_shared<client::Serialized>(
      "FS-BROKER", m_io->shared(), std::make_shared<FsClientAppCtx>())),
    m_type_underlying(fs_type(
      Env::Config::settings()->get<std::string>("swc.fs.broker.underlying"))),
    cfg_timeout(Env::Config::settings()->get_ptr<gInt32t>(
      "swc.fs.broker.timeout")),
    cfg_timeout_ratio(Env::Config::settings()->get_ptr<gInt32t>(
      "swc.fs.broker.timeout.bytes.ratio")),
    m_endpoints(get_endpoints()),
    m_run(true) {
  m_io->run(m_io);
}

FileSystemBroker::~FileSystemBroker() { }

void FileSystemBroker::stop() {
  m_run = false;
  m_service->stop();
  m_io->stop();
}

Types::Fs FileSystemBroker::get_type() {
  return m_type_underlying;
}

const std::string FileSystemBroker::to_string() {
  return format(
    "(type=BROKER underlying-type=%s)",
    type_to_string(m_type_underlying).c_str()
  );
}


bool FileSystemBroker::send_request(Protocol::Req::Base::Ptr hdlr) {
  ConnHandlerPtr conn = nullptr;
  do {
    if(!m_run) {
      auto ev = Event::make(Event::Type::ERROR, Error::SERVER_SHUTTING_DOWN);
      hdlr->handle(conn, ev);
      return true;
    }
    conn = m_service->get_connection(
      m_endpoints, std::chrono::milliseconds(20000), 3);
  } while(conn == nullptr);
  m_service->preserve(conn);

  if(conn->send_request(hdlr->cbp, hdlr) != Error::OK) 
    return false;
  return true;
}

void FileSystemBroker::send_request_sync(Protocol::Req::Base::Ptr hdlr, 
                                         std::promise<void> res){
  while(!send_request(hdlr));
  res.get_future().wait();
}

/// File/Dir name actions

bool FileSystemBroker::exists(int &err, const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Exists>(
    cfg_timeout->get(), name);
    
  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;    
  return hdlr->state;
}

void FileSystemBroker::exists(Callback::ExistsCb_t cb,  
                              const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Exists>(
    cfg_timeout->get(), name, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::remove(int &err, const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Remove>(
    cfg_timeout->get(), name);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::remove(Callback::RemoveCb_t cb, 
                              const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Remove>(
    cfg_timeout->get(), name, cb);
      
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::length(int &err, const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Length>(
    cfg_timeout->get(), name);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
  return hdlr->length;
}

void FileSystemBroker::length(Callback::LengthCb_t cb, 
                              const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Length>(
    cfg_timeout->get(), name, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::mkdirs(int &err, const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Mkdirs>(
    cfg_timeout->get(), name);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::mkdirs(Callback::MkdirsCb_t cb,  
                              const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Mkdirs>(
    cfg_timeout->get(), name, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::readdir(int &err, const std::string &name, 
                                DirentList &results) {
  auto hdlr = std::make_shared<Protocol::Req::Readdir>(
    cfg_timeout->get(), name);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
  results = hdlr->listing;
}

void FileSystemBroker::readdir(Callback::ReaddirCb_t cb, 
                               const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Readdir>(
    cfg_timeout->get(), name, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::rmdir(int &err, const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Rmdir>(
    cfg_timeout->get(), name);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::rmdir(Callback::RmdirCb_t cb, const std::string &name) {
  auto hdlr = std::make_shared<Protocol::Req::Rmdir>(
    cfg_timeout->get(), name, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::rename(int &err, const std::string &from, 
                              const std::string &to)  {
  auto hdlr = std::make_shared<Protocol::Req::Rename>(
    cfg_timeout->get(), from, to);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::rename(Callback::RenameCb_t cb, 
                              const std::string &from, 
                              const std::string &to)  {
  auto hdlr = std::make_shared<Protocol::Req::Rename>(
    cfg_timeout->get(), from, to, cb);
      
  while(!send_request(hdlr));
}

/// SmartFd actions

void FileSystemBroker::write(int &err, SmartFd::Ptr &smartfd,
                             int32_t replication, int64_t blksz, 
                             StaticBuffer &buffer) {
  auto hdlr = std::make_shared<Protocol::Req::Write>(
    cfg_timeout->get(), smartfd, replication, blksz, buffer);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::write(Callback::WriteCb_t cb, SmartFd::Ptr &smartfd,
                             int32_t replication, int64_t blksz, 
                             StaticBuffer &buffer) {
  auto hdlr = std::make_shared<Protocol::Req::Write>(
    cfg_timeout->get(), smartfd, replication, blksz, buffer, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::create(int &err, SmartFd::Ptr &smartfd,
                              int32_t bufsz, int32_t replication, 
                              int64_t blksz) {
  auto hdlr = std::make_shared<Protocol::Req::Create>(
    cfg_timeout->get(), smartfd, bufsz, replication, blksz);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::create(Callback::CreateCb_t cb, SmartFd::Ptr &smartfd,
                              int32_t bufsz, int32_t replication, 
                              int64_t blksz) {
  auto hdlr = std::make_shared<Protocol::Req::Create>(
    cfg_timeout->get(), smartfd, bufsz, replication, blksz, cb);
      
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::append(int &err, SmartFd::Ptr &smartfd, 
                                StaticBuffer &buffer, Flags flags) {
  auto hdlr = std::make_shared<Protocol::Req::Append>(
    cfg_timeout->get()+buffer.size/cfg_timeout_ratio->get(), 
    smartfd, buffer, flags);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
  return hdlr->amount;
}
   
void FileSystemBroker::append(Callback::AppendCb_t cb, SmartFd::Ptr &smartfd, 
                              StaticBuffer &buffer, Flags flags) {
  auto hdlr = std::make_shared<Protocol::Req::Append>(
    cfg_timeout->get()+buffer.size/cfg_timeout_ratio->get(), 
    smartfd, buffer, flags, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz) {
  auto hdlr = std::make_shared<Protocol::Req::Open>(
    cfg_timeout->get(), smartfd, bufsz);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::open(Callback::OpenCb_t cb, SmartFd::Ptr &smartfd, 
                            int32_t bufsz) {
  auto hdlr = std::make_shared<Protocol::Req::Open>(
    cfg_timeout->get(), smartfd, bufsz, cb);
      
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::read(int &err, SmartFd::Ptr &smartfd, 
                              void* dst, size_t amount) {
  auto hdlr = std::make_shared<Protocol::Req::Read>(
    cfg_timeout->get()+amount/cfg_timeout_ratio->get(), 
    smartfd, dst, amount, true);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
  return hdlr->amount;
}
   
size_t FileSystemBroker::read(int &err, SmartFd::Ptr &smartfd, 
                              StaticBuffer* dst, size_t amount) {
  auto hdlr = std::make_shared<Protocol::Req::Read>(
    cfg_timeout->get()+amount/cfg_timeout_ratio->get(), 
    smartfd, (void*)dst, amount, false);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
  return hdlr->amount;
}
   
void FileSystemBroker::read(Callback::ReadCb_t cb, SmartFd::Ptr &smartfd, 
                            size_t amount) {
  auto hdlr = std::make_shared<Protocol::Req::Read>(
    cfg_timeout->get()+amount/cfg_timeout_ratio->get(), 
    smartfd, nullptr, amount, false, cb);
      
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::pread(int &err, SmartFd::Ptr &smartfd, 
                               uint64_t offset, void* dst, size_t amount) {
  auto hdlr = std::make_shared<Protocol::Req::Pread>(
    cfg_timeout->get()+amount/cfg_timeout_ratio->get(), 
    smartfd, offset, dst, amount, true);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
  return hdlr->amount;
}

size_t FileSystemBroker::pread(int &err, SmartFd::Ptr &smartfd, 
                               uint64_t offset, StaticBuffer* dst, 
                               size_t amount) {
    auto hdlr = std::make_shared<Protocol::Req::Pread>(
      cfg_timeout->get()+amount/cfg_timeout_ratio->get(), 
      smartfd, offset, (void*)dst, amount, false);

    send_request_sync(hdlr, hdlr->promise());
    err = hdlr->error;
    return hdlr->amount;
  }
   
void FileSystemBroker::pread(Callback::ReadCb_t cb, SmartFd::Ptr &smartfd, 
                             uint64_t offset, size_t amount) {
  auto hdlr = std::make_shared<Protocol::Req::Pread>(
    cfg_timeout->get()+amount/cfg_timeout_ratio->get(), 
    smartfd, offset, nullptr, amount, false, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::seek(int &err, SmartFd::Ptr &smartfd, size_t offset) {
  auto hdlr = std::make_shared<Protocol::Req::Seek>(
    cfg_timeout->get(), smartfd, offset);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}
   
void FileSystemBroker::seek(Callback::SeekCb_t cb, SmartFd::Ptr &smartfd, 
                            size_t offset) {
  auto hdlr = std::make_shared<Protocol::Req::Seek>(
    cfg_timeout->get(), smartfd, offset, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::flush(int &err, SmartFd::Ptr &smartfd) {
  auto hdlr = std::make_shared<Protocol::Req::Flush>(
    cfg_timeout->get(), smartfd);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}
  
void FileSystemBroker::flush(Callback::FlushCb_t cb, SmartFd::Ptr &smartfd) {
  auto hdlr = std::make_shared<Protocol::Req::Flush>(
    cfg_timeout->get(), smartfd, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::sync(int &err, SmartFd::Ptr &smartfd) {
  auto hdlr = std::make_shared<Protocol::Req::Sync>(
    cfg_timeout->get(), smartfd);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}
  
void FileSystemBroker::sync(Callback::SyncCb_t cb, SmartFd::Ptr &smartfd) {
  auto hdlr = std::make_shared<Protocol::Req::Sync>(
    cfg_timeout->get(), smartfd, cb);
      
  while(!send_request(hdlr));
}

void FileSystemBroker::close(int &err, SmartFd::Ptr &smartfd) {
  auto hdlr = std::make_shared<Protocol::Req::Close>(
    cfg_timeout->get(), smartfd);

  send_request_sync(hdlr, hdlr->promise());
  err = hdlr->error;
}

void FileSystemBroker::close(Callback::CreateCb_t cb, SmartFd::Ptr &smartfd) {
  auto hdlr = std::make_shared<Protocol::Req::Close>(
    cfg_timeout->get(), smartfd, cb);
      
  while(!send_request(hdlr));
}


}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_broker(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemBroker());
};
void fs_apply_cfg_broker(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
};
}
