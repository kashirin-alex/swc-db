/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/HadoopJVM/FileSystem.h"

#include <iostream>


namespace SWC { namespace FS {

 
Configurables apply_hadoop_jvm() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.hadoop_jvm.path.root", Config::str(""),
      "HadoopJVM FileSystem's base root path")
    ("swc.fs.hadoop_jvm.cfg.dyn", Config::strs(),
      "Dyn-config file")

    ("swc.fs.hadoop_jvm.namenode", Config::strs(),
      "Namenode Host + optional(:Port), muliple")
    ("swc.fs.hadoop_jvm.namenode.port", Config::i16(),
      "Namenode Port")
    ("swc.fs.hadoop_jvm.user", Config::str(),
      "HadoopJVM user")
    ("swc.fs.hadoop_jvm.fds.max", Config::g_i32(256),
      "Max Open Fds for opt. without closing")
    ("swc.fs.hadoop_jvm.reconnect.delay.ms", Config::g_i32(3000),
      "In ms delay use of connection after re-connect")
  ;

  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.hadoop_jvm.cfg", ""),
    "swc.fs.hadoop_jvm.cfg.dyn"
  );

  Configurables config;
  config.path_root = Env::Config::settings()->get_str(
    "swc.fs.hadoop_jvm.path.root");
  config.cfg_fds_max = Env::Config::settings()
    ->get<Config::Property::V_GINT32>("swc.fs.hadoop_jvm.fds.max");
  return config;
}



FileSystemHadoopJVM::SmartFdHadoopJVM::Ptr 
FileSystemHadoopJVM::SmartFdHadoopJVM::make_ptr(
        const std::string& filepath, uint32_t flags) {
  return std::make_shared<SmartFdHadoopJVM>(filepath, flags);
}

FileSystemHadoopJVM::SmartFdHadoopJVM::Ptr 
FileSystemHadoopJVM::SmartFdHadoopJVM::make_ptr(SmartFd::Ptr& smart_fd) {
  return std::make_shared<SmartFdHadoopJVM>(
    smart_fd->filepath(), smart_fd->flags(),
    smart_fd->fd(), smart_fd->pos()
  );
}

FileSystemHadoopJVM::SmartFdHadoopJVM::SmartFdHadoopJVM(
    const std::string& filepath, uint32_t flags, int32_t fd, uint64_t pos)
    : SmartFd(filepath, flags, fd, pos), m_hfile(nullptr), m_use_count(0) {
}

FileSystemHadoopJVM::SmartFdHadoopJVM::~SmartFdHadoopJVM() { }

hdfsFile FileSystemHadoopJVM::SmartFdHadoopJVM::file() noexcept {
  auto c = m_use_count.fetch_add(1);
  auto hfile = m_hfile.load();
  if(!hfile) {
     m_use_count.fetch_sub(1);
  } else if(c) {
    // Debugging Info -- that should not be happening!
    SWC_LOGF(LOG_WARN, "hfile '%s' waiting for file-use-released=%lu",
             to_string().c_str(), c);
    do std::this_thread::sleep_for(std::chrono::microseconds(1));
    while(m_use_count > 1);
    SWC_LOGF(LOG_WARN, "hfile '%s' waited for file-use-released",
             to_string().c_str());
  }
  return hfile;
}

void FileSystemHadoopJVM::SmartFdHadoopJVM::file(const hdfsFile& file)
                                                              noexcept {
  m_hfile.store(file);
}

void FileSystemHadoopJVM::SmartFdHadoopJVM::use_release() noexcept {
  m_use_count.fetch_sub(1);
}

bool FileSystemHadoopJVM::SmartFdHadoopJVM::file_used() const noexcept {
  return m_use_count;
}

hdfsFile FileSystemHadoopJVM::SmartFdHadoopJVM::invalidate() noexcept {
  auto hfile = m_hfile.exchange(nullptr);
  pos(0);
  fd(-1);
  return hfile;
}



FileSystemHadoopJVM::SmartFdHadoopJVM::Ptr 
FileSystemHadoopJVM::get_fd(SmartFd::Ptr& smartfd){
  auto hd_fd = std::dynamic_pointer_cast<SmartFdHadoopJVM>(smartfd);
  if(!hd_fd){
    hd_fd = SmartFdHadoopJVM::make_ptr(smartfd);
    smartfd = std::static_pointer_cast<SmartFd>(hd_fd);
  }
  return hd_fd;
}


