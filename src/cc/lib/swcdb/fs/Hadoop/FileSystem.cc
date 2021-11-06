/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Hadoop/FileSystem.h"

extern "C" {
#include <fcntl.h>
}

#include "hdfspp/config_parser.h"


namespace SWC { namespace FS {


Configurables* apply_hadoop(Configurables* config) {
  config->settings->file_desc.add_options()
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

    ("swc.fs.hadoop.metrics.enabled", Config::boo(true),
     "Enable or Disable Metrics Tracking")

    ("swc.fs.hadoop.fds.max", Config::g_i32(256),
      "Max Open Fds for opt. without closing")
  ;

  config->settings->parse_file(
    config->settings->get_str("swc.fs.hadoop.cfg", ""),
    "swc.fs.hadoop.cfg.dyn"
  );

  config->path_root = config->settings->get_str(
    "swc.fs.hadoop.path.root");
  config->cfg_fds_max = config->settings
    ->get<Config::Property::Value_int32_g>("swc.fs.hadoop.fds.max");
  config->stats_enabled = config->settings->get_bool(
    "swc.fs.hadoop.metrics.enabled");
  return config;
}



FileSystemHadoop::SmartFdHadoop::Ptr
FileSystemHadoop::SmartFdHadoop::make_ptr(
      const std::string& filepath, uint32_t flags) {
  return SmartFdHadoop::Ptr(new SmartFdHadoop(filepath, flags));
}

FileSystemHadoop::SmartFdHadoop::Ptr
FileSystemHadoop::SmartFdHadoop::make_ptr(SmartFd::Ptr& smart_fd) {
  return SmartFdHadoop::Ptr(new SmartFdHadoop(
    smart_fd->filepath(), smart_fd->flags(),
    smart_fd->fd(), smart_fd->pos()
  ));
}

FileSystemHadoop::SmartFdHadoop::SmartFdHadoop(
    const std::string& filepath, uint32_t flags, int32_t fd, uint64_t pos)
    : SmartFd(filepath, flags, fd, pos), m_file(nullptr) {
}

FileSystemHadoop::SmartFdHadoop::~SmartFdHadoop() noexcept { }

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


FileSystemHadoop::FileSystemHadoop(Configurables* config)
    : FileSystem(apply_hadoop(config)),
      m_nxt_fd(0), m_connecting(false),
      m_fs(setup_connection()) {
}

FileSystemHadoop::~FileSystemHadoop() noexcept { }

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
    Core::ScopedLock lock(m_mutex);
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
      "FS-Hadoop, unable to initialize connection to hadoop, try=%u",
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
    fs.reset(new Service(connection));
  } else {
    delete connection;
  }

  return bool(fs);
}

FileSystemHadoop::Service::Ptr FileSystemHadoop::get_fs(int& err) {
  if(m_run && !m_fs) {
    bool connect = false;
    {
      Core::UniqueLock lock_wait(m_mutex);
      if(m_connecting) {
        m_cv.wait(lock_wait, [this](){ return !m_connecting || !m_run; });
      } else {
        connect = m_connecting = true;
      }
    }
    if(connect) {
      auto fs = setup_connection();
      Core::ScopedLock lock(m_mutex);
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
    Core::ScopedLock lock(m_mutex);
    if(m_fs == fs)
      m_fs = nullptr;
  }
  // ? org.apache.hadoop.ipc.StandbyException
}


bool FileSystemHadoop::exists(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::EXISTS_SYNC);
  SWC_FS_EXISTS_START(name);
  std::string abspath;
  get_abspath(name, abspath);
  bool state = false;
  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    state = false; //!hdfsExists(fs->srv, abspath.c_str());
    need_reconnect(err = errno == ENOENT ? Error::OK : errno, fs);
  }
  SWC_FS_EXISTS_FINISH(err, abspath, state, tracker);
  return state;
}

void FileSystemHadoop::remove(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::REMOVE_SYNC);
  SWC_FS_REMOVE_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if (true) { // hdfsDelete(fs->srv, abspath.c_str(), false) == -1) {
      err = (errno == EIO || errno == ENOENT ? Error::OK: errno);
      need_reconnect(err, fs);
    }
  }
  SWC_FS_REMOVE_FINISH(err, abspath, tracker);
}

size_t FileSystemHadoop::length(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::LENGTH_SYNC);
  SWC_FS_LENGTH_START(name);
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
  SWC_FS_LENGTH_FINISH(err, abspath, len, tracker);
  return len;
}

