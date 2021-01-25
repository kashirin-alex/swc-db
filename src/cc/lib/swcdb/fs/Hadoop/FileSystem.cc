/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/Hadoop/FileSystem.h"

#include <iostream>

#include "hdfspp/config_parser.h"


namespace SWC { namespace FS {


Configurables apply_hadoop() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.hadoop.path.root", Config::str(""),
      "Hadoop FileSystem's base root path")
    ("swc.fs.hadoop.cfg.dyn", Config::strs(),
      "Dyn-config file")

    ("swc.fs.hadoop.namenode", Config::strs(),
      "Namenode Host + optional(:Port), muliple")
    ("swc.fs.hadoop.namenode.port", Config::i16(),
      "Namenode Port")
    ("swc.fs.hadoop.user", Config::str(),
      "Hadoop user")
    ("swc.fs.hadoop.handlers", Config::i32(48),
      "Handlers for hadoop tasks")
    ("swc.fs.hadoop.fds.max", Config::g_i32(256),
      "Max Open Fds for opt. without closing")
  ;

  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.hadoop.cfg", ""),
    "swc.fs.hadoop.cfg.dyn"
  );

  Configurables config;
  config.path_root = Env::Config::settings()->get_str(
    "swc.fs.hadoop.path.root");
  config.cfg_fds_max = Env::Config::settings()
    ->get<Config::Property::V_GINT32>("swc.fs.hadoop.fds.max");
  return config;
}



FileSystemHadoop::SmartFdHadoop::Ptr
FileSystemHadoop::SmartFdHadoop::make_ptr(
      const std::string& filepath, uint32_t flags) {
  return std::make_shared<SmartFdHadoop>(filepath, flags);
}

FileSystemHadoop::SmartFdHadoop::Ptr
FileSystemHadoop::SmartFdHadoop::make_ptr(SmartFd::Ptr& smart_fd) {
  return std::make_shared<SmartFdHadoop>(
    smart_fd->filepath(), smart_fd->flags(),
    smart_fd->fd(), smart_fd->pos()
  );
}

FileSystemHadoop::SmartFdHadoop::SmartFdHadoop(
    const std::string& filepath, uint32_t flags, int32_t fd, uint64_t pos)
    : SmartFd(filepath, flags, fd, pos), m_file(nullptr) {
}

FileSystemHadoop::SmartFdHadoop::~SmartFdHadoop() { }

hdfs::FileHandle* FileSystemHadoop::SmartFdHadoop::file() const {
  return m_file;
}

void FileSystemHadoop::SmartFdHadoop::file(hdfs::FileHandle* file) {
  m_file = file;
}



FileSystemHadoop::SmartFdHadoop::Ptr
FileSystemHadoop::get_fd(SmartFd::Ptr& smartfd){
  auto hd_fd = std::dynamic_pointer_cast<SmartFdHadoop>(smartfd);
  if(!hd_fd){
    hd_fd = SmartFdHadoop::make_ptr(smartfd);
    smartfd = std::static_pointer_cast<SmartFd>(hd_fd);
  }
  return hd_fd;
}


FileSystemHadoop::FileSystemHadoop()
    : FileSystem(apply_hadoop()),
      m_nxt_fd(0), m_connecting(false),
      m_fs(setup_connection()) {
}

FileSystemHadoop::~FileSystemHadoop() { }

Type FileSystemHadoop::get_type() const noexcept {
  return Type::HADOOP;
}

