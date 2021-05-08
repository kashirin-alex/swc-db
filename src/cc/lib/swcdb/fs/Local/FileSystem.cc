/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Local/FileSystem.h"
#include <filesystem>
#include <fcntl.h>
#include <dirent.h>


namespace SWC { namespace FS {


Configurables apply_local() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.local.path.root", Config::str(""),
     "Local FileSystem's base root path")
    ("swc.fs.local.cfg.dyn", Config::strs(),
     "Dyn-config file")

    ("swc.fs.local.metrics.enabled", Config::boo(true),
     "Enable or Disable Metrics Tracking")

    ("swc.fs.local.fds.max", Config::g_i32(1024),
      "Max Open Fds for opt. without closing")
  ;
  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.local.cfg", ""),
    "swc.fs.local.cfg.dyn"
  );

  Configurables config;
  config.path_root = Env::Config::settings()->get_str(
    "swc.fs.local.path.root");
  config.cfg_fds_max = Env::Config::settings()
    ->get<Config::Property::V_GINT32>("swc.fs.local.fds.max");
  config.stats_enabled = Env::Config::settings()->get_bool(
    "swc.fs.local.metrics.enabled");
  return config;
}


FileSystemLocal::FileSystemLocal()
                : FileSystem(apply_local()) {
  m_directio = Env::Config::settings()->get_bool(
    "swc.fs.local.DirectIO", false);
}

FileSystemLocal::~FileSystemLocal() { }

Type FileSystemLocal::get_type() const noexcept {
  return Type::LOCAL;
}

std::string FileSystemLocal::to_string() const {
  return format(
    "(type=LOCAL path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}



bool FileSystemLocal::exists(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::EXISTS_SYNC);
  SWC_FS_EXISTS_START(name);
  std::string abspath;
  get_abspath(name, abspath);
  struct stat statbuf;
  errno = 0;
  bool state = !::stat(abspath.c_str(), &statbuf);
  err = errno == ENOENT ? Error::OK : errno;
  SWC_FS_EXISTS_FINISH(err, abspath, state, tracker);
  return state;
}

void FileSystemLocal::remove(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::REMOVE_SYNC);
  SWC_FS_REMOVE_START(name);
  std::string abspath;
  get_abspath(name, abspath);
  errno = 0;
  err = ::unlink(abspath.c_str())==-1 && errno != ENOENT ? errno : Error::OK;
  SWC_FS_REMOVE_FINISH(err, abspath, tracker);
}

size_t FileSystemLocal::length(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::LENGTH_SYNC);
  SWC_FS_LENGTH_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  struct stat statbuf;
  errno = 0;
  size_t len = stat(abspath.c_str(), &statbuf) ? 0 : statbuf.st_size;
  err = errno;
  SWC_FS_LENGTH_FINISH(err, abspath, len, tracker);
  return len;
}


namespace {
SWC_CAN_INLINE
int _mkdirs(std::string& dirname) {
  dirname.append("/");
  struct stat statbuf;
  errno = 0;
  for(auto it = dirname.begin(); ++it != dirname.end(); ) {
    if(*it != '/')
      continue;
    *it = 0;
    if(stat(dirname.c_str(), &statbuf)) {
      if(errno != ENOENT)
        break;
      errno = 0;
      if(mkdir(dirname.c_str(), 0755)) {
        if(errno != EEXIST)
          break;
      }
      errno = 0;
    }
    *it = '/';
  }
  return errno;
}
}

void FileSystemLocal::mkdirs(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::MKDIRS_SYNC);
  SWC_FS_MKDIRS_START(name);
  std::string abspath;
  get_abspath(name, abspath, 1);
  err = _mkdirs(abspath);
  SWC_FS_MKDIRS_FINISH(err, abspath, tracker);
}

void FileSystemLocal::readdir(int& err, const std::string& name,
                              DirentList& results) {
  auto tracker = statistics.tracker(Statistics::READDIR_SYNC);
  SWC_FS_READDIR_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  DIR* dirp;
  std::string full_entry_path;
  struct stat statbuf;

  _do:
  errno = 0;
  dirp = ::opendir(abspath.c_str());
  if(!dirp || errno) {
    err = errno;
    goto _finish;
  }

  for(struct dirent* dep; (dep = ::readdir(dirp)); ) {
    if(!dep->d_name[0] || dep->d_name[0] == '.')
      continue;
    full_entry_path.clear();
    full_entry_path.reserve(abspath.length() + 1 + strlen(dep->d_name));
    full_entry_path.append(abspath);
    full_entry_path.append("/");
    full_entry_path.append(dep->d_name);
    if(::stat(full_entry_path.c_str(), &statbuf) == -1) {
      if(errno == ENOENT) {
        results.clear();
        goto _do; // and do all again directory changed
      }
      err = errno;
      goto _finish;
    }
    results.emplace_back(
      dep->d_name, statbuf.st_mtime, dep->d_type == DT_DIR, statbuf.st_size);
  }
  err = errno;

  _finish:
    if(dirp)
      closedir(dirp);
    SWC_FS_READDIR_FINISH(err, abspath, results.size(), tracker);
}