FileSystemHadoopJVM::FileSystemHadoopJVM()
    : FileSystem(apply_hadoop_jvm()),
      m_nxt_fd(0), m_connecting(false),
      m_fs(setup_connection()),
      cfg_use_delay(
        Env::Config::settings()->get<Config::Property::V_GINT32>(
          "swc.fs.hadoop_jvm.reconnect.delay.ms")) {
}

FileSystemHadoopJVM::~FileSystemHadoopJVM() { }

Type FileSystemHadoopJVM::get_type() const noexcept {
  return Type::HADOOP_JVM;
};

std::string FileSystemHadoopJVM::to_string() const {
  return format(
    "(type=HADOOP_JVM path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}

void FileSystemHadoopJVM::stop() {
  m_run.store(false);
  {
    std::scoped_lock lock(m_mutex);
    m_fs = nullptr;
    m_cv.notify_all();
  }
  FileSystem::stop();
}

FileSystemHadoopJVM::Service::Ptr 
FileSystemHadoopJVM::setup_connection() {
  Service::Ptr fs;
  uint32_t tries=0;
  while(m_run && !initialize(fs)) {
    SWC_LOGF(LOG_ERROR,
      "FS-HadoopJVM, unable to initialize connection to hadoop_jvm, try=%d",
      ++tries);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  if(!fs)
    return fs;

  std::string abspath;
  get_abspath("", abspath);
  hdfsSetWorkingDirectory(fs->srv, abspath.c_str());

  int value = 0;
  hdfsConfGetInt("dfs.namenode.fs-limits.min-block-size", &value);
  if(value)
    hdfs_cfg_min_blk_sz = value;
  /*
  char* host;
  uint16_t port;
  hdfsConfGetStr("hdfs.namenode.host", &host);
  hdfsConfGetInt("hdfs.namenode.port", &port);
  SWC_LOGF(LOG_INFO,
            "FS-HadoopJV<, connected to namenode=[%s]:%d", host, port);
  hdfsConfStrFree(host);
  */

  // status check
  char buffer[256];
  hdfsGetWorkingDirectory(fs->srv, buffer, 256);
  SWC_LOGF(LOG_DEBUG, "FS-HadoopJVM, working Dir='%s'", buffer);
  return fs;
}

bool FileSystemHadoopJVM::initialize(FileSystemHadoopJVM::Service::Ptr& fs) {
  auto settings = Env::Config::settings();

  hdfsFS connection = nullptr;
  if (settings->has("swc.fs.hadoop_jvm.namenode")) {
    for(auto& h : settings->get_strs("swc.fs.hadoop_jvm.namenode")) {
      hdfsBuilder* bld = hdfsNewBuilder();
      hdfsBuilderSetNameNode(bld, h.c_str());

      if (settings->has("swc.fs.hadoop_jvm.namenode.port"))
        hdfsBuilderSetNameNodePort(
          bld, settings->get_i16("swc.fs.hadoop_jvm.namenode.port"));

      if (settings->has("swc.fs.hadoop_jvm.user"))
        hdfsBuilderSetUserName(
          bld,
          settings->get_str("swc.fs.hadoop_jvm.user").c_str()
        );

      SWC_LOGF(LOG_INFO, "Connecting to namenode=%s", h.c_str());
      connection = hdfsBuilderConnect(bld);
      // java.lang.IllegalArgumentException: java.net.UnknownHostException:
      if(!connection)
        continue;

      // check status, namenode need to be active
      errno = 0;
      int64_t sz_used = hdfsGetUsed(connection);
      if(sz_used == -1) {
        if(errno != 255)
          hdfsDisconnect(connection);
        connection = nullptr;
        SWC_LOGF(LOG_ERROR, "hdfsGetUsed('%s') failed - %d(%s)",
                  h.c_str(), errno, Error::get_text(errno));
        continue;
      }
      SWC_LOGF(LOG_INFO, "Non DFS Used bytes: %ld", sz_used);

      errno = 0;
      sz_used = hdfsGetCapacity(connection);
      if(sz_used == -1) {
        if(errno != 255)
          hdfsDisconnect(connection);
        connection = nullptr;
        SWC_LOGF(LOG_ERROR, "hdfsGetCapacity('%s') failed - %d(%s)",
                  h.c_str(), errno, Error::get_text(errno));
        continue;
      }
      SWC_LOGF(LOG_INFO, "Configured Capacity bytes: %ld", sz_used);
      break;
    }

  } else {
    //"default" > from the XML configuration file
    /*
    hdfsBuilder* bld = hdfsNewBuilder();
    hdfsBuilderSetNameNode(bld, "default");
    connection = hdfsBuilderConnect(bld);
    */
    connection = hdfsConnect("default", 0);

    char* value;
    hdfsConfGetStr("fs.defaultFS", &value);
    SWC_LOGF(LOG_INFO,
      "FS-HadoopJVM, connecting to default namenode=%s", value);
  }

  return bool(
    fs = connection ? std::make_shared<Service>(connection) : nullptr);
}

FileSystemHadoopJVM::Service::Ptr FileSystemHadoopJVM::get_fs(int& err) {
  if(m_run && !m_fs) {
    bool connect = false;
    {
      std::unique_lock lock_wait(m_mutex);
      if(m_connecting) {
        m_cv.wait(lock_wait, [this](){ return !m_connecting || !m_run; });
      } else {
        connect = m_connecting = true;
      }
    }
    if(connect) {
      auto fs = setup_connection();
      std::this_thread::sleep_for(
        std::chrono::milliseconds(cfg_use_delay->get()));
      std::scoped_lock lock(m_mutex);
      m_fs = fs;
      m_connecting = false;
      m_cv.notify_all();
    }
  }

  auto fs = m_fs;
  err = m_run
    ? (fs ? Error::OK : Error::SERVER_NOT_READY)
    : Error::SERVER_SHUTTING_DOWN;
  return fs;
}

void FileSystemHadoopJVM::need_reconnect(
        int& err, FileSystemHadoopJVM::Service::Ptr& fs) {
  if(err == 255) {
    std::scoped_lock lock(m_mutex);
    if(m_fs == fs)
      m_fs = nullptr;
  }
  // ? org.apache.hadoop.ipc.StandbyException
}


bool FileSystemHadoopJVM::exists(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

  bool state = false;
  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    state = !hdfsExists(fs->srv, abspath.c_str());
    need_reconnect(err = errno == ENOENT ? Error::OK : errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "exists('%s') state='%d' - %d(%s)",
    abspath.c_str(), (int)state, err, Error::get_text(err));
  return state;
}
  
void FileSystemHadoopJVM::remove(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(hdfsDelete(fs->srv, abspath.c_str(), false) == -1) {
      tmperr = errno;
      need_reconnect(
        err = (errno == EIO || errno == ENOENT ? Error::OK: errno), fs);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "remove('%s') - %d(%s)",
    abspath.c_str(), tmperr, Error::get_text(tmperr));
}

size_t FileSystemHadoopJVM::length(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  hdfsFileInfo *fileInfo;
  size_t len = 0;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(!(fileInfo = hdfsGetPathInfo(fs->srv, abspath.c_str()))) {
      need_reconnect(err = errno, fs);
    } else {
      len = fileInfo->mSize;
      hdfsFreeFileInfo(fileInfo, 1);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "length('%s') len='%lu' - %d(%s)",
    abspath.c_str(), len, err, Error::get_text(err));
  return len;
}

void FileSystemHadoopJVM::mkdirs(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(hdfsCreateDirectory(fs->srv, abspath.c_str()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "mkdirs('%s') - %d(%s)",
    abspath.c_str(), err, Error::get_text(err));
}

void FileSystemHadoopJVM::readdir(int& err, const std::string& name,
                                  DirentList& results) {
  std::string abspath;
  get_abspath(name, abspath);
  hdfsFileInfo *fileInfo;
  int numEntries;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if (!(fileInfo = hdfsListDirectory(
                      fs->srv, abspath.c_str(), &numEntries))) {
      need_reconnect(err = errno, fs);

    } else {
      for (int i=0; i<numEntries; ++i) {
        if (fileInfo[i].mName[0] == '.' || !fileInfo[i].mName[0])
          continue;
        auto& entry = results.emplace_back();
        const char *ptr;
        if ((ptr = strrchr(fileInfo[i].mName, '/')))
          entry.name = (std::string)(ptr+1);
        else
          entry.name = (std::string)fileInfo[i].mName;

        entry.length = fileInfo[i].mSize;
        entry.last_modification_time = fileInfo[i].mLastMod;
        entry.is_dir = fileInfo[i].mKind == kObjectKindDirectory;
      }
      hdfsFreeFileInfo(fileInfo, numEntries);
    }
  }

  SWC_LOGF(err && err != ENOENT ? LOG_ERROR: LOG_DEBUG,
    "readdir('%s') - %d(%s)",
    abspath.c_str(), err, Error::get_text(err));
}

void FileSystemHadoopJVM::rmdir(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(hdfsDelete(fs->srv, abspath.c_str(), true) == -1) {
      // io error(not-exists)
      need_reconnect(err = (tmperr = errno) == EIO ? ENOENT: tmperr, fs);
    }
  }
  SWC_LOGF(err && err != ENOENT ? LOG_ERROR: LOG_DEBUG,
    "rmdir('%s') - %d(%s)",
    abspath.c_str(), tmperr, Error::get_text(tmperr));
}

void FileSystemHadoopJVM::rename(int& err, const std::string& from,
                                 const std::string& to)  {
  std::string abspath_from;
  get_abspath(from, abspath_from);
  std::string abspath_to;
  get_abspath(to, abspath_to);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(hdfsRename(fs->srv, abspath_from.c_str(), abspath_to.c_str()) == -1)
      need_reconnect(err = errno == EIO ? ENOENT : errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "rename('%s' to '%s') - %d(%s)",
    abspath_from.c_str(), abspath_to.c_str(), err, Error::get_text(err));
}

void FileSystemHadoopJVM::create(int& err, SmartFd::Ptr& smartfd,
                                 int32_t bufsz, uint8_t replication,
                                 int64_t blksz) {
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);

  int oflags = O_WRONLY;
  if(!(smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE))
    oflags |= O_APPEND;

  if (bufsz <= -1)
    bufsz = 0;
  blksz = blksz <= hdfs_cfg_min_blk_sz ? 0 : (blksz/512)*512;
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    auto hadoop_fd = get_fd(smartfd);
    errno = 0;
    /* Open the file */
    auto hfile = hdfsOpenFile(
      fs->srv, abspath.c_str(), oflags, bufsz, replication, blksz);
    if(!hfile) {
      need_reconnect(tmperr = err = errno, fs);
      if(err == EACCES || err == ENOENT)
        err = Error::FS_PATH_NOT_FOUND;
      else if (err == EPERM)
        err = Error::FS_PERMISSION_DENIED;
    } else {
      hadoop_fd->file(hfile);
      hadoop_fd->fd(m_nxt_fd.add_rslt(1));
      fd_open_incr();
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "create %d(%s) bufsz=%d replication=%d blksz=%ld %s",
    tmperr, Error::get_text(tmperr), bufsz, replication, blksz,
    smartfd->to_string().c_str());
}

void FileSystemHadoopJVM::open(int& err, SmartFd::Ptr& smartfd,
                               int32_t bufsz) {
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);
  int oflags = O_RDONLY;
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    auto hadoop_fd = get_fd(smartfd);
    errno = 0;
    /* Open the file */
    auto hfile = hdfsOpenFile(fs->srv, abspath.c_str(), oflags,
                              bufsz<=-1 ? 0 : bufsz, 0, 0);
    if(!hfile) {
      need_reconnect(err = tmperr = errno, fs);
      if(err == EACCES || err == ENOENT)
        err = Error::FS_PATH_NOT_FOUND;
      else if (err == EPERM)
        err = Error::FS_PERMISSION_DENIED;
    } else {
      hadoop_fd->file(hfile);
      hadoop_fd->fd(m_nxt_fd.add_rslt(1));
      fd_open_incr();
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "open %d(%s) %s",
    tmperr, Error::get_text(tmperr), smartfd->to_string().c_str());
}
  
