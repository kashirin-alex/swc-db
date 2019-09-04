/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Local_FileSystem_h
#define swc_lib_fs_Local_FileSystem_h

#include <iostream>
#include <filesystem>
#include "swcdb/lib/fs/FileSystem.h"
#include "swcdb/lib/core/FileUtils.h"

namespace SWC{ namespace FS {

bool apply_local();


class FileSystemLocal: public FileSystem {
  public:

  FileSystemLocal()
    : FileSystem(
        Env::Config::settings()->get<String>("swc.fs.local.path.root"),
        apply_local()
      ),
      m_directio(Env::Config::settings()->get<bool>(
        "swc.fs.local.DirectIO", false)) { }

  virtual ~FileSystemLocal(){}

  Types::Fs get_type() override;

  const std::string to_string() override {
    return format(
      "(type=LOCAL, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }




  bool exists(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    bool state = FileUtils::exists(abspath);
    err = errno==2?0:errno;
    HT_DEBUGF("exists state='%d' path='%s'", (int)state, abspath.c_str());
    return state;
  }

  void remove(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    if(!FileUtils::unlink(abspath)) {
      err = errno;
      if(err != 2) {
        HT_ERRORF("remove('%s') failed - %d(%s)", 
                  abspath.c_str(), errno, strerror(errno));
        return;
      }
    }
    HT_DEBUGF("remove('%s')", abspath.c_str());
  }
  
  size_t length(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    
    size_t len = 0; 
    if ((len = FileUtils::length(abspath)) == (size_t)-1) {
      err = errno;
      HT_ERRORF("length('%s') failed - %d(%s)", 
                abspath.c_str(), errno, strerror(errno));
      len = 0;
      return len;
    }
    HT_DEBUGF("length len='%d' path='%s'", len, abspath.c_str());
    return len;
  }

  void mkdirs(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("mkdirs path='%s'", abspath.c_str());
    
    errno = 0;
    FileUtils::mkdirs(abspath);
    err = errno;
  }

  void readdir(int &err, const String &name, DirentList &results) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("Readdir dir='%s'", abspath.c_str());

    std::vector<struct dirent> listing;
    errno = 0;
    FileUtils::readdir(abspath, "", listing);
    if (errno > 0) {
      err = errno;
      HT_ERRORF("FileUtils::readdir('%s') failed - %s", 
                abspath.c_str(), strerror(errno));
      return;
    }
    
    Dirent entry;
    String full_entry_path;
    struct stat statbuf;

    for(auto& result : listing){
      if (result.d_name[0] == '.' || result.d_name[0] == 0)
        continue;

      entry.name.clear();
      entry.name.append(result.d_name);
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
        HT_ERRORF("readdir('%s') stat failed - %d(%s)", 
                   full_entry_path.c_str(), errno, strerror(errno));
        return;
      }
      entry.length = (uint64_t)statbuf.st_size;
      entry.last_modification_time = statbuf.st_mtime;
      results.push_back(entry);
    }
  }

  void rmdir(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    std::error_code ec;
    std::filesystem::remove_all(abspath, ec);
    if (ec) {
      err = ec.value();
      HT_ERRORF("rmdir('%s') failed - %s", abspath.c_str(), strerror(errno));
      return;
    }
    HT_DEBUGF("rmdir('%s')", abspath.c_str());
  }
  