std::string FileSystemHadoop::to_string() const {
  return format(
    "(type=HADOOP path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}

void FileSystemHadoop::stop() {
  m_run.store(false);
  {
    std::scoped_lock lock(m_mutex);
    m_fs = nullptr;
    m_cv.notify_all();
  }
  FileSystem::stop();
}

FileSystemHadoop::Service::Ptr
FileSystemHadoop::setup_connection() {
  Service::Ptr fs;
  uint32_t tries=0;
  while(m_run && !initialize(fs)) {
    SWC_LOGF(LOG_ERROR,
      "FS-Hadoop, unable to initialize connection to hadoop, try=%d",
      ++tries);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  if(!fs)
    return fs;

  //std::string abspath;
  //get_abspath("", abspath);
  //hdfsSetWorkingDirectory(fs->srv, abspath.c_str());

  //int value = 0;
  //hdfsConfGetInt("dfs.namenode.fs-limits.min-block-size", &value);
  //if(value)
    //hdfs_cfg_min_blk_sz = value;
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
  //char buffer[256];
  //hdfsGetWorkingDirectory(fs->srv, buffer, 256);
  //SWC_LOGF(LOG_DEBUG, "FS-Hadoop, working Dir='%s'", buffer);
  return fs;
}

bool FileSystemHadoop::initialize(FileSystemHadoop::Service::Ptr& fs) {
  auto settings = Env::Config::settings();

  hdfs::ConfigParser parser;
  if(!parser.LoadDefaultResources())
    SWC_LOG_FATAL("hdfs::ConfigParser could not load default resources.");

  auto stats = parser.ValidateResources();
  for(auto& s : stats) {
    SWC_PRINT << s.first << "=" << s.second.ToString() << SWC_PRINT_CLOSE;
    SWC_ASSERT(s.second.ok());
  }
  SWC_PRINT << " fs.defaultFS=" << parser.get_string_or("fs.defaultFS", "NONE") << SWC_PRINT_CLOSE;

  hdfs::Options options;
  if(!parser.get_options(options))
    SWC_LOG_FATAL("hdfs::ConfigParser could not load Options object.");

  hdfs::IoService* io_service = hdfs::IoService::New();
  io_service->InitWorkers(settings->get_i32("swc.fs.hadoop.handlers"));

  hdfs::FileSystem* connection = hdfs::FileSystem::New(
    io_service,
    settings->get_str("swc.fs.hadoop.user", ""),
    options
  );
  SWC_PRINT << "hdfs::FileSystem Initialized" << SWC_PRINT_CLOSE;

  hdfs::FsInfo fs_info;
  hdfs::Status status;
  std::string port;
  if(settings->has("swc.fs.hadoop.namenode")) {
    for(auto& h : settings->get_strs("swc.fs.hadoop.namenode")) {
      port = std::to_string(settings->get_i16(
        "swc.fs.hadoop.namenode.port", 0));

      SWC_LOGF(LOG_DEBUG, "FS-Hadoop, Connecting to namenode=[%s]:%s",
                            h.c_str(), port.c_str());

      status = connection->Connect(h, port);
      if(status.ok()) {
        status =  connection->GetFsStats(fs_info);
        if(status.ok())
          break;
      }
      SWC_PRINT << "FS-Hadoop, Could not connect to " << h << ":" << port
                << ". " << status.ToString() << SWC_PRINT_CLOSE;
    }

  } else {
    SWC_LOGF(LOG_DEBUG, "FS-Hadoop, connecting to default namenode=%s", options.defaultFS.get_path().c_str());
    status = connection->ConnectToDefaultFs();
    if(status.ok())
      status =  connection->GetFsStats(fs_info);
    if(!status.ok())
      SWC_PRINT << "FS-Hadoop, Could not connect to " << options.defaultFS
                << ". " << status.ToString() << SWC_PRINT_CLOSE;
  }

  if(status.ok()) {
    SWC_LOGF(LOG_INFO, "%s", fs_info.str("FS-Hadoop").c_str());
    fs = std::make_shared<Service>(connection);
  } else {
    delete connection;
  }

  return bool(fs);
}

FileSystemHadoop::Service::Ptr FileSystemHadoop::get_fs(int& err) {
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

void FileSystemHadoop::need_reconnect(
        int& err, FileSystemHadoop::Service::Ptr& fs) {
  if(err == 255) {
    std::scoped_lock lock(m_mutex);
    if(m_fs == fs)
      m_fs = nullptr;
  }
  // ? org.apache.hadoop.ipc.StandbyException
}


bool FileSystemHadoop::exists(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

  bool state = false;
  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    state = false; //!hdfsExists(fs->srv, abspath.c_str());
    need_reconnect(err = errno == ENOENT ? Error::OK : errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "exists('%s') state='%d' - %d(%s)",
    abspath.c_str(), state, err, Error::get_text(err));
  return state;
}

void FileSystemHadoop::remove(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if (true) { // hdfsDelete(fs->srv, abspath.c_str(), false) == -1) {
      tmperr = errno;
      need_reconnect(
        err = (errno == EIO || errno == ENOENT ? Error::OK: errno), fs);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "remove('%s') - %d(%s)",
    abspath.c_str(), tmperr, Error::get_text(tmperr));
}

size_t FileSystemHadoop::length(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  //hdfsFileInfo *fileInfo;
  size_t len = 0;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true) {//!(fileInfo = hdfsGetPathInfo(fs->srv, abspath.c_str()))) {
      need_reconnect(err = errno, fs);
    } else {
      //len = fileInfo->mSize;
      //hdfsFreeFileInfo(fileInfo, 1);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "length('%s') len='%lu' - %d(%s)",
    abspath.c_str(), len, err, Error::get_text(err));
  return len;
}

void FileSystemHadoop::mkdirs(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(false) //hdfsCreateDirectory(fs->srv, abspath.c_str()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "mkdirs('%s') - %d(%s)",
    abspath.c_str(), err, Error::get_text(err));
}

void FileSystemHadoop::readdir(int& err, const std::string& name,
                               DirentList& results) {
  std::string abspath;
  get_abspath(name, abspath);
  (void)results;
  /*
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
        if((ptr = strrchr(fileInfo[i].mName, '/')))
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
  */
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "readdir('%s') - %d(%s)",
    abspath.c_str(), err, Error::get_text(err));
}

void FileSystemHadoop::rmdir(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(false) { // hdfsDelete(fs->srv, abspath.c_str(), true) == -1) {
      // io error(not-exists)
      need_reconnect(err = (tmperr = errno) == EIO ? ENOENT: tmperr, fs);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "rmdir('%s') - %d(%s)",
    abspath.c_str(), tmperr, Error::get_text(tmperr));
}

void FileSystemHadoop::rename(int& err, const std::string& from,
                                 const std::string& to)  {
  std::string abspath_from;
  get_abspath(from, abspath_from);
  std::string abspath_to;
  get_abspath(to, abspath_to);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(false) // hdfsRename(fs->srv, abspath_from.c_str(), abspath_to.c_str()) == -1)
      need_reconnect(err = errno == EIO ? ENOENT : errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "rename('%s' to '%s') - %d(%s)",
    abspath_from.c_str(), abspath_to.c_str(), err, Error::get_text(err));
}

void FileSystemHadoop::create(int& err, SmartFd::Ptr& smartfd,
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
    hdfs::FileHandle* fd = 0;
    (void)oflags;
    // fd = hdfsOpenFile(
      // fs->srv, abspath.c_str(), oflags, bufsz, replication, blksz);
    if(!fd) {
      need_reconnect(tmperr = err = errno, fs);
      hadoop_fd->file(fd);
      hadoop_fd->fd(-1);

      if(err == EACCES || err == ENOENT)
        err = Error::FS_PATH_NOT_FOUND;
      else if (err == EPERM)
        err = Error::FS_PERMISSION_DENIED;
    } else {
      hadoop_fd->file(fd);
      hadoop_fd->fd(m_nxt_fd.add_rslt(1));
      fd_open_incr();
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "create %d(%s) bufsz=%d replication=%d blksz=%ld %s",
    tmperr, Error::get_text(tmperr), bufsz, replication, blksz,
    smartfd->to_string().c_str());
}

void FileSystemHadoop::open(int& err, SmartFd::Ptr& smartfd,
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
    hdfs::FileHandle* fd = 0;
    (void)oflags;
    (void)bufsz;
    //fd = hdfsOpenFile(fs->srv, abspath.c_str(), oflags,
    //                        bufsz<=-1 ? 0 : bufsz, 0, 0);
    if(!fd) {
      need_reconnect(err = tmperr = errno, fs);
      hadoop_fd->file(fd);
      hadoop_fd->fd(-1);

      if(err == EACCES || err == ENOENT)
        err = Error::FS_PATH_NOT_FOUND;
      else if (err == EPERM)
        err = Error::FS_PERMISSION_DENIED;
    } else {
      hadoop_fd->file(fd);
      hadoop_fd->fd(m_nxt_fd.add_rslt(1));
      fd_open_incr();
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "open %d(%s) %s",
    tmperr, Error::get_text(tmperr), smartfd->to_string().c_str());
}

size_t FileSystemHadoop::read(int& err, SmartFd::Ptr& smartfd,
              void *dst, size_t amount) {
  auto hadoop_fd = get_fd(smartfd);
  size_t ret = 0;
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    /*
    uint64_t offset;
    if ((offset = (uint64_t)hdfsTell(fs->srv, hadoop_fd->file()))
                  == (uint64_t)-1) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "read, tell failed: %d(%s), %s offset=%lu",
                err, Error::get_text(err), smartfd->to_string().c_str(), offset);
      return nread;
    }
    */
    (void)dst;
    errno = 0;
    ssize_t nread = -1; //(ssize_t)hdfsRead(fs->srv, hadoop_fd->file(),
                        //              dst, (tSize)amount);
    if(nread == -1) {
      nread = 0;
      need_reconnect(err = tmperr = errno, fs);
    } else {
      if((ret = nread) != amount)
        err = Error::FS_EOF;
      hadoop_fd->forward(nread);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG, 
    "read %d(%s) amount=%lu/%lu eof=%d %s",
    tmperr, Error::get_text(tmperr), ret, amount, err == Error::FS_EOF,
    hadoop_fd->to_string().c_str());
  return ret;
}

size_t FileSystemHadoop::pread(int& err, SmartFd::Ptr& smartfd, 
                               uint64_t offset, void *dst, size_t amount) {
  auto hadoop_fd = get_fd(smartfd);
  size_t ret = 0;
  int tmperr = Error::OK;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    (void)dst;
    ssize_t nread = -1; //(ssize_t)hdfsPread(
      // fs->srv, hadoop_fd->file(), (tOffset)offset, dst, (tSize)amount);
    if(nread == -1) {
      nread = 0;
      need_reconnect(err = tmperr = errno, fs);
    } else {
      if((ret = nread) != amount)
        err = Error::FS_EOF;
      hadoop_fd->pos(offset + nread);
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG, 
    "pread %d(%s) offset=%lu amount=%lu/%lu eof=%d %s", 
    tmperr, Error::get_text(tmperr), offset, ret, amount, 
    err == Error::FS_EOF, hadoop_fd->to_string().c_str());
  return ret;
}

size_t FileSystemHadoop::append(int& err, SmartFd::Ptr& smartfd, 
                                StaticBuffer& buffer, Flags flags) {
  auto hadoop_fd = get_fd(smartfd);
  ssize_t nwritten = 0;

  auto fs = get_fs(err);
  if(!err) {
    /* 
    uint64_t offset;
    if ((offset = (uint64_t)hdfsTell(fs->srv, hadoop_fd->file()))
            == (uint64_t)-1) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "write, tell failed: %d(%s), %s offset=%lu", 
                err, Error::get_text(err), smartfd->to_string().c_str(), offset);
      return nwritten;
    }
    */
    errno = 0;
    if(true) { //(nwritten = (ssize_t)hdfsWrite(fs->srv, hadoop_fd->file(), 
               //                buffer.base, (tSize)buffer.size)) == -1) {
      nwritten = 0;
      need_reconnect(err = errno, fs);
    } else {
      hadoop_fd->forward(nwritten);

      if(flags == Flags::FLUSH || flags == Flags::SYNC) {
        if(true) { //hdfsFlush(fs->srv, hadoop_fd->file()) == -1) {
          need_reconnect(err = errno, fs);
          SWC_LOGF(LOG_ERROR, "append-fsync %d(%s) %s", 
                    err, Error::get_text(err), smartfd->to_string().c_str());
        }
      }
    }
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG, 
    "append %d(%s) amount=%lu flags=%d %s", 
    err, Error::get_text(err), buffer.size, flags,
    hadoop_fd->to_string().c_str());
  return nwritten;
}

void FileSystemHadoop::seek(int& err, SmartFd::Ptr& smartfd, 
                               size_t offset) {
  auto hadoop_fd = get_fd(smartfd);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true)//hdfsSeek(fs->srv, hadoop_fd->file(), (tOffset)offset) == -1)
      need_reconnect(err = errno, fs);
    else
      hadoop_fd->pos(offset);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG, 
    "seek %d(%s) offset=%lu %s", 
    err, Error::get_text(err), offset, hadoop_fd->to_string().c_str());
}

