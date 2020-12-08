/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/ConnHandler.h"

#include "swcdb/fs/Broker/FileSystem.h"
#include "swcdb/fs/Broker/Protocol/Commands.h"
#include "swcdb/fs/Broker/Protocol/req/Base.h"
#include "swcdb/fs/Broker/AppContext.h"
 
#include "swcdb/fs/Broker/Protocol/req/Exists.h"
#include "swcdb/fs/Broker/Protocol/req/ExistsSync.h"
#include "swcdb/fs/Broker/Protocol/req/Remove.h"
#include "swcdb/fs/Broker/Protocol/req/RemoveSync.h"
#include "swcdb/fs/Broker/Protocol/req/Length.h"
#include "swcdb/fs/Broker/Protocol/req/LengthSync.h"
#include "swcdb/fs/Broker/Protocol/req/Mkdirs.h"
#include "swcdb/fs/Broker/Protocol/req/MkdirsSync.h"
#include "swcdb/fs/Broker/Protocol/req/Readdir.h"
#include "swcdb/fs/Broker/Protocol/req/ReaddirSync.h"
#include "swcdb/fs/Broker/Protocol/req/Rmdir.h"
#include "swcdb/fs/Broker/Protocol/req/RmdirSync.h"
#include "swcdb/fs/Broker/Protocol/req/Rename.h"
#include "swcdb/fs/Broker/Protocol/req/RenameSync.h"
#include "swcdb/fs/Broker/Protocol/req/Write.h"
#include "swcdb/fs/Broker/Protocol/req/WriteSync.h"
#include "swcdb/fs/Broker/Protocol/req/ReadAll.h"
#include "swcdb/fs/Broker/Protocol/req/ReadAllSync.h"
#include "swcdb/fs/Broker/Protocol/req/Create.h"
#include "swcdb/fs/Broker/Protocol/req/CreateSync.h"
#include "swcdb/fs/Broker/Protocol/req/Append.h"
#include "swcdb/fs/Broker/Protocol/req/AppendSync.h"
#include "swcdb/fs/Broker/Protocol/req/Open.h"
#include "swcdb/fs/Broker/Protocol/req/OpenSync.h"
#include "swcdb/fs/Broker/Protocol/req/Read.h"
#include "swcdb/fs/Broker/Protocol/req/ReadSync.h"
#include "swcdb/fs/Broker/Protocol/req/Pread.h"
#include "swcdb/fs/Broker/Protocol/req/PreadSync.h"
#include "swcdb/fs/Broker/Protocol/req/Seek.h"
#include "swcdb/fs/Broker/Protocol/req/SeekSync.h"
#include "swcdb/fs/Broker/Protocol/req/Flush.h"
#include "swcdb/fs/Broker/Protocol/req/FlushSync.h"
#include "swcdb/fs/Broker/Protocol/req/Sync.h"
#include "swcdb/fs/Broker/Protocol/req/SyncSync.h"
#include "swcdb/fs/Broker/Protocol/req/Close.h"
#include "swcdb/fs/Broker/Protocol/req/CloseSync.h"


namespace SWC { namespace FS {


Configurables apply_broker() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.broker.cfg.dyn", Config::strs(),
     "Dyn-config file")

    ("swc.fs.broker.host", Config::str(),
     "FsBroker host (default by hostname)") 
    ("swc.fs.broker.port", Config::i16(17000),
     "FsBroker port")

    ("swc.fs.broker.handlers", Config::i32(48),
     "Handlers for broker tasks")

    ("swc.fs.broker.timeout", Config::g_i32(120000),
     "Default request timeout in ms")
    ("swc.fs.broker.timeout.bytes.ratio", Config::g_i32(1000), 
     "Timeout ratio to bytes, bytes/ratio=ms added to default timeout")

    ("swc.fs.broker.comm.encoder", 
      Config::g_enum(
        (int)SWC_DEFAULT_COMM_ENCODER,
        0,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.fs.broker.fds.max", Config::g_i32(256), 
      "Max Open Fds for opt. without closing")
  ;
  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.broker.cfg", ""),
    "swc.fs.broker.cfg.dyn"
  );
  Configurables config;
  config.cfg_fds_max = Env::Config::settings()
    ->get<Config::Property::V_GINT32>("swc.fs.broker.fds.max");
  return config;
}



