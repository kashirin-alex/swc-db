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
#include "swcdb/fs/Broker/Protocol/req/CombiPread.h"
#include "swcdb/fs/Broker/Protocol/req/CombiPreadSync.h"
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


Configurables* apply_broker(Configurables* config) {
  config->settings->file_desc.add_options()
    ("swc.fs.broker.cfg.dyn", Config::strs(),
     "Dyn-config file")

    ("swc.fs.broker.host", Config::str(),
     "FsBroker host (default by hostname)")
    ("swc.fs.broker.port", Config::i16(14000),
     "FsBroker port")

    ("swc.fs.broker.metrics.enabled", Config::boo(true),
     "Enable or Disable Metrics Tracking")

    ("swc.fs.broker.handlers", Config::i32(48),
     "Handlers for broker tasks")

    ("swc.fs.broker.timeout", Config::g_i32(120000),
     "Default request timeout in ms")
    ("swc.fs.broker.timeout.bytes.ratio", Config::g_i32(1000),
     "Timeout ratio to bytes, bytes/ratio=ms added to default timeout")

    ("swc.fs.broker.comm.encoder",
      Config::g_enum(
        int(SWC_DEFAULT_COMM_ENCODER),
        nullptr,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding),
     "The encoding to use in communication, options PLAIN/ZSTD/SNAPPY/ZLIB")

    ("swc.fs.broker.fds.max", Config::g_i32(256),
      "Max Open Fds for opt. without closing")
  ;
  config->settings->parse_file(
    config->settings->get_str("swc.fs.broker.cfg", ""),
    "swc.fs.broker.cfg.dyn"
  );

  config->cfg_fds_max = config->settings
    ->get<Config::Property::Value_int32_g>("swc.fs.broker.fds.max");
  config->stats_enabled = config->settings->get_bool(
    "swc.fs.broker.metrics.enabled");
  return config;
}



Comm::EndPoints
FileSystemBroker::get_endpoints(const Config::Settings::Ptr& settings) {
  std::string host = settings->get_str(
    "swc.fs.broker.host", "");
  if(host.empty()) {
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) == -1)
      SWC_THROW(errno, "gethostname");
    host.append(hostname);
  }

  Comm::Networks nets;
  asio::error_code ec;
  Comm::Resolver::get_networks(
    settings->get_strs("swc.comm.network.priority"), nets, ec);
  if(ec)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "swc.comm.network.priority error(%s)",
              ec.message().c_str());

  Config::Strings addr;
  return Comm::Resolver::get_endpoints(
    settings->get_i16("swc.fs.broker.port"),
    addr, host, nets, true
  );
}



FileSystemBroker::FileSystemBroker(Configurables* config)
    : FileSystem(apply_broker(config), ImplOptions(true)),
      m_io(new Comm::IoContext("FsBroker",
        settings->get_i32("swc.fs.broker.handlers"))),
      m_service(
        new Comm::client::Serialized(
          *settings,
          "FS-BROKER",
          m_io,
          client::FsBroker::AppContext::Ptr(
            new client::FsBroker::AppContext(*settings))
        )
      ),
      m_type_underlying(fs_type(
        settings->get_str("swc.fs.broker.underlying"))),
      m_endpoints(get_endpoints(settings)),
      m_connections(),
      cfg_timeout(
        settings->get<Config::Property::Value_int32_g>(
          "swc.fs.broker.timeout")),
      cfg_timeout_ratio(
        settings->get<Config::Property::Value_int32_g>(
          "swc.fs.broker.timeout.bytes.ratio")) {
}

FileSystemBroker::~FileSystemBroker() noexcept { }

void FileSystemBroker::stop() {
  m_run.store(false);
  m_service->stop();
  for(Comm::ConnHandlerPtr conn; m_connections.pop(&conn); ) conn->do_close();
  m_io->stop();
  FileSystem::stop();
}

Type FileSystemBroker::get_type() const noexcept {
  return Type::BROKER;
}

Type FileSystemBroker::get_type_underlying() const noexcept {
  return m_type_underlying;
}

std::string FileSystemBroker::to_string() const {
  return format(
    "(type=BROKER underlying-type=%s)",
    FS::to_string(m_type_underlying)
  );
}


