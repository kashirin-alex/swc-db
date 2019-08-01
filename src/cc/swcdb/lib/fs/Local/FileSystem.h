/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Local_FileSystem_h
#define swc_lib_fs_Local_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"
#include "swcdb/lib/core/FileUtils.h"

namespace SWC{ namespace FS {

bool apply_local();


class FileSystemLocal: public FileSystem {
  public:

  FileSystemLocal()
    : FileSystem(
        EnvConfig::settings()->get<String>("swc.fs.local.path.root"),
        apply_local()
      )
  { 
    m_directio = EnvConfig::settings()->get<bool>("swc.fs.local.DirectIO", false);
  }

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
    err = errno;
    HT_DEBUGF("exists state='%d' path='%s'", (int)state, abspath.c_str());
    return state;
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

    for(auto result : listing){
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
        err = errno;
        HT_ERRORF("readdir('%s') stat failed - %s", 
                   abspath.c_str(), strerror(errno));
        return;
      }
      entry.length = (uint64_t)statbuf.st_size;
      entry.last_modification_time = statbuf.st_mtime;
      results.push_back(entry);
    }
  }

  void remove(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    if (FileUtils::unlink(abspath) == -1) {
      err = errno;
      HT_ERRORF("remove('%s') failed - %s", abspath.c_str(), strerror(errno));
      return;
    }
    HT_DEBUGF("remove('%s')", abspath.c_str());
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
      HT_ERRORF("create failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
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
      HT_ERRORF("open failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
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
      err = errno;
      HT_ERRORF("read failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
    } else {

      smartfd->pos(nread);
      HT_DEBUGF("read(ed) %s amount=%d", smartfd->to_string().c_str(), nread);
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
      HT_ERRORF("append, lseek failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return nwritten;
    }
    */

    if ((nwritten = FileUtils::write(
                    smartfd->fd(), buffer.base, buffer.size)) == -1) {
      err = errno;
      HT_ERRORF("write failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return nwritten;
    }
    smartfd->pos(nwritten);
    
    if (flags == Flags::FLUSH || flags == Flags::SYNC) {
      if (fsync(smartfd->fd()) != 0) {     
        err = errno;
        HT_ERRORF("write, fsync failed: %d(%s),  %s", 
                  errno, strerror(errno), smartfd->to_string().c_str());
      }
    }
    
    HT_DEBUGF("appended %s written=%d", 
              smartfd->to_string().c_str(), nwritten);
    return nwritten;
  }




  void close(int &err, SmartFdPtr &smartfd) {
    HT_DEBUGF("close %s", smartfd->to_string().c_str());
    ::close(smartfd->fd());
  }



  private:
  bool m_directio;
};


}}



#endif  // swc_lib_fs_Local_FileSystem_h