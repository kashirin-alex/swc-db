/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/fs/Local/FileSystem.h"

#include <iostream>
#include <filesystem>

#include "swcdb/core/FileUtils.h"

#include <fcntl.h>

namespace SWC{ namespace FS {


bool apply_local() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.local.path.root", str(""), "Local FileSystem's base root path")
    ("swc.fs.local.OnFileChange.cfg", str(), "Dyn-config file")
  ;
  Env::Config::settings()->parse_file(
    Env::Config::settings()->get<std::string>(
      "swc.fs.local.cfg", ""),
    Env::Config::settings()->get<std::string>(
      "swc.fs.local.OnFileChange.cfg", "")
  );
  return true;
}


FileSystemLocal::FileSystemLocal()
    : FileSystem(
        Env::Config::settings()->get<std::string>("swc.fs.local.path.root"),
        apply_local()
      ),
      m_directio(Env::Config::settings()->get<bool>(
        "swc.fs.local.DirectIO", false)) { 
}

FileSystemLocal::~FileSystemLocal() { }

Types::Fs FileSystemLocal::get_type() {
  return Types::Fs::LOCAL;
};

const std::string FileSystemLocal::to_string() {
  return format(
    "(type=LOCAL path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}



bool FileSystemLocal::exists(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  bool state = FileUtils::exists(abspath);
  err = errno==2 ? 0 : errno;
  SWC_LOGF(LOG_DEBUG, "exists state='%d' path='%s'", 
            (int)state, abspath.c_str());
  return state;
}

void FileSystemLocal::remove(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  if(!FileUtils::unlink(abspath)) {
    if(errno != 2) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "remove('%s') failed - %d(%s)", 
                abspath.c_str(), errno, strerror(errno));
      return;
    }
  }
  SWC_LOGF(LOG_DEBUG, "remove('%s')", abspath.c_str());
}
  
size_t FileSystemLocal::length(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
    
  size_t len = 0; 
  if ((len = FileUtils::length(abspath)) == (size_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "length('%s') failed - %d(%s)", 
              abspath.c_str(), errno, strerror(errno));
    len = 0;
    return len;
  }
  SWC_LOGF(LOG_DEBUG, "length len='%lld' path='%s'", len, abspath.c_str());
  return len;
}

void FileSystemLocal::mkdirs(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", abspath.c_str());
  
  errno = 0;
  FileUtils::mkdirs(abspath);
  err = errno;
}

void FileSystemLocal::readdir(int &err, const std::string &name, 
                              DirentList &results) {
  std::string abspath = get_abspath(name);
  SWC_LOGF(LOG_DEBUG, "Readdir dir='%s'", abspath.c_str());

  std::vector<struct dirent> listing;
  errno = 0;
  FileUtils::readdir(abspath, "", listing);
  if (errno) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "FileUtils::readdir('%s') failed - %s", 
              abspath.c_str(), strerror(errno));
    return;
  }
    
  std::string full_entry_path;
  struct stat statbuf;

  for(auto& result : listing){
    if (result.d_name[0] == '.' || !result.d_name[0])
      continue;

    auto& entry = results.emplace_back();
    entry.name = result.d_name;
    entry.is_dir = result.d_type == DT_DIR;

    full_entry_path.clear();
    full_entry_path.append(abspath);
    full_entry_path.append("/");
    full_entry_path.append(entry.name);
    if (stat(full_entry_path.c_str(), &statbuf) == -1) {
      if(errno == 2) { 
        // and do all again directory changed
        results.clear();
        readdir(err, name, results);
        return;
      }
      err = errno;
      SWC_LOGF(LOG_ERROR, "readdir('%s') stat failed - %d(%s)", 
                full_entry_path.c_str(), errno, strerror(errno));
      return;
    }
    entry.length = (uint64_t)statbuf.st_size;
    entry.last_modification_time = statbuf.st_mtime;
  }
}

void FileSystemLocal::rmdir(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  std::error_code ec;
  std::filesystem::remove_all(abspath, ec);
  if (ec) {
    err = ec.value();
    SWC_LOGF(LOG_ERROR, "rmdir('%s') failed - %s", 
              abspath.c_str(), strerror(errno));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rmdir('%s')", abspath.c_str());
}

void FileSystemLocal::rename(int &err, const std::string &from, 
                              const std::string &to)  {
  std::string abspath_from = get_abspath(from);
  std::string abspath_to = get_abspath(to);
  std::error_code ec;
  std::filesystem::rename(abspath_from, abspath_to, ec);
  if (ec) {
    err = ec.value();
    SWC_LOGF(LOG_ERROR, "rename('%s' to '%s') failed - %s", 
              abspath_from.c_str(), abspath_to.c_str(), strerror(errno));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rename('%s' to '%s')", 
            abspath_from.c_str(), abspath_to.c_str());
}

void FileSystemLocal::create(int &err, SmartFd::Ptr &smartfd, 
                             int32_t bufsz, uint8_t replication, 
                             int64_t blksz) {
  std::string abspath = get_abspath(smartfd->filepath());
  SWC_LOGF(LOG_DEBUG, "create %s bufsz=%d replication=%d blksz=%lld",
            smartfd->to_string().c_str(), 
            bufsz, replication, (Lld)blksz);

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
              errno, strerror(errno), smartfd->to_string().c_str());

    if(err == EACCES || err == ENOENT)
      err == Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err == Error::FS_PERMISSION_DENIED;
    return;
  }
    
  SWC_LOGF(LOG_DEBUG, "created %s bufsz=%d replication=%d blksz=%lld",
            smartfd->to_string().c_str(), 
            bufsz, replication, (Lld)blksz);