bool FileSystemBroker::send_request(
      Comm::Protocol::FsBroker::Req::Base::Ptr hdlr) {
  Comm::ConnHandlerPtr conn;
  while(m_run && (!m_connections.pop(&conn) || !conn->is_open()) &&
        !(conn = m_service->get_connection(
                    m_endpoints, std::chrono::milliseconds(20000), 3)));
  if(m_run) {
    m_connections.push(conn);
    return conn->send_request(hdlr->cbp, hdlr);
  } else {
    if(conn)
      conn->do_close();
    hdlr->handle(conn, Comm::Event::make(Error::SERVER_SHUTTING_DOWN));
  }
  return true;
}

/// File/Dir name actions

bool FileSystemBroker::exists(int& err, const std::string& name) {
  SWC_FS_EXISTS_START(name);

  Comm::Protocol::FsBroker::Req::ExistsSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::ExistsSync(
      statistics, cfg_timeout->get(), name)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->state;
}

void FileSystemBroker::exists(Callback::ExistsCb_t&& cb,
                              const std::string& name) {
  SWC_FS_EXISTS_START(name);

  Comm::Protocol::FsBroker::Req::Exists::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Exists(
      statistics, cfg_timeout->get(), name, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::remove(int& err, const std::string& name) {
  SWC_FS_REMOVE_START(name);

  Comm::Protocol::FsBroker::Req::RemoveSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::RemoveSync(
      statistics, cfg_timeout->get(), name));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::remove(Callback::RemoveCb_t&& cb,
                              const std::string& name) {
  SWC_FS_REMOVE_START(name);

  Comm::Protocol::FsBroker::Req::Remove::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Remove(
      statistics, cfg_timeout->get(), name, std::move(cb)));
  while(!send_request(hdlr));
}

size_t FileSystemBroker::length(int& err, const std::string& name) {
  SWC_FS_LENGTH_START(name);

  Comm::Protocol::FsBroker::Req::LengthSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::LengthSync(
      statistics, cfg_timeout->get(), name));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->length;
}