void FileSystemLocal::rmdir(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::RMDIR_SYNC);
  SWC_FS_RMDIR_START(name);
  std::string abspath;
  get_abspath(name, abspath);
  std::error_code ec;
  std::filesystem::remove_all(abspath, ec);
  err = ec.value();
  SWC_FS_RMDIR_FINISH(err, abspath, tracker);
}

void FileSystemLocal::rename(int& err, const std::string& from,
                              const std::string& to)  {
  auto tracker = statistics.tracker(Statistics::RENAME_SYNC);
  SWC_FS_RENAME_START(from, to);
  std::string abspath_from;
  get_abspath(from, abspath_from);
  std::string abspath_to;
  get_abspath(to, abspath_to);
  std::error_code ec;
  std::filesystem::rename(abspath_from, abspath_to, ec);
  err = ec.value();
  SWC_FS_RENAME_FINISH(err, abspath_from, abspath_to, tracker);
}

void FileSystemLocal::create(int& err, SmartFd::Ptr& smartfd,
                             int32_t bufsz, uint8_t replication,
                             int64_t blksz) {
  auto tracker = statistics.tracker(Statistics::CREATE_SYNC);
  SWC_FS_CREATE_START(smartfd, bufsz, replication, blksz);
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);

  int oflags = O_WRONLY | O_CREAT
    | (smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE ? O_TRUNC : O_APPEND);

  #ifdef O_DIRECT
    if (m_directio && smartfd->flags() & OpenFlags::OPEN_FLAG_DIRECTIO)
      oflags |= O_DIRECT;
  #endif

  /* Open the file */
  int tmperr;
  errno = 0;
  smartfd->fd(::open(abspath.c_str(), oflags, 0644));
  if (!smartfd->valid()) {
    tmperr = errno;
    if(tmperr == EACCES || tmperr == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (tmperr == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    else
      err = tmperr;
  } else {
    err = tmperr = Error::OK;
    fd_open_incr();
    #if defined(__APPLE__)
      #ifdef F_NOCACHE
        fcntl(smartfd->fd(), F_NOCACHE, 1);
      #endif
    #elif defined(__sun__)
      if (m_directio)
        directio(smartfd->fd(), DIRECTIO_ON);
    #endif
  }
  SWC_FS_CREATE_FINISH(tmperr, smartfd, fds_open(), tracker);
}

void FileSystemLocal::open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz) {
  auto tracker = statistics.tracker(Statistics::OPEN_SYNC);
  SWC_FS_OPEN_START(smartfd, bufsz);
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);

  int oflags = O_RDONLY;
  #ifdef O_DIRECT
    if(m_directio && smartfd->flags() & OpenFlags::OPEN_FLAG_DIRECTIO)
      oflags |= O_DIRECT;
  #endif

  /* Open the file */
  int tmperr;
  errno = 0;
  smartfd->fd(::open(abspath.c_str(), oflags));
  if (!smartfd->valid()) {
    tmperr = errno;
    if(tmperr == EACCES || tmperr == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (tmperr == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    else
      err = tmperr;
  } else {
    err = tmperr = Error::OK;
    fd_open_incr();

    #if defined(__sun__)
      if(m_directio)
        directio(smartfd->fd(), DIRECTIO_ON);
    #endif
  }
  SWC_FS_OPEN_FINISH(tmperr, smartfd, fds_open(), tracker);
}


namespace {
SWC_CAN_INLINE
ssize_t _read(int fd, uint8_t* ptr, size_t n) noexcept {
  size_t nleft = n;
  for(ssize_t nread; nleft; nleft -= nread, ptr += nread) {
    if ((nread = ::read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)
        nread = 0;/* and call read() again */
      else if (errno == EAGAIN)
        break;
      else
        return -1;
    } else if (!nread) {
      break; /* EOF */
    }
  }
  return n - nleft;
}
}

size_t FileSystemLocal::read(int& err, SmartFd::Ptr& smartfd,
                             void *dst, size_t amount) {
  auto tracker = statistics.tracker(Statistics::READ_SYNC);
  SWC_FS_READ_START(smartfd, amount);
  size_t ret;
  errno = 0;
  ssize_t nread = _read(smartfd->fd(), static_cast<uint8_t*>(dst), amount);
  if (nread == -1) {
    ret = 0;
    nread = 0;
    err = errno;
  } else {
    err = (ret = nread) == amount ? Error::OK : Error::FS_EOF;
    smartfd->forward(ret);
  }
  SWC_FS_READ_FINISH(err, smartfd, ret, tracker);
  return ret;
}


namespace {
SWC_CAN_INLINE
ssize_t _pread(int fd, off_t offset, uint8_t* ptr, size_t n) noexcept {
  ssize_t nleft = n;
  for(ssize_t nread; nleft; nleft -= nread, ptr += nread, offset += nread) {
    if((nread = ::pread(fd, ptr, nleft, offset)) < 0) {
      if(errno == EINTR)
        nread = 0;/* and call read() again */
      else if(errno == EAGAIN)
        break;
      else
        return -1;
    } else if (!nread) {
      break; /* EOF */
    }
  }
  return n - nleft;
}
}

size_t FileSystemLocal::pread(int& err, SmartFd::Ptr& smartfd,
                              uint64_t offset, void* dst,
                              size_t amount) {
  auto tracker = statistics.tracker(Statistics::PREAD_SYNC);
  SWC_FS_PREAD_START(smartfd, offset, amount);

  size_t ret;
  errno = 0;
  ssize_t nread = _pread(
    smartfd->fd(), off_t(offset), static_cast<uint8_t*>(dst), amount);
  if (nread == -1) {
    ret = 0;
    nread = 0;
    err = errno;
  } else {
    err = (ret = nread) == amount ? Error::OK : Error::FS_EOF;
    smartfd->pos(offset + ret);
  }
  SWC_FS_PREAD_FINISH(err, smartfd, ret, tracker);
  return ret;
}


namespace {
SWC_CAN_INLINE
bool _write(int fd, const uint8_t* ptr, size_t nleft) noexcept {
  for(ssize_t nwritten; nleft; nleft -= nwritten, ptr += nwritten) {
    if((nwritten = ::write(fd, ptr, nleft)) <= 0) {
      if(errno != EINTR)
        return false; /* error */
      nwritten = 0; /* again */
    }
  }
  return true;
}
}

size_t FileSystemLocal::append(int& err, SmartFd::Ptr& smartfd,
                               StaticBuffer& buffer, Flags flags) {
  auto tracker = statistics.tracker(Statistics::APPEND_SYNC);
  SWC_FS_APPEND_START(smartfd, buffer.size, flags);

  errno = 0;
  if(!_write(smartfd->fd(), buffer.base, buffer.size)) {
    err = errno ? errno : ECANCELED;
  } else {
    smartfd->forward(buffer.size);
    if((flags == Flags::FLUSH || flags == Flags::SYNC) &&
       ::fsync(smartfd->fd())) {
      err = errno;
    } else {
      err = Error::OK;
    }
  }
  SWC_FS_APPEND_FINISH(err, smartfd, buffer.size, tracker);
  return buffer.size;
}

void FileSystemLocal::seek(int& err, SmartFd::Ptr& smartfd, size_t offset) {
  auto tracker = statistics.tracker(Statistics::SEEK_SYNC);
  SWC_FS_SEEK_START(smartfd, offset);

  errno = 0;
  uint64_t at = ::lseek(smartfd->fd(), offset, SEEK_SET);
  err = errno;
  if (at == uint64_t(-1) || at != offset || err) {
    if(!err)
      smartfd->pos(at);
  } else {
    smartfd->pos(offset);
  }
  SWC_FS_SEEK_FINISH(err, smartfd, tracker);
}

void FileSystemLocal::flush(int& err, SmartFd::Ptr& smartfd) {
  sync(err, smartfd);
}

void FileSystemLocal::sync(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::SYNC_SYNC);
  SWC_FS_SYNC_START(smartfd);

  err = ::fsync(smartfd->fd()) ? errno : Error::OK;
  SWC_FS_SYNC_FINISH(err, smartfd, tracker);
}

void FileSystemLocal::close(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::CLOSE_SYNC);
  SWC_FS_CLOSE_START(smartfd);

  int32_t fd = smartfd->invalidate();
  if(fd != -1) {
    errno = 0;
    ::close(fd);
    fd_open_decr();
    err = errno;
  } else {
    err = EBADR;
  }
  SWC_FS_CLOSE_FINISH(err, smartfd, tracker);
}



}} // namespace SWC




extern "C" {
SWC::FS::FileSystem* fs_make_new_local(){
  return static_cast<SWC::FS::FileSystem*>(new SWC::FS::FileSystemLocal());
}
void fs_apply_cfg_local(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
}
}
