/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/FileSystem.h"


namespace SWC { namespace FS {


std::string normalize_pathname(std::string s) {
  if(*(--s.end()) != '/')
    s.append("/");
  return s;
}

Type fs_type(const std::string& fs_name) {

#if !defined (FS_BROKER_APP)
  if(!strncasecmp(fs_name.data(), "broker", fs_name.size()))
    return Type::BROKER;
#endif

  if(!strncasecmp(fs_name.data(), "local", fs_name.size()))
    return Type::LOCAL;

  if(!strncasecmp(fs_name.data(), "hadoop", fs_name.size()))
    return Type::HADOOP;

  if(!strncasecmp(fs_name.data(), "hadoop_jvm", fs_name.size()))
    return Type::HADOOP_JVM;

  if(!strncasecmp(fs_name.data(), "ceph", fs_name.size()))
    return Type::CEPH;

  if(!strncasecmp(fs_name.data(), "custom", fs_name.size()))
    return Type::CUSTOM;

  else
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Unknown FileSystem name=%s", fs_name.c_str());
  return Type::UNKNOWN;
}

const char* to_string(Type typ) noexcept {
  switch(typ) {
    case Type::LOCAL:
      return "Local";
    case Type::BROKER:
      return "Broker";
    case Type::HADOOP:
      return "Hadoop";
    case Type::HADOOP_JVM:
      return "HadoopJVM";
    case Type::CEPH:
      return "Ceph";
    case Type::CUSTOM:
      return "Custom";
    default:
      return "Unknown";
  }
}


FileSystem::FileSystem(const Configurables& config)
    : path_root(config.path_root.empty()
        ? "" : normalize_pathname(config.path_root)),
      path_data(
        normalize_pathname(
          Env::Config::settings()->get_str("swc.fs.path.data"))),
      cfg_fds_max(config.cfg_fds_max), fds_count(0), m_run(true) {
}

FileSystem::~FileSystem() {
}

void FileSystem::stop() {
  m_run.store(false);
  if(fds_count.load())
    SWC_LOGF(LOG_WARN, "FS %s remained with open-fds=%lu",
             to_string().c_str(), fds_count.load());
}

Type FileSystem::get_type() const noexcept {
  return Type::UNKNOWN;
}

std::string FileSystem::to_string() const {
  return format(
    "(type=UNKNOWN path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}

void FileSystem::get_abspath(const std::string& name, std::string& abspath) {
  abspath.append(path_root);
  if(!name.empty()){
    abspath.append(path_data);
    abspath.append(name);
  } else {
    abspath.append(path_data.substr(0, path_data.length()-1));
  }
}

void FileSystem::fd_open_incr() noexcept {
  fds_count.fetch_add(1);
}

void FileSystem::fd_open_decr() noexcept {
  fds_count.fetch_sub(1);
}

bool FileSystem::need_fds() const noexcept {
  return fds_count >= size_t(cfg_fds_max->get());
}

size_t FileSystem::fds_open() const noexcept {
  return fds_count.load();
}


/* Default functions of an async call,
 * used if a FileSystem(typ) does not implement
*/

void FileSystem::exists(const Callback::ExistsCb_t& cb,
                        const std::string& name) {
  int err = Error::OK;
  bool state = exists(err, name);
  cb(err, state);
}

void FileSystem::remove(const Callback::RemoveCb_t& cb,
                        const std::string& name) {
  int err = Error::OK;
  remove(err, name);
  cb(err);
}

void FileSystem::length(const Callback::LengthCb_t& cb,
                        const std::string& name) {
  int err = Error::OK;
  size_t len = length(err, name);
  cb(err, len);
}

void FileSystem::mkdirs(const Callback::MkdirsCb_t& cb,
                        const std::string& name) {
  int err = Error::OK;
  mkdirs(err, name);
  cb(err);
}

void FileSystem::readdir(const Callback::ReaddirCb_t& cb,
                         const std::string& name) {
  int err = Error::OK;
  DirentList listing;
  readdir(err, name, listing);
  cb(err, listing);
}

void FileSystem::rmdir(const Callback::RmdirCb_t& cb,
                       const std::string& name) {
  int err = Error::OK;
  rmdir(err, name);
  cb(err);
}

void FileSystem::rename(const Callback::RmdirCb_t& cb,
                        const std::string& from, const std::string& to) {
  int err = Error::OK;
  rename(err, from, to);
  cb(err);
}

void FileSystem::default_write(int& err, SmartFd::Ptr& smartfd,
                               uint8_t replication, int64_t blksz,
                               StaticBuffer& buffer) {
  SWC_LOG_OUT(LOG_DEBUG, smartfd->print(SWC_LOG_OSTREAM << "write "); );

  create(err, smartfd, 0, replication, blksz);
  if(!smartfd->valid() || err) {
    if(!err)
      err = EBADF;
    goto finish;
  }

  if(buffer.size) {
    append(err, smartfd, buffer, Flags::FLUSH);
    if(err)
      goto finish;
  }

  finish:
    int errtmp;
    if(smartfd->valid())
      close(err ? errtmp : err, smartfd);

  if(err)
    SWC_LOG_OUT(LOG_ERROR,
      Error::print(SWC_LOG_OSTREAM <<  "write failed: ", err);
      smartfd->print(SWC_LOG_OSTREAM << " ");
    );
}

void FileSystem::write(const Callback::WriteCb_t& cb, SmartFd::Ptr& smartfd,
                       uint8_t replication, int64_t blksz,
                       StaticBuffer& buffer) {
  int err = Error::OK;
  write(err, smartfd, replication, blksz, buffer);
  cb(err, smartfd);
}

void FileSystem::default_read(int& err, const std::string& name,
                              StaticBuffer* buffer) {
  SWC_LOGF(LOG_DEBUG, "read-all %s", name.c_str());

  size_t len;
  FS::SmartFd::Ptr smartfd;

  if(!exists(err, name)) {
    if(!err)
      err = Error::FS_PATH_NOT_FOUND;
    goto finish;
  }
  len = length(err, name);
  if(err)
    goto finish;

  smartfd = FS::SmartFd::make_ptr(name, 0);

  open(err, smartfd);
  if(!err && !smartfd->valid())
    err = EBADR;
  if(!err) {
    buffer->free();
    if(read(err, smartfd, buffer, len) != len)
      err = Error::FS_EOF;
  }
  if(smartfd->valid()) {
    int errtmp;
    close(err ? errtmp : err, smartfd);
  }

  finish:
  if(err)
    SWC_LOGF(LOG_ERROR, "read-all failed: %d(%s), %s",
              err, Error::get_text(err), name.c_str());
}

void FileSystem::read(const Callback::ReadAllCb_t& cb,
                      const std::string& name) {
  int err = Error::OK;
  auto dst = std::make_shared<StaticBuffer>();
  read(err, name, dst.get());
  cb(err, name, dst);
}

void FileSystem::default_combi_pread(int& err, SmartFd::Ptr& smartfd,
                                     uint64_t offset, uint32_t amount,
                                     StaticBuffer* buffer) {
  SWC_LOGF(LOG_DEBUG, "combi-pread %s offset=%lu amount=%u",
           smartfd->filepath().c_str(), offset, amount);

  open(err, smartfd);
  if(!smartfd->valid()) {
    if(!err)
      err = EBADR;
  }
  if(err)
    goto finish;

  buffer->free();
  if(pread(err, smartfd, offset, buffer, amount) != amount)
    err = Error::FS_EOF;

  finish:
    int errtmp;
    if(smartfd->valid())
      close(err ? errtmp : err, smartfd);

  if(err)
    SWC_LOGF(LOG_ERROR, "combi-pread failed: %d(%s), %s",
              err, Error::get_text(err), smartfd->filepath().c_str());
}

void FileSystem::combi_pread(const Callback::CombiPreadCb_t& cb,
                             SmartFd::Ptr& smartfd,
                             uint64_t offset, uint32_t amount) {
  int err = Error::OK;
  auto dst = std::make_shared<StaticBuffer>();
  combi_pread(err, smartfd, offset, amount, dst.get());
  cb(err, smartfd, dst);
}

void FileSystem::create(const Callback::CreateCb_t& cb, SmartFd::Ptr& smartfd,
                        int32_t bufsz, uint8_t replication, int64_t blksz) {
  int err = Error::OK;
  create(err, smartfd, bufsz, replication, blksz);
  cb(err, smartfd);
}

void FileSystem::open(const Callback::OpenCb_t& cb, SmartFd::Ptr& smartfd,
                      int32_t bufsz) {
  int err = Error::OK;
  open(err, smartfd, bufsz);
  cb(err, smartfd);
}

size_t FileSystem::default_read(int& err, SmartFd::Ptr& smartfd,
                                StaticBuffer* dst, size_t amount) {
  if(!dst->size)
    dst->reallocate(amount);
  return read(err, smartfd, dst->base, amount);
}

void FileSystem::read(const Callback::ReadCb_t& cb, SmartFd::Ptr& smartfd,
                      size_t amount) {
  int err = Error::OK;
  auto dst = std::make_shared<StaticBuffer>();
  read(err, smartfd, dst.get(), amount);
  cb(err, smartfd, dst);
}

size_t FileSystem::default_pread(int& err, SmartFd::Ptr& smartfd,
                                 uint64_t offset,
                                 StaticBuffer* dst, size_t amount) {
  if(!dst->size)
    dst->reallocate(amount);
  return pread(err, smartfd, offset, dst->base, amount);
}

void FileSystem::pread(const Callback::PreadCb_t& cb, SmartFd::Ptr& smartfd,
                       uint64_t offset, size_t amount) {
  int err = Error::OK;
  auto dst = std::make_shared<StaticBuffer>();
  pread(err, smartfd, offset, dst.get(), amount);
  cb(err, smartfd, dst);
}

void FileSystem::append(const Callback::AppendCb_t& cb, SmartFd::Ptr& smartfd,
                        StaticBuffer& buffer, Flags flags) {
  int err = Error::OK;
  size_t len = append(err, smartfd, buffer, flags);
  cb(err, smartfd, len);
}

void FileSystem::seek(const Callback::CloseCb_t& cb, SmartFd::Ptr& smartfd,
                      size_t offset) {
  int err = Error::OK;
  seek(err, smartfd, offset);
  cb(err, smartfd);
}

void FileSystem::flush(const Callback::FlushCb_t& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  flush(err, smartfd);
  cb(err, smartfd);
}

void FileSystem::sync(const Callback::SyncCb_t& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  sync(err, smartfd);
  cb(err, smartfd);
}

void FileSystem::close(const Callback::CloseCb_t& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  close(err, smartfd);
  cb(err, smartfd);
}


}}