void FileSystemBroker::length(Callback::LengthCb_t&& cb,
                              const std::string& name) {
  SWC_FS_LENGTH_START(name);

  Comm::Protocol::FsBroker::Req::Length::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Length(
    statistics, cfg_timeout->get(), name, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::mkdirs(int& err, const std::string& name) {
  SWC_FS_MKDIRS_START(name);

  Comm::Protocol::FsBroker::Req::MkdirsSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::MkdirsSync(
      statistics, cfg_timeout->get(), name));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::mkdirs(Callback::MkdirsCb_t&& cb,
                              const std::string& name) {
  SWC_FS_MKDIRS_START(name);

  Comm::Protocol::FsBroker::Req::Mkdirs::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Mkdirs(
    statistics, cfg_timeout->get(), name, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::readdir(int& err, const std::string& name,
                                DirentList& results) {
  SWC_FS_READDIR_START(name);

  Comm::Protocol::FsBroker::Req::ReaddirSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::ReaddirSync(
      statistics, cfg_timeout->get(), name, results));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::readdir(Callback::ReaddirCb_t&& cb,
                               const std::string& name) {
  SWC_FS_READDIR_START(name);

  Comm::Protocol::FsBroker::Req::Readdir::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Readdir(
      statistics, cfg_timeout->get(), name, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::rmdir(int& err, const std::string& name) {
  SWC_FS_RMDIR_START(name);

  Comm::Protocol::FsBroker::Req::RmdirSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::RmdirSync(
      statistics, cfg_timeout->get(), name));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::rmdir(Callback::RmdirCb_t&& cb,
                             const std::string& name) {
  SWC_FS_RMDIR_START(name);

  Comm::Protocol::FsBroker::Req::Rmdir::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Rmdir(
      statistics, cfg_timeout->get(), name, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::rename(int& err, const std::string& from,
                              const std::string& to)  {
  SWC_FS_RENAME_START(from, to);

  Comm::Protocol::FsBroker::Req::RenameSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::RenameSync(
      statistics, cfg_timeout->get(), from, to));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::rename(Callback::RenameCb_t&& cb,
                              const std::string& from,
                              const std::string& to)  {
  SWC_FS_RENAME_START(from, to);

  Comm::Protocol::FsBroker::Req::Rename::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Rename(
      statistics, cfg_timeout->get(), from, to, std::move(cb)));
  while(!send_request(hdlr));
}

/// SmartFd actions

void FileSystemBroker::write(int& err, SmartFd::Ptr& smartfd,
                             uint8_t replication, StaticBuffer& buffer) {
  SWC_FS_WRITE_START(smartfd, replication, buffer.size);

  Comm::Protocol::FsBroker::Req::WriteSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::WriteSync(
      statistics, cfg_timeout->get(), smartfd, replication, buffer));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::write(Callback::WriteCb_t&& cb,
                             SmartFd::Ptr& smartfd,
                             uint8_t replication,
                             StaticBuffer& buffer) {
  SWC_FS_WRITE_START(smartfd, replication, buffer.size);

  Comm::Protocol::FsBroker::Req::Write::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Write(
      statistics,
      cfg_timeout->get(), smartfd, replication, buffer, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::read(int& err, const std::string& name,
                            StaticBuffer* dst) {
  SWC_FS_READALL_START(name);

  Comm::Protocol::FsBroker::Req::ReadAllSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::ReadAllSync(
      statistics, cfg_timeout->get(), name, dst));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::read(Callback::ReadAllCb_t&& cb,
                            const std::string& name) {
  SWC_FS_READALL_START(name);

  Comm::Protocol::FsBroker::Req::ReadAll::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::ReadAll(
      statistics, cfg_timeout->get(), name, std::move(cb))
  );
  while(!send_request(hdlr));
}

void FileSystemBroker::combi_pread(int& err, SmartFd::Ptr& smartfd,
                                   uint64_t offset, uint32_t amount,
                                   StaticBuffer* dst) {
  SWC_FS_COMBI_PREAD_START(smartfd, offset, amount);

  Comm::Protocol::FsBroker::Req::CombiPreadSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::CombiPreadSync(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, offset, amount, dst)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::combi_pread(Callback::CombiPreadCb_t&& cb,
                                   SmartFd::Ptr& smartfd,
                                   uint64_t offset, uint32_t amount) {
  SWC_FS_COMBI_PREAD_START(smartfd, offset, amount);

  Comm::Protocol::FsBroker::Req::CombiPread::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::CombiPread(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, offset, amount, std::move(cb))
  );
  while(!send_request(hdlr));
}

void FileSystemBroker::create(int& err, SmartFd::Ptr& smartfd,
                              uint8_t replication) {
  SWC_FS_CREATE_START(smartfd, replication);

  Comm::Protocol::FsBroker::Req::CreateSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::CreateSync(
      shared_from_this(),
      cfg_timeout->get(), smartfd, replication)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::create(Callback::CreateCb_t&& cb,
                              SmartFd::Ptr& smartfd,
                              uint8_t replication) {
  SWC_FS_CREATE_START(smartfd, replication);

  Comm::Protocol::FsBroker::Req::Create::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Create(
      shared_from_this(),
      cfg_timeout->get(), smartfd, replication, std::move(cb))
  );
  while(!send_request(hdlr));
}

size_t FileSystemBroker::append(int& err, SmartFd::Ptr& smartfd,
                                StaticBuffer& buffer, Flags flags) {
  SWC_FS_APPEND_START(smartfd, buffer.size, flags);

  Comm::Protocol::FsBroker::Req::AppendSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::AppendSync(
      statistics,
      cfg_timeout->get() + buffer.size/cfg_timeout_ratio->get(),
      smartfd, buffer, flags)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}

void FileSystemBroker::append(Callback::AppendCb_t&& cb,
                              SmartFd::Ptr& smartfd,
                              StaticBuffer& buffer, Flags flags) {
  SWC_FS_APPEND_START(smartfd, buffer.size, flags);

  Comm::Protocol::FsBroker::Req::Append::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Append(
      statistics,
      cfg_timeout->get() + buffer.size/cfg_timeout_ratio->get(),
      smartfd, buffer, flags, std::move(cb))
  );
  while(!send_request(hdlr));
}

void FileSystemBroker::open(int& err, SmartFd::Ptr& smartfd) {
  SWC_FS_OPEN_START(smartfd);

  Comm::Protocol::FsBroker::Req::OpenSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::OpenSync(
      shared_from_this(), cfg_timeout->get(), smartfd));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::open(Callback::OpenCb_t&& cb, SmartFd::Ptr& smartfd) {
  SWC_FS_OPEN_START(smartfd);

  Comm::Protocol::FsBroker::Req::Open::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Open(
    shared_from_this(), cfg_timeout->get(), smartfd, std::move(cb)));
  while(!send_request(hdlr));
}

size_t FileSystemBroker::read(int& err, SmartFd::Ptr& smartfd,
                              void* dst, size_t amount) {
  SWC_FS_READ_START(smartfd, amount);

  Comm::Protocol::FsBroker::Req::ReadSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::ReadSync(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, dst, amount, true)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}

size_t FileSystemBroker::read(int& err, SmartFd::Ptr& smartfd,
                              StaticBuffer* dst, size_t amount) {
  SWC_FS_READ_START(smartfd, amount);

  Comm::Protocol::FsBroker::Req::ReadSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::ReadSync(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, static_cast<void*>(dst), amount, false)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}

void FileSystemBroker::read(Callback::ReadCb_t&& cb,
                            SmartFd::Ptr& smartfd, size_t amount) {
  SWC_FS_READ_START(smartfd, amount);

  Comm::Protocol::FsBroker::Req::Read::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Read(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, amount, std::move(cb))
  );
  while(!send_request(hdlr));
}

size_t FileSystemBroker::pread(int& err, SmartFd::Ptr& smartfd,
                               uint64_t offset, void* dst,
                               size_t amount) {
  SWC_FS_PREAD_START(smartfd, offset, amount);

  Comm::Protocol::FsBroker::Req::PreadSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::PreadSync(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, offset, dst, amount, true)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}

size_t FileSystemBroker::pread(int& err, SmartFd::Ptr& smartfd,
                               uint64_t offset, StaticBuffer* dst,
                               size_t amount) {
  SWC_FS_PREAD_START(smartfd, offset, amount);

  Comm::Protocol::FsBroker::Req::PreadSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::PreadSync(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, offset, static_cast<void*>(dst), amount, false)
  );
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
  return hdlr->amount;
}

void FileSystemBroker::pread(Callback::ReadCb_t&& cb,
                             SmartFd::Ptr& smartfd,
                             uint64_t offset, size_t amount) {
  SWC_FS_PREAD_START(smartfd, offset, amount);

  Comm::Protocol::FsBroker::Req::Pread::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Pread(
      statistics,
      cfg_timeout->get() + amount/cfg_timeout_ratio->get(),
      smartfd, offset, amount, std::move(cb))
  );
  while(!send_request(hdlr));
}

void FileSystemBroker::seek(int& err, SmartFd::Ptr& smartfd, size_t offset) {
  SWC_FS_SEEK_START(smartfd, offset);

  Comm::Protocol::FsBroker::Req::SeekSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::SeekSync(
    statistics, cfg_timeout->get(), smartfd, offset));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::seek(Callback::SeekCb_t&& cb,
                            SmartFd::Ptr& smartfd,
                            size_t offset) {
  SWC_FS_SEEK_START(smartfd, offset);

  Comm::Protocol::FsBroker::Req::Seek::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Seek(
      statistics, cfg_timeout->get(), smartfd, offset, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::flush(int& err, SmartFd::Ptr& smartfd) {
  SWC_FS_FLUSH_START(smartfd);

  Comm::Protocol::FsBroker::Req::FlushSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::FlushSync(
    statistics, cfg_timeout->get(), smartfd));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::flush(Callback::FlushCb_t&& cb,
                             SmartFd::Ptr& smartfd) {
  SWC_FS_FLUSH_START(smartfd);

  Comm::Protocol::FsBroker::Req::Flush::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Flush(
    statistics, cfg_timeout->get(), smartfd, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::sync(int& err, SmartFd::Ptr& smartfd) {
  SWC_FS_SYNC_START(smartfd);

  Comm::Protocol::FsBroker::Req::SyncSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::SyncSync(
      statistics, cfg_timeout->get(), smartfd));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::sync(Callback::SyncCb_t&& cb,
                            SmartFd::Ptr& smartfd) {
  SWC_FS_SYNC_START(smartfd);

  Comm::Protocol::FsBroker::Req::Sync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Sync(
    statistics, cfg_timeout->get(), smartfd, std::move(cb)));
  while(!send_request(hdlr));
}

void FileSystemBroker::close(int& err, SmartFd::Ptr& smartfd) {
  SWC_FS_CLOSE_START(smartfd);

  Comm::Protocol::FsBroker::Req::CloseSync::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::CloseSync(
    shared_from_this(), cfg_timeout->get(), smartfd));
  while(!send_request(hdlr));
  hdlr->wait();
  err = hdlr->error;
}

void FileSystemBroker::close(Callback::CloseCb_t&& cb,
                             SmartFd::Ptr& smartfd) {
  SWC_FS_CLOSE_START(smartfd);

  Comm::Protocol::FsBroker::Req::Close::Ptr hdlr(
    new Comm::Protocol::FsBroker::Req::Close(
      shared_from_this(), cfg_timeout->get(), smartfd, std::move(cb)));
  while(!send_request(hdlr));
}


}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_broker(SWC::FS::Configurables* config) {
  return static_cast<SWC::FS::FileSystem*>(
    new SWC::FS::FileSystemBroker(config));
}
}
