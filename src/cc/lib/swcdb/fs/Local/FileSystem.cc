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
  std::string abspath;
  get_abspath(name, abspath);
  struct stat statbuf;
  errno = 0;
  bool state = !::stat(abspath.c_str(), &statbuf);
  err = errno == ENOENT ? Error::OK : errno;
  SWC_LOGF(LOG_DEBUG, "exists state='%d' path='%s'", state, abspath.c_str());
  return state;
}

void FileSystemLocal::remove(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  errno = 0;
  if(::unlink(abspath.c_str()) == -1) {
    if(errno != ENOENT) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "remove('%s') failed - %d(%s)",
                abspath.c_str(), errno, Error::get_text(errno));
      return;
    }
  }
  SWC_LOGF(LOG_DEBUG, "remove('%s')", abspath.c_str());
}

size_t FileSystemLocal::length(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  errno = 0;

  size_t len = 0;
  struct stat statbuf;
  if(stat(abspath.c_str(), &statbuf)) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "length('%s') failed - %d(%s)",
              abspath.c_str(), errno, Error::get_text(errno));
    return len;
  }
  len = statbuf.st_size;
  SWC_LOGF(LOG_DEBUG, "length len='%lu' path='%s'", len, abspath.c_str());
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
  std::string abspath;
  get_abspath(name, abspath, 1);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", abspath.c_str());

  if((err = _mkdirs(abspath)))
    SWC_LOGF(LOG_ERROR, "mkdirs failed: %d(%s) at path='%s'",
              err, Error::get_text(err), abspath.c_str());

}

void FileSystemLocal::readdir(int& err, const std::string& name,
                              DirentList& results) {
  std::string abspath;
  get_abspath(name, abspath);
  SWC_LOGF(LOG_DEBUG, "Readdir dir='%s'", abspath.c_str());

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
  if(err)
    SWC_LOGF(LOG_ERROR, "Readdir failed: %d(%s), %s",
              err, Error::get_text(err), abspath.c_str());
  if(dirp)
    closedir(dirp);
}

void FileSystemLocal::rmdir(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  std::error_code ec;
  std::filesystem::remove_all(abspath, ec);
  if (ec) {
    err = ec.value();
    SWC_LOGF(LOG_ERROR, "rmdir('%s') failed - %s",
              abspath.c_str(), Error::get_text(errno));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rmdir('%s')", abspath.c_str());
}

void FileSystemLocal::rename(int& err, const std::string& from,
                              const std::string& to)  {
  std::string abspath_from;
  get_abspath(from, abspath_from);
  std::string abspath_to;
  get_abspath(to, abspath_to);
  std::error_code ec;
  std::filesystem::rename(abspath_from, abspath_to, ec);
  if (ec) {
    err = ec.value();
    SWC_LOGF(LOG_ERROR, "rename('%s' to '%s') failed - %s",
              abspath_from.c_str(), abspath_to.c_str(), Error::get_text(errno));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rename('%s' to '%s')",
            abspath_from.c_str(), abspath_to.c_str());
}

void FileSystemLocal::create(int& err, SmartFd::Ptr& smartfd,
                             int32_t bufsz, uint8_t replication,
                             int64_t blksz) {
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);
  SWC_LOGF(LOG_DEBUG, "create %s bufsz=%d replication=%d blksz=%ld",
            smartfd->to_string().c_str(), bufsz, replication, blksz);

  int oflags = O_WRONLY | O_CREAT
    | (smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE ? O_TRUNC : O_APPEND);

#ifdef O_DIRECT
  if (m_directio && smartfd->flags() & OpenFlags::OPEN_FLAG_DIRECTIO)
    oflags |= O_DIRECT;
#endif

  /* Open the file */
  errno = 0;
  smartfd->fd(::open(abspath.c_str(), oflags, 0644));
  if (!smartfd->valid()) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "create failed: %d(%s), %s",
              errno, Error::get_text(errno), smartfd->to_string().c_str());

    if(err == EACCES || err == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    return;
  }
  fd_open_incr();
  SWC_LOGF(LOG_DEBUG, "created %s bufsz=%d replication=%d blksz=%ld",
            smartfd->to_string().c_str(), bufsz, replication, blksz);

#if defined(__APPLE__)
#ifdef F_NOCACHE
  fcntl(smartfd->fd(), F_NOCACHE, 1);
#endif
#elif defined(__sun__)
  if (m_directio)
    directio(smartfd->fd(), DIRECTIO_ON);
#endif

}

void FileSystemLocal::open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz) {
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);
  SWC_LOGF(LOG_DEBUG, "open %s, bufsz=%d",
            smartfd->to_string().c_str(), bufsz);

  int oflags = O_RDONLY;

#ifdef O_DIRECT
  if(m_directio && smartfd->flags() & OpenFlags::OPEN_FLAG_DIRECTIO)
    oflags |= O_DIRECT;
