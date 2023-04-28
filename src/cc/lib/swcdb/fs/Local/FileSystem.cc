/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Local/FileSystem.h"
#include <filesystem>
#include <fcntl.h>
#include <dirent.h>

#if defined(SWC_FS_LOCAL_USE_IO_URING)
#include <asio/stream_file.hpp>
#endif


#if defined(__MINGW64__) || defined(_WIN32)
  #include <Windows.h>
  #include <io.h>
  #include <fileapi.h>

  namespace {
  SWC_CAN_INLINE
  ssize_t __pread(int fd, uint8_t* ptr, size_t n, off_t offset) noexcept {
    long unsigned int read_bytes = 0;
    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(OVERLAPPED));
    overlapped.OffsetHigh = offset >> 32;
    overlapped.Offset = ((offset << 32) >> 32);
    SetLastError(0);
    if(!ReadFile(HANDLE(_get_osfhandle(fd)), ptr, n, &read_bytes, &overlapped)) {
      errno = GetLastError();
      return -1;
    }
    return read_bytes;
  }
  }
  #define SWC_MKDIR(path, perms)            ::mkdir(path)
  #define SWC_FSYNC(fd)                     ::_commit(fd)
  #define SWC_PREAD(fd, ptr, nleft, offset) __pread(fd, ptr, nleft, offset)

#else
  #define SWC_MKDIR(path, perms)            ::mkdir(path, perms)
  #define SWC_FSYNC(fd)                     ::fsync(fd)
  #define SWC_PREAD(fd, ptr, nleft, offset) ::pread(fd, ptr, nleft, offset)
#endif