void FileSystemHadoop::mkdirs(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::MKDIRS_SYNC);
  SWC_FS_MKDIRS_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(false) //hdfsCreateDirectory(fs->srv, abspath.c_str()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_FS_MKDIRS_FINISH(err, abspath, tracker);
}

void FileSystemHadoop::readdir(int& err, const std::string& name,
                               DirentList& results) {
  auto tracker = statistics.tracker(Statistics::READDIR_SYNC);
  SWC_FS_READDIR_START(name);
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
  SWC_FS_READDIR_FINISH(err, abspath, results.size(), tracker);
}

void FileSystemHadoop::rmdir(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::RMDIR_SYNC);
  SWC_FS_RMDIR_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(false) { // hdfsDelete(fs->srv, abspath.c_str(), true) == -1) {
      err = errno == EIO ? ENOENT: errno;
      need_reconnect(err, fs);
    }
  }
  SWC_FS_RMDIR_FINISH(err == ENOENT? Error::OK : err, abspath, tracker);
}

void FileSystemHadoop::rename(int& err, const std::string& from,
                                 const std::string& to)  {
  auto tracker = statistics.tracker(Statistics::RENAME_SYNC);
  SWC_FS_RENAME_START(from, to);
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
  SWC_FS_RENAME_FINISH(err, abspath_from, abspath_to, tracker);
}

void FileSystemHadoop::create(int& err, SmartFd::Ptr& smartfd,
                              uint8_t replication, int64_t blksz) {
  auto tracker = statistics.tracker(Statistics::CREATE_SYNC);
  SWC_FS_CREATE_START(smartfd, replication, blksz);
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);

  int oflags = O_WRONLY;
  if(!(smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE))
    oflags |= O_APPEND;

  blksz = blksz <= hdfs_cfg_min_blk_sz ? 0 : (blksz/512)*512;
  int tmperr;

  auto fs = get_fs(err);
  if(!(tmperr = err)) {
    auto hadoop_fd = get_fd(smartfd);
    errno = 0;
    /* Open the file */
    hdfs::FileHandle* fd = nullptr;
    (void)oflags;
    // fd = hdfsOpenFile(
      // fs->srv, abspath.c_str(), oflags, replication, blksz);
    if(!fd) {
      need_reconnect(tmperr = errno, fs);
      hadoop_fd->file(fd);
      hadoop_fd->fd(-1);

      if(tmperr == EACCES || tmperr == ENOENT)
        err = Error::FS_PATH_NOT_FOUND;
      else if (tmperr == EPERM)
        err = Error::FS_PERMISSION_DENIED;
      else
        err = tmperr;
    } else {
      hadoop_fd->file(fd);
      hadoop_fd->fd(m_nxt_fd.add_rslt(1));
      fd_open_incr();
    }
  }
  SWC_FS_CREATE_FINISH(tmperr, smartfd, fds_open(), tracker);
}

void FileSystemHadoop::open(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::OPEN_SYNC);
  SWC_FS_OPEN_START(smartfd);
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);
  int oflags = O_RDONLY;
  int tmperr;

  auto fs = get_fs(err);
  if(!(tmperr = err)) {
    auto hadoop_fd = get_fd(smartfd);
    errno = 0;
    /* Open the file */
    hdfs::FileHandle* fd = nullptr;
    (void)oflags;
    //fd = hdfsOpenFile(fs->srv, abspath.c_str(), oflags, 0, 0, 0);
    if(!fd) {
      need_reconnect(tmperr = errno, fs);
      hadoop_fd->file(fd);
      hadoop_fd->fd(-1);

      if(tmperr == EACCES || tmperr == ENOENT)
        err = Error::FS_PATH_NOT_FOUND;
      else if (tmperr == EPERM)
        err = Error::FS_PERMISSION_DENIED;
      else
        err = tmperr;
    } else {
      hadoop_fd->file(fd);
      hadoop_fd->fd(m_nxt_fd.add_rslt(1));
      fd_open_incr();
    }
  }
  SWC_FS_OPEN_FINISH(tmperr, smartfd, fds_open(), tracker);
}