#endif

  /* Open the file */
  errno = 0;
  smartfd->fd(::open(abspath.c_str(), oflags));
  if (!smartfd->valid()) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "open failed: %d(%s), %s",
              errno, Error::get_text(errno), smartfd->to_string().c_str());

    if(err == EACCES || err == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    return;
  }
  fd_open_incr();
  SWC_LOGF(LOG_DEBUG, "opened %s", smartfd->to_string().c_str());

#if defined(__sun__)
  if(m_directio)
    directio(smartfd->fd(), DIRECTIO_ON);
#endif

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
  SWC_LOGF(LOG_DEBUG, "read %s amount=%lu",
            smartfd->to_string().c_str(), amount);
  /*
  uint64_t offset;
  if ((offset = (uint64_t)lseek(smartfd->fd(), 0, SEEK_CUR)) == (uint64_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "read, lseek failed: %d(%s), %s offset=%lu",
              errno, Error::get_text(errno), smartfd->to_string().c_str(), offset);
    return nread;
  }
  */

  size_t ret;
  errno = 0;
  ssize_t nread = _read(smartfd->fd(), static_cast<uint8_t*>(dst), amount);
  if (nread == -1) {
    ret = 0;
    nread = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "read failed: %d(%s), %s",
              errno, Error::get_text(errno), smartfd->to_string().c_str());
  } else {
    if((ret = nread) != amount)
      err = Error::FS_EOF;
    smartfd->forward(ret);
    SWC_LOGF(LOG_DEBUG, "read(ed) %s amount=%lu eof=%d",
              smartfd->to_string().c_str(), ret, err == Error::FS_EOF);
  }
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
  SWC_LOGF(LOG_DEBUG, "pread %s offset=%lu amount=%lu",
            smartfd->to_string().c_str(), offset, amount);

  size_t ret;
  errno = 0;
  ssize_t nread = _pread(
    smartfd->fd(), off_t(offset), static_cast<uint8_t*>(dst), amount);
  if (nread == -1) {
    ret = 0;
    nread = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "pread failed: %d(%s), %s",
              errno, Error::get_text(errno), smartfd->to_string().c_str());
  } else {
    if((ret = nread) != amount)
      err = Error::FS_EOF;
    smartfd->pos(offset + ret);
    SWC_LOGF(LOG_DEBUG, "pread(ed) %s amount=%lu  eof=%d",
              smartfd->to_string().c_str(), ret, err == Error::FS_EOF);
  }
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
  SWC_LOGF(LOG_DEBUG, "append %s amount=%lu flags=%d",
            smartfd->to_string().c_str(), buffer.size, flags);

  /*
  if(smartfd->pos()
    && lseek(smartfd->fd(), 0, SEEK_CUR) == (uint64_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "append, lseek failed: %d(%s), %s",
              errno, Error::get_text(errno), smartfd->to_string().c_str());
    return 0;
  }
  */

  errno = 0;
  if(!_write(smartfd->fd(), buffer.base, buffer.size)) {
    err = errno ? errno : ECANCELED;
    SWC_LOGF(LOG_ERROR, "write failed: %d(%s), %s",
              err, Error::get_text(err), smartfd->to_string().c_str());
    return 0;
  }
  smartfd->forward(buffer.size);

  if(flags == Flags::FLUSH || flags == Flags::SYNC) {
    if(fsync(smartfd->fd())) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "write, fsync failed: %d(%s), %s",
                errno, Error::get_text(errno), smartfd->to_string().c_str());
    }
  }

  SWC_LOGF(LOG_DEBUG, "appended %s written=%lu",
            smartfd->to_string().c_str(), buffer.size);
  return buffer.size;
}

void FileSystemLocal::seek(int& err, SmartFd::Ptr& smartfd, size_t offset) {
  SWC_LOGF(LOG_DEBUG, "seek %s offset=%lu",
            smartfd->to_string().c_str(), offset);

  errno = 0;
  uint64_t at = lseek(smartfd->fd(), offset, SEEK_SET);
  if (at == uint64_t(-1) || at != offset || errno) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "seek failed - %d(%s) %s",
              err, Error::get_text(errno), smartfd->to_string().c_str());
    if(!errno)
      smartfd->pos(at);
    return;
  }
  smartfd->pos(offset);
}

void FileSystemLocal::flush(int& err, SmartFd::Ptr& smartfd) {
  sync(err, smartfd);
}

void FileSystemLocal::sync(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOGF(LOG_DEBUG, "sync %s", smartfd->to_string().c_str());

  errno = 0;
  if(fsync(smartfd->fd()) != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "sync failed - %d(%s) %s",
              err, Error::get_text(errno), smartfd->to_string().c_str());
  }
}

void FileSystemLocal::close(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOGF(LOG_DEBUG, "close %s", smartfd->to_string().c_str());
  int32_t fd = smartfd->invalidate();
  if(fd != -1) {
    errno = 0;
    ::close(fd);
    fd_open_decr();
    err = errno;
  } else {
    err = EBADR;
  }
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