size_t FileSystemHadoopJVM::read(int& err, SmartFd::Ptr& smartfd,
              void *dst, size_t amount) {
  auto hadoop_fd = get_fd(smartfd);
  size_t ret = 0;
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->file();
    if(hfile) {
      errno = 0;
      ssize_t nread = hdfsRead(fs->srv, hfile, dst, (tSize)amount);
      hadoop_fd->use_release();
      if(nread == -1) {
        nread = 0;
        need_reconnect(err = tmperr = errno, fs);
      } else {
        if((ret = nread) != amount)
          err = Error::FS_EOF;
        hadoop_fd->forward(nread);
      }
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "read %d(%s) amount=%lu/%lu eof=%d %s",
    tmperr, Error::get_text(tmperr), ret, amount, err == Error::FS_EOF,
    hadoop_fd->to_string().c_str());
  return ret;
}

size_t FileSystemHadoopJVM::pread(int& err, SmartFd::Ptr& smartfd,
                               uint64_t offset, void *dst, size_t amount) {
  auto hadoop_fd = get_fd(smartfd);
  size_t ret = 0;
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->file();
    if(hfile) {
      errno = 0;
      ssize_t nread = hdfsPread(
        fs->srv, hfile, (tOffset)offset, dst, (tSize)amount);
      hadoop_fd->use_release();
      if(nread == -1) {
        nread = 0;
        need_reconnect(err = tmperr = errno, fs);
      } else {
        if((ret = nread) != amount)
          err = Error::FS_EOF;
        hadoop_fd->pos(offset + nread);
      }
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "pread %d(%s) offset=%lu amount=%lu/%lu eof=%d %s",
    tmperr, Error::get_text(tmperr), offset, ret, amount,
    err == Error::FS_EOF, hadoop_fd->to_string().c_str());
  return ret;
}