Comm::EndPoints FileSystemBroker::get_endpoints() {
  std::string host = Env::Config::settings()->get_str(
    "swc.fs.broker.host", "");
  if(host.empty()) {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }

  std::vector<Comm::Network> nets;
  asio::error_code ec;
  Comm::Resolver::get_networks(
    Env::Config::settings()->get_strs("swc.comm.network.priority"), nets, ec);
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "swc.comm.network.priority error(%s)",
              ec.message().c_str());

  Config::Strings addr;
  return Comm::Resolver::get_endpoints(
    Env::Config::settings()->get_i16("swc.fs.broker.port"),
    addr, host, nets, true
  );
}



FileSystemBroker::FileSystemBroker()
  : FileSystem(apply_broker()),
    m_io(std::make_shared<Comm::IoContext>("FsBroker",
      Env::Config::settings()->get_i32("swc.fs.broker.handlers"))),
    m_service(
      std::make_shared<Comm::client::Serialized>(
        "FS-BROKER", 
        m_io->shared(), 
        std::make_shared<client::FsBroker::AppContext>()
      )
    ),
    m_type_underlying(fs_type(
      Env::Config::settings()->get_str("swc.fs.broker.underlying"))),
    m_endpoints(get_endpoints()),
    m_run(true),
    cfg_timeout(
      Env::Config::settings()->get<Config::Property::V_GINT32>(
        "swc.fs.broker.timeout")),
    cfg_timeout_ratio(
      Env::Config::settings()->get<Config::Property::V_GINT32>( 
        "swc.fs.broker.timeout.bytes.ratio")) {
  m_io->run(m_io);
}

FileSystemBroker::~FileSystemBroker() { }

void FileSystemBroker::stop() {
  m_run = false;
  m_service->stop();
  m_io->stop();
  FileSystem::stop();
}

Type FileSystemBroker::get_type() {
  return m_type_underlying;
}

std::string FileSystemBroker::to_string() {
  return format(
    "(type=BROKER underlying-type=%s)",
    FS::to_string(m_type_underlying).c_str()
  );
}


bool FileSystemBroker::send_request(
      Comm::Protocol::FsBroker::Req::Base::Ptr hdlr) {
  Comm::ConnHandlerPtr conn = nullptr;
  do {
    if(!m_run) {
      auto ev = Comm::Event::make(
        Comm::Event::Type::ERROR, Error::SERVER_SHUTTING_DOWN);
      hdlr->handle(conn, ev);
      return true;
    }
  } while(!(conn = m_service->get_connection(
    m_endpoints, std::chrono::milliseconds(20000), 3)));

  m_service->preserve(conn);

  return conn->send_request(hdlr->cbp, hdlr);
}

/// File/Dir name actions

bool FileSystemBroker::exists(int& err, const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "exists path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::ExistsSync>(
    cfg_timeout->get(), name);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;    
  return hdlr->state;
}