void FileSystemHadoop::flush(int& err, SmartFd::Ptr& smartfd) {
  auto hadoop_fd = get_fd(smartfd);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true)//hdfsHFlush(fs->srv, hadoop_fd->file()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG, 
    "flush %d(%s) %s", 
    err, Error::get_text(err), hadoop_fd->to_string().c_str());
}

void FileSystemHadoop::sync(int& err, SmartFd::Ptr& smartfd) {
  auto hadoop_fd = get_fd(smartfd);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true)//hdfsHSync(fs->srv, hadoop_fd->file()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG, 
    "sync %d(%s) %s", 
    err, Error::get_text(err), hadoop_fd->to_string().c_str());
}

void FileSystemHadoop::close(int& err, SmartFd::Ptr& smartfd) {
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "close %s", hadoop_fd->to_string().c_str());

  if(hadoop_fd->file()) {
    fd_open_decr();
    auto fs = get_fs(err);
    if(!err) {
      errno = 0;
      if(true)//hdfsCloseFile(fs->srv, hadoop_fd->file()) == -1)
        need_reconnect(err = errno, fs);
      hadoop_fd->file(0);
    }
  } else {
    err = EBADR;
  }
  hadoop_fd->fd(-1);
  smartfd->pos(0);

  SWC_LOGF(err ? LOG_ERROR: LOG_DEBUG,
    "close %d(%s) %s", 
    err, Error::get_text(err), hadoop_fd->to_string().c_str());
}







}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_hadoop() {
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemHadoop());
}
void fs_apply_cfg_hadoop(SWC::Env::Config::Ptr env) {
  SWC::Env::Config::set(env);
}
}