size_t FileSystemHadoopJVM::append(int& err, SmartFd::Ptr& smartfd,
                                StaticBuffer& buffer, Flags flags) {
  auto hadoop_fd = get_fd(smartfd);
  ssize_t nwritten = 0;

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->file();
    if(hfile) {
      errno = 0;
      nwritten = hdfsWrite(fs->srv, hfile, buffer.base, (tSize)buffer.size);
      if(nwritten == -1) {
        hadoop_fd->use_release();
        nwritten = 0;
        need_reconnect(err = errno, fs);
      } else {
        hadoop_fd->forward(nwritten);

        if(flags == Flags::FLUSH || flags == Flags::SYNC) {
          auto res = hdfsFlush(fs->srv, hfile);
          hadoop_fd->use_release();
          if(res == -1) {
            need_reconnect(err = errno, fs);
            SWC_LOGF(LOG_ERROR, "append-fsync %d(%s) %s",
                      err, Error::get_text(err), smartfd->to_string().c_str());
          }
        } else {
          hadoop_fd->use_release();
        }
      }
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "append %d(%s) amount=%lu flags=%d %s",
    err, Error::get_text(err), buffer.size, flags,
    hadoop_fd->to_string().c_str());
  return nwritten;
}

void FileSystemHadoopJVM::seek(int& err, SmartFd::Ptr& smartfd,
                               size_t offset) {
  auto hadoop_fd = get_fd(smartfd);

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->file();
    if(hfile) {
      errno = 0;
      auto res = hdfsSeek(fs->srv, hfile, (tOffset)offset);
      hadoop_fd->use_release();
      if(res == -1)
        need_reconnect(err = errno, fs);
      else
        hadoop_fd->pos(offset);
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "seek %d(%s) offset=%lu %s",
    err, Error::get_text(err), offset, hadoop_fd->to_string().c_str());
}