void FileSystemBroker::exists(const Callback::ExistsCb_t& cb,  
                              const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "exists path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Exists>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::remove(int& err, const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "remove path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::RemoveSync>(
    cfg_timeout->get(), name);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::remove(const Callback::RemoveCb_t& cb, 
                              const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "remove path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Remove>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::length(int& err, const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "length path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::LengthSync>(
    cfg_timeout->get(), name);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->length;
}

void FileSystemBroker::length(const Callback::LengthCb_t& cb, 
                              const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "length path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Length>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::mkdirs(int& err, const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::MkdirsSync>(
    cfg_timeout->get(), name);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::mkdirs(const Callback::MkdirsCb_t& cb,  
                              const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Mkdirs>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::readdir(int& err, const std::string& name, 
                                DirentList& results) {
  SWC_LOGF(LOG_DEBUG, "readdir path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::ReaddirSync>(
    cfg_timeout->get(), name, results);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::readdir(const Callback::ReaddirCb_t& cb, 
                               const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "readdir path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Readdir>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::rmdir(int& err, const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::RmdirSync>(
    cfg_timeout->get(), name);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::rmdir(const Callback::RmdirCb_t& cb, 
                             const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "rmdir path='%s'", name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Rmdir>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::rename(int& err, const std::string& from, 
                              const std::string& to)  {
  SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s'", from.c_str(), to.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::RenameSync>(
    cfg_timeout->get(), from, to);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::rename(const Callback::RenameCb_t& cb, 
                              const std::string& from, 
                              const std::string& to)  {
  SWC_LOGF(LOG_DEBUG, "rename '%s' to '%s'", from.c_str(), to.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Rename>(
    cfg_timeout->get(), from, to, cb);
  while(!send_request(hdlr));
}

/// SmartFd actions

void FileSystemBroker::write(int& err, SmartFd::Ptr& smartfd,
                             uint8_t replication, int64_t blksz, 
                             StaticBuffer& buffer) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("write amount=%lu replication(%u) blksz(%ld) timeout=%d ",
                    buffer.size, replication, blksz, cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::WriteSync>(
    cfg_timeout->get(), smartfd, replication, blksz, buffer);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::write(const Callback::WriteCb_t& cb, 
                             SmartFd::Ptr& smartfd,
                             uint8_t replication, int64_t blksz, 
                             StaticBuffer& buffer) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("write amount=%lu replication(%u) blksz(%ld) timeout=%d ",
                    buffer.size, replication, blksz, cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Write>(
    cfg_timeout->get(), smartfd, replication, blksz, buffer, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::read(int& err, const std::string& name, 
                            StaticBuffer* dst) {
  SWC_LOGF(LOG_DEBUG, "read-all timeout=%d %s", 
            cfg_timeout->get(), name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::ReadAllSync>(
    cfg_timeout->get(), name, dst);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::read(const Callback::ReadAllCb_t& cb, 
                            const std::string& name) {
  SWC_LOGF(LOG_DEBUG, "read-all timeout=%d %s", 
            cfg_timeout->get(), name.c_str());

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::ReadAll>(
    cfg_timeout->get(), name, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::create(int& err, SmartFd::Ptr& smartfd,
                              int32_t bufsz, uint8_t replication, 
                              int64_t blksz) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("create bufsz(%d) replication(%d) blksz(%ld) timeout=%d ", 
                    bufsz, replication, blksz, cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::CreateSync>(
    shared_from_this(), 
    cfg_timeout->get(), smartfd, bufsz, replication, blksz);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::create(const Callback::CreateCb_t& cb, 
                              SmartFd::Ptr& smartfd,
                              int32_t bufsz, uint8_t replication, 
                              int64_t blksz) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("create bufsz(%d) replication(%d) blksz(%ld) timeout=%d ", 
                    bufsz, replication, blksz, cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Create>(
    shared_from_this(), 
    cfg_timeout->get(), smartfd, bufsz, replication, blksz, cb);
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::append(int& err, SmartFd::Ptr& smartfd, 
                                StaticBuffer& buffer, Flags flags) {
  uint32_t timeout = cfg_timeout->get()+buffer.size/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("append flags=%d amount=%lu timeout=%d ",
                    flags, buffer.size, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::AppendSync>(
    timeout, smartfd, buffer, flags);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}
   
void FileSystemBroker::append(const Callback::AppendCb_t& cb, 
                              SmartFd::Ptr& smartfd, 
                              StaticBuffer& buffer, Flags flags) {
  uint32_t timeout = cfg_timeout->get()+buffer.size/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("append flags=%d amount=%lu timeout=%d ", 
                    flags, buffer.size, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Append>(
    timeout, smartfd, buffer, flags, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("open timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM); 
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::OpenSync>(
    shared_from_this(), cfg_timeout->get(), smartfd, bufsz);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::open(const Callback::OpenCb_t& cb, 
                            SmartFd::Ptr& smartfd, int32_t bufsz) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("open timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM); 
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Open>(
    shared_from_this(), cfg_timeout->get(), smartfd, bufsz, cb);
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::read(int& err, SmartFd::Ptr& smartfd,
                              void* dst, size_t amount) {
  uint32_t timeout = cfg_timeout->get() + amount/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read len=%lu timeout=%d ", amount, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::ReadSync>(
    timeout, smartfd, dst, amount, true);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}
   
size_t FileSystemBroker::read(int& err, SmartFd::Ptr& smartfd,
                              StaticBuffer* dst, size_t amount) {
  uint32_t timeout = cfg_timeout->get() + amount/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read len=%lu timeout=%d ", amount, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::ReadSync>(
    timeout, smartfd, (void*)dst, amount, false);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}
   
void FileSystemBroker::read(const Callback::ReadCb_t& cb,
                            SmartFd::Ptr& smartfd, size_t amount) {
  uint32_t timeout = cfg_timeout->get() + amount/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("read len=%lu timeout=%d ", amount, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Read>(
    timeout, smartfd, amount, cb);
  while(!send_request(hdlr));
}
  
size_t FileSystemBroker::pread(int& err, SmartFd::Ptr& smartfd,
                               uint64_t offset, void* dst, 
                               size_t amount) {
  uint32_t timeout = cfg_timeout->get() + amount/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("pread offset=%lu len=%lu timeout=%d ",
                    offset, amount, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );
  
  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::PreadSync>(
    timeout, smartfd, offset, dst, amount, true);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}

size_t FileSystemBroker::pread(int& err, SmartFd::Ptr& smartfd,
                               uint64_t offset, StaticBuffer* dst, 
                               size_t amount) {
  uint32_t timeout = cfg_timeout->get() + amount/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("pread offset=%lu len=%lu timeout=%d ",
                    offset, amount, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::PreadSync>(
    timeout, smartfd, offset, (void*)dst, amount, false);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}
   
void FileSystemBroker::pread(const Callback::ReadCb_t& cb,
                             SmartFd::Ptr& smartfd,
                             uint64_t offset, size_t amount) {
  uint32_t timeout = cfg_timeout->get() + amount/cfg_timeout_ratio->get();
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("pread offset=%lu len=%lu timeout=%d ", 
                    offset, amount, timeout);
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Pread>(
    timeout, smartfd, offset, amount, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::seek(int& err, SmartFd::Ptr& smartfd, size_t offset) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("seek offset=%lu timeout=%d ", offset, cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::SeekSync>(
    cfg_timeout->get(), smartfd, offset);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}
   
void FileSystemBroker::seek(const Callback::SeekCb_t& cb, 
                            SmartFd::Ptr& smartfd, 
                            size_t offset) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("seek offset=%lu timeout=%d ", offset, cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Seek>(
    cfg_timeout->get(), smartfd, offset, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::flush(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("flush timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::FlushSync>(
    cfg_timeout->get(), smartfd);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}
  
void FileSystemBroker::flush(const Callback::FlushCb_t& cb, 
                             SmartFd::Ptr& smartfd) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("flush timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Flush>(
    cfg_timeout->get(), smartfd, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::sync(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("sync timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::SyncSync>(
    cfg_timeout->get(), smartfd);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}
  
void FileSystemBroker::sync(const Callback::SyncCb_t& cb, 
                            SmartFd::Ptr& smartfd) {
  SWC_LOG_OUT(LOG_DEBUG,
    SWC_LOG_PRINTF("sync timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM);
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Sync>(
    cfg_timeout->get(), smartfd, cb);
  while(!send_request(hdlr));
}

void FileSystemBroker::close(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("close timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM); 
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::CloseSync>(
    shared_from_this(), cfg_timeout->get(), smartfd);
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::close(const Callback::CloseCb_t& cb, 
                             SmartFd::Ptr& smartfd) {
  SWC_LOG_OUT(LOG_DEBUG, 
    SWC_LOG_PRINTF("close timeout=%d ", cfg_timeout->get());
    smartfd->print(SWC_LOG_OSTREAM); 
  );

  auto hdlr = std::make_shared<Comm::Protocol::FsBroker::Req::Close>(
    shared_from_this(), cfg_timeout->get(), smartfd, cb);
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