  void write(int &err, SmartFdPtr &smartfd,
             int32_t replication, int64_t blksz, 
             StaticBuffer &buffer) {
    HT_DEBUGF("write %s", smartfd->to_string().c_str());

    create(err, smartfd, 0, replication, blksz);
    if(!smartfd->valid() || err != Error::OK){
      if(err == Error::OK) 
        err = EBADF;
      goto finish;
    }

    if(buffer.size > 0) {
      append(err, smartfd, buffer, Flags::FLUSH);
      if(err != Error::OK)
        goto finish;
    }
    
    finish:
      int errtmp;
      if(smartfd->valid())
        close(err == Error::OK ? err : errtmp, smartfd);
    
    if(err != Error::OK)
      HT_ERRORF("write failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
  }

  void create(int &err, SmartFdPtr &smartfd, 
              int32_t bufsz, int32_t replication, int64_t blksz) override {

    std::string abspath = get_abspath(smartfd->filepath());
    HT_DEBUGF("create %s bufsz=%d replication=%d blksz=%lld",
              smartfd->to_string().c_str(), 
              bufsz, (int)replication, (Lld)blksz);

    int oflags = O_WRONLY | O_CREAT
        | (smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE 
           ? O_TRUNC : O_APPEND);
               
#ifdef O_DIRECT
    if (m_directio && smartfd->flags() & OpenFlags::OPEN_FLAG_DIRECTIO)
      oflags |= O_DIRECT;
#endif

    /* Open the file */
    errno = 0;
    smartfd->fd(::open(abspath.c_str(), oflags, 0644));
    if (!smartfd->valid()) {
      err = errno;
      HT_ERRORF("create failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());

      if(err == EACCES || err == ENOENT)
        err == Error::FS_PATH_NOT_FOUND;
      else if (err == EPERM)
        err == Error::FS_PERMISSION_DENIED;
      return;
    }
    
    HT_DEBUGF("created %s bufsz=%d replication=%d blksz=%lld",
              smartfd->to_string().c_str(), 
              bufsz, (int)replication, (Lld)blksz);

#if defined(__APPLE__)
#ifdef F_NOCACHE
    fcntl(smartfd->fd(), F_NOCACHE, 1);
#endif  
#elif defined(__sun__)
    if (m_directio)
      directio(smartfd->fd(), DIRECTIO_ON);
#endif

  }

  void open(int &err, SmartFdPtr &smartfd, int32_t bufsz=0) override {

    std::string abspath = get_abspath(smartfd->filepath());
    HT_DEBUGF("open %s, bufsz=%d", smartfd->to_string().c_str(), bufsz);

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
      HT_ERRORF("open failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
                
      if(err == EACCES || err == ENOENT)
        err == Error::FS_PATH_NOT_FOUND;
      else if (err == EPERM)
        err == Error::FS_PERMISSION_DENIED;
      return;
    }
    
    HT_DEBUGF("opened %s", smartfd->to_string().c_str());

#if defined(__sun__)
    if(m_directio)
      directio(smartfd->fd(), DIRECTIO_ON);
#endif

  }
  
  size_t read(int &err, SmartFdPtr &smartfd, 
              void *dst, size_t amount) override {
    
    HT_DEBUGF("read %s amount=%d", smartfd->to_string().c_str(), amount);
    ssize_t nread = 0;
    errno = 0;
    
    /*
    uint64_t offset;
    if ((offset = (uint64_t)lseek(smartfd->fd(), 0, SEEK_CUR)) == (uint64_t)-1) {
      err = errno;
      HT_ERRORF("read, lseek failed: %d(%s), %s offset=%d", 
                errno, strerror(errno), smartfd->to_string().c_str(), offset);
      return nread;
    }
    */
    
    nread = FileUtils::read(smartfd->fd(), dst, amount);
    if (nread == -1) {
      nread = 0;
      err = errno;
      HT_ERRORF("read failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
    } else {
      if(nread != amount)
        err = Error::FS_EOF;
      smartfd->pos(smartfd->pos()+nread);
      HT_DEBUGF("read(ed) %s amount=%d eof=%d", 
                smartfd->to_string().c_str(), nread, err == Error::FS_EOF);
    }
    return nread;
  }

  size_t pread(int &err, SmartFdPtr &smartfd, 
               uint64_t offset, void *dst, size_t amount) override {
    
    HT_DEBUGF("pread %s offset=%d amount=%d", 
               smartfd->to_string().c_str(), offset, amount);

    errno = 0;
    ssize_t nread = FileUtils::pread(smartfd->fd(), (off_t)offset, dst, amount);
    if (nread == -1) {
      nread = 0;
      err = errno;
      HT_ERRORF("pread failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
    } else {
      if(nread != amount)
        err = Error::FS_EOF;
      smartfd->pos(offset+nread);
      HT_DEBUGF("pread(ed) %s amount=%d  eof=%d", 
                smartfd->to_string().c_str(), nread, err == Error::FS_EOF);
    }
    return nread;
  }

  size_t append(int &err, SmartFdPtr &smartfd, 
                StaticBuffer &buffer, Flags flags) override {
    
    HT_DEBUGF("append %s amount=%d flags=%d", 
              smartfd->to_string().c_str(), buffer.size, flags);
    
    ssize_t nwritten = 0;
    errno = 0;

    /* 
    if (smartfd->pos() != 0 
      && lseek(smartfd->fd(), 0, SEEK_CUR) == (uint64_t)-1) {
      err = errno;
      HT_ERRORF("append, lseek failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return nwritten;
    }
    */

    if ((nwritten = FileUtils::write(
                    smartfd->fd(), buffer.base, buffer.size)) == -1) {
      nwritten = 0;
      err = errno;
      HT_ERRORF("write failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return nwritten;
    }
    smartfd->pos(smartfd->pos()+nwritten);
    
    if (flags == Flags::FLUSH || flags == Flags::SYNC) {
      if (fsync(smartfd->fd()) != 0) {     
        err = errno;
        HT_ERRORF("write, fsync failed: %d(%s), %s", 
                  errno, strerror(errno), smartfd->to_string().c_str());
      }
    }
    
    HT_DEBUGF("appended %s written=%d", 
              smartfd->to_string().c_str(), nwritten);
    return nwritten;
  }

  void seek(int &err, SmartFdPtr &smartfd, size_t offset) override {
    HT_DEBUGF("seek %s offset=%d", smartfd->to_string().c_str(), offset);
    
    errno = 0;
    uint64_t at = lseek(smartfd->fd(), offset, SEEK_SET); 
    if (at == (uint64_t)-1 || at != offset || errno != Error::OK) {
      err = errno;
      HT_ERRORF("seek failed - %d(%s) %s", 
                err, strerror(errno), smartfd->to_string().c_str());
      if(at > 0)
        smartfd->pos(at);
      return;
    }
    smartfd->pos(offset);
  }


  void flush(int &err, SmartFdPtr &smartfd) override {
    sync(err, smartfd);
  }
  void sync(int &err, SmartFdPtr &smartfd) override {
    HT_DEBUGF("sync %s", smartfd->to_string().c_str());
    
    errno = 0;
    if(fsync(smartfd->fd()) != Error::OK) {
      err = errno;
      HT_ERRORF("sync failed - %d(%s) %s", 
                err, strerror(errno), smartfd->to_string().c_str());
    }
  }

  void close(int &err, SmartFdPtr &smartfd) override {
    HT_DEBUGF("close %s", smartfd->to_string().c_str());
    ::close(smartfd->fd());
    smartfd->fd(-1);
    smartfd->pos(0);
  }



  private:
  bool m_directio;
};


}}



extern "C" {
SWC::FS::FileSystem* fs_make_new_local();
void fs_apply_cfg_local(SWC::Env::ConfigPtr env);
}

#endif  // swc_lib_fs_Local_FileSystem_h