void FileSystemHadoopJVM::flush(int& err, SmartFd::Ptr& smartfd) {
  auto hadoop_fd = get_fd(smartfd);

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->file();
    if(hfile) {
      errno = 0;
      auto res = hdfsHFlush(fs->srv, hfile);
      hadoop_fd->use_release();
      if(res == -1)
        need_reconnect(err = errno, fs);
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "flush %d(%s) %s",
    err, Error::get_text(err), hadoop_fd->to_string().c_str());
}

void FileSystemHadoopJVM::sync(int& err, SmartFd::Ptr& smartfd) {
  auto hadoop_fd = get_fd(smartfd);

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->file();
    if(hfile) {
      errno = 0;
      auto res = hdfsHSync(fs->srv, hfile);
      hadoop_fd->use_release();
      if(res == -1)
        need_reconnect(err = errno, fs);
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "sync %d(%s) %s",
    err, Error::get_text(err), hadoop_fd->to_string().c_str());
}

void FileSystemHadoopJVM::close(int& err, SmartFd::Ptr& smartfd) {
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "close %s", hadoop_fd->to_string().c_str());

  auto fs = get_fs(err);
  if(!err) {
    auto hfile = hadoop_fd->invalidate();
    if(hfile) {
      fd_open_decr();
      if(hadoop_fd->file_used()) {
        SWC_LOGF(LOG_WARN, "close '%s' waiting for file-use-released",
          hadoop_fd->to_string().c_str());
        do std::this_thread::sleep_for(std::chrono::microseconds(1));
        while(hadoop_fd->file_used());
        hadoop_fd->pos(0);
        SWC_LOGF(LOG_WARN, "close '%s' waited for file-use-released",
          hadoop_fd->to_string().c_str());
      }
      errno = 0;
      if(hdfsCloseFile(fs->srv, hfile) == -1)
        err = errno == 255 ? Error::FS_BAD_FILE_HANDLE : errno;
    } else {
      err = EBADR;
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "close %d(%s) %s",
    err, Error::get_text(err), hadoop_fd->to_string().c_str());
}




}} // namespace SWC



extern "C" { 
SWC::FS::FileSystem* fs_make_new_hadoop_jvm() {
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemHadoopJVM());
};
void fs_apply_cfg_hadoop_jvm(SWC::Env::Config::Ptr env) {
  SWC::Env::Config::set(env);
};
}