size_t FileSystemHadoop::read(int& err, SmartFd::Ptr& smartfd,
              void *dst, size_t amount) {
  auto tracker = statistics.tracker(Statistics::READ_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_READ_START(hadoop_fd, amount);
  size_t ret = 0;

  auto fs = get_fs(err);
  if(!err) {
    (void)dst;
    errno = 0;
    ssize_t nread = -1; //(ssize_t)hdfsRead(fs->srv, hadoop_fd->file(),
                        //              dst, (tSize)amount);
    if(nread == -1) {
      nread = 0;
      need_reconnect(err = errno, fs);
    } else {
      if((ret = nread) != amount)
        err = Error::FS_EOF;
      hadoop_fd->forward(nread);
    }
  }
  SWC_FS_READ_FINISH(err, hadoop_fd, ret, tracker);
  return ret;
}

size_t FileSystemHadoop::pread(int& err, SmartFd::Ptr& smartfd,
                               uint64_t offset, void *dst, size_t amount) {
  auto tracker = statistics.tracker(Statistics::PREAD_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_PREAD_START(hadoop_fd, offset, amount);
  size_t ret = 0;

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    (void)dst;
    ssize_t nread = -1; //(ssize_t)hdfsPread(
      // fs->srv, hadoop_fd->file(), (tOffset)offset, dst, (tSize)amount);
    if(nread == -1) {
      nread = 0;
      need_reconnect(err = errno, fs);
    } else {
      if((ret = nread) != amount)
        err = Error::FS_EOF;
      hadoop_fd->pos(offset + nread);
    }
  }
  SWC_FS_PREAD_FINISH(err, hadoop_fd, ret, tracker);
  return ret;
}

size_t FileSystemHadoop::append(int& err, SmartFd::Ptr& smartfd,
                                StaticBuffer& buffer, Flags flags) {
  auto tracker = statistics.tracker(Statistics::APPEND_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_APPEND_START(hadoop_fd, buffer.size, flags);
  ssize_t nwritten = 0;

  auto fs = get_fs(err);
  if(!err) {
    /*
    uint64_t offset;
    if ((offset = (uint64_t)hdfsTell(fs->srv, hadoop_fd->file()))
            == (uint64_t)-1) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "write, tell failed: %d(%s), %s offset=" SWC_FMT_LU,
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
        }
      }
    }
  }
  SWC_FS_APPEND_FINISH(err, hadoop_fd, nwritten, tracker);
  return nwritten;
}

void FileSystemHadoop::seek(int& err, SmartFd::Ptr& smartfd,
                               size_t offset) {
  auto tracker = statistics.tracker(Statistics::SEEK_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_SEEK_START(hadoop_fd, offset);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true)//hdfsSeek(fs->srv, hadoop_fd->file(), (tOffset)offset) == -1)
      need_reconnect(err = errno, fs);
    else
      hadoop_fd->pos(offset);
  }
  SWC_FS_SEEK_FINISH(err, hadoop_fd, tracker);
}

void FileSystemHadoop::flush(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::FLUSH_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_FLUSH_START(hadoop_fd);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true)//hdfsHFlush(fs->srv, hadoop_fd->file()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_FS_FLUSH_FINISH(err, hadoop_fd, tracker);
}

void FileSystemHadoop::sync(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::SYNC_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_SYNC_START(hadoop_fd);

  auto fs = get_fs(err);
  if(!err) {
    errno = 0;
    if(true)//hdfsHSync(fs->srv, hadoop_fd->file()) == -1)
      need_reconnect(err = errno, fs);
  }
  SWC_FS_SYNC_FINISH(err, hadoop_fd, tracker);
}

void FileSystemHadoop::close(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::CLOSE_SYNC);
  auto hadoop_fd = get_fd(smartfd);
  SWC_FS_CLOSE_START(hadoop_fd);

  if(hadoop_fd->file()) {
    fd_open_decr();
    auto fs = get_fs(err);
    if(!err) {
      errno = 0;
      if(true)//hdfsCloseFile(fs->srv, hadoop_fd->file()) == -1)
        need_reconnect(err = errno, fs);
      hadoop_fd->file(nullptr);
    }
  } else {
    err = EBADR;
  }
  hadoop_fd->fd(-1);
  hadoop_fd->pos(0);

  SWC_FS_CLOSE_FINISH(err, hadoop_fd, tracker);
}







}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_hadoop(SWC::FS::Configurables* config) {
  return static_cast<SWC::FS::FileSystem*>(
    new SWC::FS::FileSystemHadoop(config));
}
}