#if defined(__APPLE__)
#ifdef F_NOCACHE
  fcntl(smartfd->fd(), F_NOCACHE, 1);
#endif  
#elif defined(__sun__)
  if (m_directio)
    directio(smartfd->fd(), DIRECTIO_ON);
#endif

}

void FileSystemLocal::open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz) {
  std::string abspath = get_abspath(smartfd->filepath());
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
              errno, strerror(errno), smartfd->to_string().c_str());
                
    if(err == EACCES || err == ENOENT)
      err == Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err == Error::FS_PERMISSION_DENIED;
    return;
  }
    
  SWC_LOGF(LOG_DEBUG, "opened %s", smartfd->to_string().c_str());

#if defined(__sun__)
  if(m_directio)
    directio(smartfd->fd(), DIRECTIO_ON);
#endif

}
  
size_t FileSystemLocal::read(int &err, SmartFd::Ptr &smartfd, 
                             void *dst, size_t amount) {
  SWC_LOGF(LOG_DEBUG, "read %s amount=%lld", 
            smartfd->to_string().c_str(), amount);
  ssize_t nread = 0;
  errno = 0;
    
  /*
  uint64_t offset;
  if ((offset = (uint64_t)lseek(smartfd->fd(), 0, SEEK_CUR)) == (uint64_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "read, lseek failed: %d(%s), %s offset=%llu", 
              errno, strerror(errno), smartfd->to_string().c_str(), offset);
    return nread;
  }
  */
    
  nread = FileUtils::read(smartfd->fd(), dst, amount);
  if (nread == -1) {
    nread = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "read failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  } else {
    if(nread != amount)
      err = Error::FS_EOF;
    smartfd->pos(smartfd->pos()+nread);
    SWC_LOGF(LOG_DEBUG, "read(ed) %s amount=%llu eof=%d", 
              smartfd->to_string().c_str(), nread, err == Error::FS_EOF);
  }
  return nread;
}

size_t FileSystemLocal::pread(int &err, SmartFd::Ptr &smartfd, 
                              uint64_t offset, void *dst, 
                              size_t amount) {
  SWC_LOGF(LOG_DEBUG, "pread %s offset=%llu amount=%lld", 
            smartfd->to_string().c_str(), offset, amount);

  errno = 0;
  ssize_t nread = FileUtils::pread(smartfd->fd(), (off_t)offset, dst, amount);
  if (nread == -1) {
    nread = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "pread failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  } else {
    if(nread != amount)
      err = Error::FS_EOF;
    smartfd->pos(offset+nread);
    SWC_LOGF(LOG_DEBUG, "pread(ed) %s amount=%llu  eof=%d", 
              smartfd->to_string().c_str(), nread, err == Error::FS_EOF);
  }
  return nread;
}

size_t FileSystemLocal::append(int &err, SmartFd::Ptr &smartfd, 
                               StaticBuffer &buffer, Flags flags) {
  SWC_LOGF(LOG_DEBUG, "append %s amount=%lld flags=%d", 
            smartfd->to_string().c_str(), buffer.size, flags);
    
  ssize_t nwritten = 0;
  errno = 0;

  /* 
  if (smartfd->pos() != 0 
    && lseek(smartfd->fd(), 0, SEEK_CUR) == (uint64_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "append, lseek failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
    return nwritten;
  }
  */

  if ((nwritten = FileUtils::write(
                  smartfd->fd(), buffer.base, buffer.size)) == -1) {
    nwritten = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "write failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
    return nwritten;
  }
  smartfd->pos(smartfd->pos()+nwritten);
    
  if (flags == Flags::FLUSH || flags == Flags::SYNC) {
    if (fsync(smartfd->fd()) != 0) {     
      err = errno;
      SWC_LOGF(LOG_ERROR, "write, fsync failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
    }
  }
    
  SWC_LOGF(LOG_DEBUG, "appended %s written=%llu", 
            smartfd->to_string().c_str(), nwritten);
  return nwritten;
}

void FileSystemLocal::seek(int &err, SmartFd::Ptr &smartfd, size_t offset) {
  SWC_LOGF(LOG_DEBUG, "seek %s offset=%llu", 
            smartfd->to_string().c_str(), offset);
    
  errno = 0;
  uint64_t at = lseek(smartfd->fd(), offset, SEEK_SET); 
  if (at == (uint64_t)-1 || at != offset || errno != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "seek failed - %d(%s) %s", 
              err, strerror(errno), smartfd->to_string().c_str());
    if(at > 0)
      smartfd->pos(at);
    return;
  }
  smartfd->pos(offset);
}

void FileSystemLocal::flush(int &err, SmartFd::Ptr &smartfd) {
  sync(err, smartfd);
}

void FileSystemLocal::sync(int &err, SmartFd::Ptr &smartfd) {
  SWC_LOGF(LOG_DEBUG, "sync %s", smartfd->to_string().c_str());
    
  errno = 0;
  if(fsync(smartfd->fd()) != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "sync failed - %d(%s) %s", 
              err, strerror(errno), smartfd->to_string().c_str());
  }
}

void FileSystemLocal::close(int &err, SmartFd::Ptr &smartfd) {
  SWC_LOGF(LOG_DEBUG, "close %s", smartfd->to_string().c_str());
  errno = 0;
  if(smartfd->valid()) { 
    ::close(smartfd->fd());
    err = errno;
  } else 
    err = EBADR;
    
  smartfd->fd(-1);
  smartfd->pos(0);
}



}} // namespace SWC




extern "C" {
SWC::FS::FileSystem* fs_make_new_local(){
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemLocal());
};
void fs_apply_cfg_local(SWC::Env::Config::Ptr env){
  SWC::Env::Config::set(env);
};
}