namespace SWC { namespace FS {


Configurables* apply_local(Configurables* config) {
  config->settings->file_desc.add_options()
    ("swc.fs.local.path.root", Config::str(""),
     "Local FileSystem's base root path")
    ("swc.fs.local.cfg.dyn", Config::strs(),
     "Dyn-config file")

    ("swc.fs.local.metrics.enabled", Config::boo(true),
     "Enable or Disable Metrics Tracking")

    #if defined(SWC_FS_LOCAL_USE_IO_URING)
    ("swc.fs.local.handlers", Config::i32(12),
     "Handlers for async filesystem")
    #endif

    ("swc.fs.local.fds.max", Config::g_i32(512),
      "Max Open Fds for opt. without closing")
  ;
  config->settings->parse_file(
    config->settings->get_str("swc.fs.local.cfg", ""),
    "swc.fs.local.cfg.dyn"
  );

  config->path_root = config->settings->get_str(
    "swc.fs.local.path.root");
  config->cfg_fds_max = config->settings
    ->get<Config::Property::Value_int32_g>("swc.fs.local.fds.max");
  config->stats_enabled = config->settings->get_bool(
    "swc.fs.local.metrics.enabled");
  return config;
}


FileSystemLocal::FileSystemLocal(Configurables* config)
    : FileSystem(
        apply_local(config),
        ImplOptions()
        #if defined(SWC_FS_LOCAL_USE_IO_URING)
        .add_async_readall()
        #endif
      ),
      m_directio(settings->get_bool(
        "swc.fs.local.DirectIO", false))
      #if defined(SWC_FS_LOCAL_USE_IO_URING)
      ,
      m_io(new Comm::IoContext(
        "FsLocal", settings->get_i32("swc.fs.local.handlers")))
      #endif
      {
}

FileSystemLocal::~FileSystemLocal() noexcept { }

#if defined(SWC_FS_LOCAL_USE_IO_URING)
void FileSystemLocal::stop() {
  m_io->stop();
  FileSystem::stop();
}
#endif

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
  for(auto it = dirname.begin(); ++it != dirname.cend(); ) {
    if(*it != '/')
      continue;
    *it = 0;
    if(stat(dirname.c_str(), &statbuf)) {
      if(errno != ENOENT)
        break;
      errno = 0;
      if(SWC_MKDIR(dirname.c_str(), 0755)) {
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

  #if defined(__MINGW64__) || defined(_WIN32)
  const std::filesystem::path _dir_path(std::move(abspath));
  std::error_code ec;
  _do:
  for(auto const& de : std::filesystem::directory_iterator(_dir_path, ec)) {
    bool is_dir = de.is_directory(ec);
    size_t sz;
    if(!ec) {
      sz = is_dir ? 0 : de.file_size(ec);
      // auto t = de.last_write_time(ec); ? no-reason
    }
    if(ec) {
      if(ec.value() == ENOENT) {
        results.clear();
        ec.clear();
        goto _do; // and do all again directory changed
      }
      break;
    }
    results.emplace_back(
      de.path().filename().string().c_str(), 0, is_dir, sz);
  }
  err = ec.value();

  #else
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
  #endif

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

#if defined(SWC_FS_LOCAL_USE_IO_URING)
/*
void FileSystemLocal::read(Callback::ReadAllCb_t&& cb,
                           const std::string& name) {
  SWC_FS_READALL_START(name);

  struct HandlerReadAll {
    FS::SmartFd::Ptr                smartfd;
    Callback::ReadAllCb_t           cb;
    StaticBuffer::Ptr               buffer;
    asio::stream_file               sf;
    size_t                          recved;
    FS::Statistics::Metric::Tracker tracker;

    HandlerReadAll(Callback::ReadAllCb_t&& a_cb,
                   const Comm::IoContextPtr& io, FS::Statistics& stats)
                  : cb(std::move(a_cb)), sf(io->executor()), recved(0),
                    tracker(stats.tracker(Statistics::READ_ALL_SYNC)) {
    }
    void run(size_t sz) {
      sf.assign(smartfd->fd());
      buffer.reset(new StaticBuffer(sz));
      operator()(asio::error_code(), 0);
    }
    void operator()(const asio::error_code& ec, size_t bytes) {
      SWC_PRINT << "HandlerReadAll() ec=" << ec << " is_open=" << sf.is_open() << " bytes=" << bytes << " left=" << (buffer->size - bytes) << SWC_PRINT_CLOSE;
      if(!ec && (recved += bytes) < buffer->size) {
        sf.async_read_some(
          asio::buffer(buffer->base + bytes, buffer->size - bytes),
          std::move(*this)
        );
      } else {
        callback(ec.value());
      }
    }
    void callback(int err) {
      SWC_PRINT << "HandlerReadAll callback() err=" << err << " recved=" << recved << SWC_PRINT_CLOSE;
      if(sf.is_open()) {
        asio::error_code tmp;
        sf.close(tmp);
      }
      SWC_FS_READALL_FINISH(err, smartfd->filepath(), recved, tracker);
      cb(
        err
          ? (err == ENOENT ? Error::FS_PATH_NOT_FOUND : err)
          : (recved != buffer->size ? Error::FS_EOF : err),
        buffer
      );
    }
  };

  HandlerReadAll hdlr(std::move(cb), m_io, statistics);

  int err = Error::OK;
  size_t len;
  if(!exists(err, name)) {
    if(!err)
      err = Error::FS_PATH_NOT_FOUND;
    len = 0;
    goto finish;
  }
  len = length(err, name);
  if(err)
    goto finish;

  hdlr.smartfd = FS::SmartFd::make_ptr(name, 0);

  open(err, hdlr.smartfd);
  if(!err && !hdlr.smartfd->valid())
    err = EBADR;
  finish:
  err ? hdlr.callback(err) : hdlr.run(len);
}
*/

void FileSystemLocal::read(Callback::ReadAllCb_t&& cb,
                           const std::string& name) {
  SWC_FS_READALL_START(name);

  struct HandlerReadAll {
    std::string                     name;
    Callback::ReadAllCb_t           cb;
    StaticBuffer::Ptr               buffer;
    asio::stream_file               sf;
    size_t                          recved;
    FS::Statistics::Metric::Tracker tracker;

    HandlerReadAll(FileSystemLocal* fs,
                   const std::string& a_name, Callback::ReadAllCb_t&& a_cb,
                   const Comm::IoContextPtr& io, FS::Statistics& stats)
                  : name(a_name), cb(std::move(a_cb)),
                    sf(io->executor()), recved(0),
                    tracker(stats.tracker(Statistics::READ_ALL_SYNC)) {
      std::string abspath;
      fs->get_abspath(name, abspath);

      asio::error_code ec;
      sf.open(abspath, asio::stream_file::read_only, ec);
      SWC_PRINT << "HandlerReadAll() open ec=" << ec << SWC_PRINT_CLOSE;
      if(!ec) {
        size_t sz = sf.size(ec);
        //if(!ec)
        //  sf.seek(0, asio::stream_file::seek_set);
        SWC_PRINT << "HandlerReadAll() ec=" << ec << " sz=" << sz << SWC_PRINT_CLOSE;
        if(!ec) {
          buffer.reset(new StaticBuffer(sz));
          operator()(ec, 0);
          return;
        }
      }
      callback(ec.value());
    }
    void operator()(const asio::error_code& ec, size_t bytes) {
      SWC_PRINT << "HandlerReadAll() ec=" << ec << " is_open=" << sf.is_open() << " bytes=" << bytes << " left=" << (buffer->size - bytes) << SWC_PRINT_CLOSE;
      if(!ec && (recved += bytes) < buffer->size) {
        sf.async_read_some(
          asio::buffer(buffer->base + bytes, buffer->size - bytes),
          std::move(*this)
        );
      } else {
        callback(ec.value());
      }
    }
    void callback(int err) {
      SWC_PRINT << "HandlerReadAll callback() err=" << err << " recved=" << recved << SWC_PRINT_CLOSE;
      if(sf.is_open()) {
        asio::error_code tmp;
        sf.close(tmp);
      }
      SWC_FS_READALL_FINISH(err, name, recved, tracker);
      cb(
        err
          ? (err == ENOENT ? Error::FS_PATH_NOT_FOUND : err)
          : (recved != buffer->size ? Error::FS_EOF : err),
        std::move(*buffer.get())
      );
    }
  };

  SWC_PRINT << "HandlerReadAll 1 " << SWC_PRINT_CLOSE;
  HandlerReadAll(this, name, std::move(cb), m_io, statistics);
  SWC_PRINT << "HandlerReadAll 2 " << SWC_PRINT_CLOSE;
}
#endif

void FileSystemLocal::create(int& err, SmartFd::Ptr& smartfd,
                             uint8_t replication) {
  auto tracker = statistics.tracker(Statistics::CREATE_SYNC);
  SWC_FS_CREATE_START(smartfd, replication);
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

void FileSystemLocal::open(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::OPEN_SYNC);
  SWC_FS_OPEN_START(smartfd);
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
    if((nread = SWC_PREAD(fd, ptr, nleft, offset)) < 0) {
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
       SWC_FSYNC(smartfd->fd())) {
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

  err = SWC_FSYNC(smartfd->fd()) ? errno : Error::OK;
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



#undef SWC_MKDIR
#undef SWC_FSYNC
#undef SWC_PREAD


extern "C" {
SWC::FS::FileSystem* fs_make_new_local(SWC::FS::Configurables* config) {
  return static_cast<SWC::FS::FileSystem*>(
    new SWC::FS::FileSystemLocal(config));
}
}
