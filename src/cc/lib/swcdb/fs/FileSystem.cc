/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/FileSystem.h"


namespace SWC { namespace FS {


std::string normalize_pathname(std::string s) {
  if(*(--s.cend()) != '/')
    s.append("/");
  return s;
}

Type fs_type(const std::string& fs_name) {

#if !defined (FS_BROKER_APP)
  if(Condition::str_case_eq(fs_name.data(), "broker", fs_name.size()))
    return Type::BROKER;
#endif

  if(Condition::str_case_eq(fs_name.data(), "local", fs_name.size()))
    return Type::LOCAL;

  if(Condition::str_case_eq(fs_name.data(), "hadoop", fs_name.size()))
    return Type::HADOOP;

  if(Condition::str_case_eq(fs_name.data(), "hadoop_jvm", fs_name.size()))
    return Type::HADOOP_JVM;

  if(Condition::str_case_eq(fs_name.data(), "ceph", fs_name.size()))
    return Type::CEPH;

  if(Condition::str_case_eq(fs_name.data(), "custom", fs_name.size()))
    return Type::CUSTOM;

  else
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Unknown FileSystem name=%s", fs_name.c_str());
  return Type::UNKNOWN;
}

const char* to_string(Type typ) noexcept {
  switch(typ) {
    case Type::LOCAL:
      return "local";
    case Type::BROKER:
      return "broker";
    case Type::HADOOP:
      return "hadoop";
    case Type::HADOOP_JVM:
      return "hadoopJVM";
    case Type::CEPH:
      return "ceph";
    case Type::CUSTOM:
      return "custom";
    default:
      return "unknown";
  }
}


FileSystem::FileSystem(const Configurables* config)
    : path_root(config->path_root.empty()
        ? "" : normalize_pathname(config->path_root)),
      path_data(
        normalize_pathname(config->settings->get_str("swc.fs.path.data"))),
      settings(config->settings),
      cfg_fds_max(config->cfg_fds_max), m_run(true),
      statistics(config->stats_enabled) {
}

FileSystem::~FileSystem() noexcept { }

void FileSystem::stop() {
  m_run.store(false);
  if(statistics.fds_count.load())
    SWC_LOGF(LOG_WARN, "FS %s remained with open-fds=" SWC_FMT_LU,
             to_string().c_str(), statistics.fds_count.load());
}

Type FileSystem::get_type() const noexcept {
  return Type::UNKNOWN;
}

Type FileSystem::get_type_underlying() const noexcept {
  return get_type();
}

std::string FileSystem::to_string() const {
  return format(
    "(type=UNKNOWN path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}

void FileSystem::get_abspath(const std::string& name, std::string& abspath,
                             size_t reserve) {
  abspath.reserve(
    path_root.length() +
    path_data.length() +
    (name.empty() ? -1 : name.length()) +
    reserve
  );
  abspath.append(path_root);
  if(name.empty()) {
    abspath.append(path_data.substr(0, path_data.length() - 1));
  } else {
    abspath.append(path_data);
    abspath.append(name);
  }
}

void FileSystem::fd_open_incr() noexcept {
  statistics.fds_count.fetch_add(1);
}

void FileSystem::fd_open_decr() noexcept {
  statistics.fds_count.fetch_sub(1);
}

bool FileSystem::need_fds() const noexcept {
  return statistics.fds_count >= size_t(cfg_fds_max->get());
}

size_t FileSystem::fds_open() const noexcept {
  return statistics.fds_count.load();
}


/* Default functions of an async call,
 * used if a FileSystem(typ) does not implement
*/

void FileSystem::exists(Callback::ExistsCb_t&& cb,
                        const std::string& name) {
  int err = Error::OK;
  bool state = exists(err, name);
  cb(err, state);
}

void FileSystem::remove(Callback::RemoveCb_t&& cb,
                        const std::string& name) {
  int err = Error::OK;
  remove(err, name);
  cb(err);
}

void FileSystem::length(Callback::LengthCb_t&& cb,
                        const std::string& name) {
  int err = Error::OK;
  size_t len = length(err, name);
  cb(err, len);
}

void FileSystem::mkdirs(Callback::MkdirsCb_t&& cb,
                        const std::string& name) {
  int err = Error::OK;
  mkdirs(err, name);
  cb(err);
}

void FileSystem::readdir(Callback::ReaddirCb_t&& cb,
                         const std::string& name) {
  int err = Error::OK;
  DirentList listing;
  readdir(err, name, listing);
  cb(err, std::move(listing));
}

void FileSystem::rmdir(Callback::RmdirCb_t&& cb,
                       const std::string& name) {
  int err = Error::OK;
  rmdir(err, name);
  cb(err);
}

void FileSystem::rename(Callback::RmdirCb_t&& cb,
                        const std::string& from, const std::string& to) {
  int err = Error::OK;
  rename(err, from, to);
  cb(err);
}

void FileSystem::default_write(int& err, SmartFd::Ptr& smartfd,
                               uint8_t replication, StaticBuffer& buffer) {
  auto tracker = statistics.tracker(Statistics::WRITE_SYNC);
  SWC_FS_WRITE_START(smartfd, replication, buffer.size);

  create(err, smartfd, replication);
  if(!smartfd->valid() || err) {
    if(!err)
      err = EBADF;
    goto finish;
  }

  if(buffer.size)
    append(err, smartfd, buffer, Flags::FLUSH);

  finish:
    int errtmp;
    if(smartfd->valid())
      close(err ? errtmp : err, smartfd);

  if(!err && smartfd->flags() & OpenFlags::WRITE_VALIDATE_LENGTH &&
     length(err, smartfd->filepath()) != buffer.size && !err) {
    err = Error::FS_EOF;
  }

  SWC_FS_WRITE_FINISH(err, smartfd, tracker);
}

void FileSystem::write(Callback::WriteCb_t&& cb, SmartFd::Ptr& smartfd,
                       uint8_t replication, StaticBuffer& buffer) {
  int err = Error::OK;
  write(err, smartfd, replication, buffer);
  cb(err, smartfd);
}

void FileSystem::default_read(int& err, const std::string& name,
                              StaticBuffer* buffer) {
  auto tracker = statistics.tracker(Statistics::READ_ALL_SYNC);
  SWC_FS_READALL_START(name);

  size_t len;
  FS::SmartFd::Ptr smartfd;

  if(!exists(err, name)) {
    if(!err)
      err = Error::FS_PATH_NOT_FOUND;
    len = 0;
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
    if(read(err, smartfd, buffer, len) != len) {
      err = Error::FS_EOF;
      buffer->free();
    }
  }
  if(smartfd->valid()) {
    int errtmp;
    close(err ? errtmp : err, smartfd);
  }

  finish:
  SWC_FS_READALL_FINISH(err, name, len, tracker);
}

void FileSystem::read(Callback::ReadAllCb_t&& cb,
                      const std::string& name) {
  int err = Error::OK;
  StaticBuffer::Ptr dst(new StaticBuffer());
  read(err, name, dst.get());
  cb(err, name, dst);
}

void FileSystem::default_combi_pread(int& err, SmartFd::Ptr& smartfd,
                                     uint64_t offset, uint32_t amount,
                                     StaticBuffer* buffer) {
  auto tracker = statistics.tracker(Statistics::COMBI_PREAD_SYNC);
  SWC_FS_COMBI_PREAD_START(smartfd, offset, amount);

  open(err, smartfd);
  if(!smartfd->valid()) {
    if(!err)
      err = EBADR;
  }
  if(err)
    goto finish;

  buffer->free();
  if(pread(err, smartfd, offset, buffer, amount) != amount) {
    err = Error::FS_EOF;
    buffer->free();
  }
  finish:
    int errtmp;
    if(smartfd->valid())
      close(err ? errtmp : err, smartfd);

  SWC_FS_COMBI_PREAD_FINISH(err, smartfd, amount, tracker);
}

void FileSystem::combi_pread(Callback::CombiPreadCb_t&& cb,
                             SmartFd::Ptr& smartfd,
                             uint64_t offset, uint32_t amount) {
  int err = Error::OK;
  StaticBuffer::Ptr dst(new StaticBuffer());
  combi_pread(err, smartfd, offset, amount, dst.get());
  cb(err, smartfd, dst);
}

void FileSystem::create(Callback::CreateCb_t&& cb, SmartFd::Ptr& smartfd,
                        uint8_t replication) {
  int err = Error::OK;
  create(err, smartfd, replication);
  cb(err, smartfd);
}

void FileSystem::open(Callback::OpenCb_t&& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  open(err, smartfd);
  cb(err, smartfd);
}

size_t FileSystem::default_read(int& err, SmartFd::Ptr& smartfd,
                                StaticBuffer* dst, size_t amount) {
  if(!dst->size)
    dst->reallocate(amount);
  return read(err, smartfd, dst->base, amount);
}

void FileSystem::read(Callback::ReadCb_t&& cb, SmartFd::Ptr& smartfd,
                      size_t amount) {
  int err = Error::OK;
  StaticBuffer::Ptr dst(new StaticBuffer());
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

void FileSystem::pread(Callback::PreadCb_t&& cb, SmartFd::Ptr& smartfd,
                       uint64_t offset, size_t amount) {
  int err = Error::OK;
  StaticBuffer::Ptr dst(new StaticBuffer());
  pread(err, smartfd, offset, dst.get(), amount);
  cb(err, smartfd, dst);
}

void FileSystem::append(Callback::AppendCb_t&& cb, SmartFd::Ptr& smartfd,
                        StaticBuffer& buffer, Flags flags) {
  int err = Error::OK;
  size_t len = append(err, smartfd, buffer, flags);
  cb(err, smartfd, len);
}

void FileSystem::seek(Callback::CloseCb_t&& cb, SmartFd::Ptr& smartfd,
                      size_t offset) {
  int err = Error::OK;
  seek(err, smartfd, offset);
  cb(err, smartfd);
}

void FileSystem::flush(Callback::FlushCb_t&& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  flush(err, smartfd);
  cb(err, smartfd);
}

void FileSystem::sync(Callback::SyncCb_t&& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  sync(err, smartfd);
  cb(err, smartfd);
}

void FileSystem::close(Callback::CloseCb_t&& cb, SmartFd::Ptr& smartfd) {
  int err = Error::OK;
  close(err, smartfd);
  cb(err, smartfd);
}


}}