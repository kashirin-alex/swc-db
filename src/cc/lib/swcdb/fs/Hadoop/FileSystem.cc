/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/fs/Hadoop/FileSystem.h"

#include <iostream>

#include "hdfspp/config_parser.h"

#include <fcntl.h>

namespace SWC{ namespace FS {

 
bool apply_hadoop() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.hadoop.path.root", str(""), 
      "Hadoop FileSystem's base root path")
    ("swc.fs.hadoop.cfg.dyn", strs(), 
      "Dyn-config file")

    ("swc.fs.hadoop.namenode", strs(), 
      "Namenode Host + optional(:Port), muliple")
    ("swc.fs.hadoop.namenode.port", i16(), 
      "Namenode Port")
    ("swc.fs.hadoop.user", str(), 
      "Hadoop user")
    ("swc.fs.hadoop.handlers", i32(48), "Handlers for hadoop tasks")
  ;

  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.hadoop.cfg", ""),
    "swc.fs.hadoop.cfg.dyn"
  );
  return true;
}


SmartFdHadoop::Ptr SmartFdHadoop::make_ptr(const std::string &filepath, 
                                           uint32_t flags) {
  return std::make_shared<SmartFdHadoop>(filepath, flags);
}

SmartFdHadoop::Ptr SmartFdHadoop::make_ptr(SmartFd::Ptr &smart_fd) {
  return std::make_shared<SmartFdHadoop>(
    smart_fd->filepath(), smart_fd->flags(), 
    smart_fd->fd(), smart_fd->pos()
  );
}

SmartFdHadoop::SmartFdHadoop(const std::string &filepath, uint32_t flags,
                             int32_t fd, uint64_t pos)
                            : SmartFd(filepath, flags, fd, pos) { 
}
SmartFdHadoop::~SmartFdHadoop() { }



FileSystemHadoop::FileSystemHadoop() 
    : FileSystem(
        Env::Config::settings()->get_str("swc.fs.hadoop.path.root"),
        apply_hadoop()
      ),
      m_run(true), m_nxt_fd(0) {

  setup_connection();

}

FileSystemHadoop::~FileSystemHadoop() { 
  std::cout << " ~FileSystemHadoop() \n"; 
}

void FileSystemHadoop::setup_connection() {

  uint32_t tries=0; 
  while(m_run.load() && !initialize()) {
    SWC_LOGF(LOG_ERROR, 
      "FS-Hadoop, unable to initialize connection to hadoop, try=%d",
      ++tries);
  }
  
  //hdfsSetWorkingDirectory(m_filesystem, get_abspath("").c_str());
    
 
  // status check
  //char buffer[256];
  //hdfsGetWorkingDirectory(m_filesystem, buffer, 256);
  //SWC_LOGF(LOG_DEBUG, "FS-Hadoop, working Dir='%s'", buffer);
}

bool FileSystemHadoop::initialize() {
  auto settings = Env::Config::settings();
  
  hdfs::ConfigParser parser;
  if(!parser.LoadDefaultResources()){
    std::cerr << "hdfs::ConfigParser could not load default resources. " << std::endl;
    exit(EXIT_FAILURE);
  }
  auto stats = parser.ValidateResources();
  for(auto s : stats) {
    std::cout << s.first << "=" << s.second.ToString() << std::endl;
    if(!s.second.ok())
      (EXIT_FAILURE);
  }
  std::cout << " fs.defaultFS=" << parser.get_string_or("fs.defaultFS", "NONE") << "\n";

  hdfs::Options options;
  if(!parser.get_options(options)){
    std::cerr << "hdfs::ConfigParser could not load Options object. " << std::endl;
    exit(EXIT_FAILURE);
  }

  hdfs::IoService* io_service = hdfs::IoService::New();
  io_service->InitWorkers(settings->get_i32("swc.fs.hadoop.handlers"));

  m_filesystem = hdfs::FileSystem::New(
    io_service, 
    settings->get_str("swc.fs.hadoop.user", ""), 
    options
  );
  std::cout << "hdfs::FileSystem Initialized" << std::endl;

  hdfs::Status status;
  std::string port;
  if(settings->has("swc.fs.hadoop.namenode")) {
    for(auto& h : settings->get_strs("swc.fs.hadoop.namenode")) {
      port = std::to_string(settings->get_i16(
        "swc.fs.hadoop.namenode.port", 0));

      SWC_LOGF(LOG_DEBUG, "FS-Hadoop, Connecting to namenode=[%s]:%s", 
                            h.c_str(), port.c_str());
      
      status = m_filesystem->Connect(h, port);
      if(status.ok()) {
        hdfs::FsInfo fs_info;
        status =  m_filesystem->GetFsStats(fs_info);
        if(status.ok()) {
          SWC_LOGF(LOG_INFO, "%s", fs_info.str("FS-Hadoop").c_str());
          return true;
        }
      }
      std::cerr << "FS-Hadoop, Could not connect to " << h << ":" << port 
                << ". " << status.ToString() << std::endl;
    }

  } else {
    SWC_LOGF(LOG_DEBUG, "FS-Hadoop, connecting to default namenode=%s", options.defaultFS);
    status = m_filesystem->ConnectToDefaultFs();
    if(status.ok()) {
      hdfs::FsInfo fs_info;
      status =  m_filesystem->GetFsStats(fs_info);
      if(status.ok()) {
        SWC_LOGF(LOG_INFO, "%s", fs_info.str("FS-Hadoop").c_str());
        return true;
      }
    }
    std::cerr << "FS-Hadoop, Could not connect to " << options.defaultFS
              << ". " << status.ToString() << std::endl;
  }
    
  return m_filesystem != nullptr;
}


void FileSystemHadoop::stop() {
  m_run.store(false);
  if(m_filesystem != nullptr) {
    //hdfsDisconnect(m_filesystem);
    delete m_filesystem;
  }
}

Types::Fs FileSystemHadoop::get_type() {
  return Types::Fs::HADOOP;
};

const std::string FileSystemHadoop::to_string() {
  return format(
    "(type=HADOOP path_root=%s path_data=%s)", 
    path_root.c_str(),
    path_data.c_str()
  );
}


bool FileSystemHadoop::exists(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  bool state = false;//hdfsExists(m_filesystem, abspath.c_str()) == 0;
  err = errno==2 ? 0 : errno;
  SWC_LOGF(LOG_DEBUG, "exists state='%d' err='%d' path='%s'", 
            (int)state, err, abspath.c_str());
  return state;
}
  
void FileSystemHadoop::remove(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  if (true) { // hdfsDelete(m_filesystem, abspath.c_str(), false) == -1) {
    int tmperr = errno == 5 ? 2: errno;
    if(tmperr != 2) {
      err = tmperr;
      SWC_LOGF(LOG_ERROR, "remove('%s') failed - %s",
                           abspath.c_str(), strerror(err));
      return;
    }
  }
  SWC_LOGF(LOG_DEBUG, "remove('%s')", abspath.c_str());
}

size_t FileSystemHadoop::length(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
    
  size_t len = 0; 
  //hdfsFileInfo *fileInfo;

  if(true) {//(fileInfo = hdfsGetPathInfo(m_filesystem, abspath.c_str())) == 0) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "length('%s') failed - %s", 
              abspath.c_str(), strerror(err));
    return len;
  }
  len = 0;//fileInfo->mSize;
  //hdfsFreeFileInfo(fileInfo, 1);

  SWC_LOGF(LOG_DEBUG, "length len='%lld' path='%s'", len, abspath.c_str());
  return len;
}

void FileSystemHadoop::mkdirs(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", abspath.c_str());
  
  errno = 0;
  //hdfsCreateDirectory(m_filesystem, abspath.c_str());
  err = errno;
}

void FileSystemHadoop::readdir(int &err, const std::string &name, 
                               DirentList &results) {
  std::string abspath = get_abspath(name);
  SWC_LOGF(LOG_DEBUG, "Readdir dir='%s'", abspath.c_str());

  Dirent entry;
  //hdfsFileInfo *fileInfo;
  int numEntries;

  errno = 0;
  if (true) { // (fileInfo = hdfsListDirectory(
              //      m_filesystem, abspath.c_str(), &numEntries)) == 0) {
    if(errno) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "readdir('%s') failed - %s", 
                abspath.c_str(), strerror(errno)); 
    }
    return;
  }
  /*
  for (int i=0; i<numEntries; i++) {
    if (fileInfo[i].mName[0] == '.' || fileInfo[i].mName[0] == 0)
      continue;
    const char *ptr;
    if ((ptr = strrchr(fileInfo[i].mName, '/')))
      entry.name = (std::string)(ptr+1);
    else
	    entry.name = (std::string)fileInfo[i].mName;

    entry.length = fileInfo[i].mSize;
    entry.last_modification_time = fileInfo[i].mLastMod;
    entry.is_dir = fileInfo[i].mKind == kObjectKindDirectory;
    results.push_back(entry);      
  }
  */

  //hdfsFreeFileInfo(fileInfo, numEntries);
}

void FileSystemHadoop::rmdir(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  if (true) { // hdfsDelete(m_filesystem, abspath.c_str(), true) == -1) {
    err = errno == 5? 2: errno; // io error(not-exists)
    if(err != 2) {
      SWC_LOGF(LOG_ERROR, "rmdir('%s') failed - %s", 
                abspath.c_str(), strerror(errno));
      return;
    }
  }
  SWC_LOGF(LOG_DEBUG, "rmdir('%s')", abspath.c_str());
}

void FileSystemHadoop::rename(int &err, const std::string &from, 
                              const std::string &to)  {
  std::string abspath_from = get_abspath(from);
  std::string abspath_to = get_abspath(to);
  errno = 0;
  if (true) { // hdfsRename(m_filesystem, abspath_from.c_str(), abspath_to.c_str())
      // == -1) {
    SWC_LOGF(LOG_ERROR, "rename('%s' to '%s') failed - %s", 
              abspath_from.c_str(), abspath_to.c_str(), strerror(errno));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rename('%s' to '%s')", 
            abspath_from.c_str(), abspath_to.c_str());
}

SmartFdHadoop::Ptr FileSystemHadoop::get_fd(SmartFd::Ptr &smartfd){
  auto hd_fd = std::dynamic_pointer_cast<SmartFdHadoop>(smartfd);
  if(!hd_fd){
    hd_fd = SmartFdHadoop::make_ptr(smartfd);
    smartfd = std::static_pointer_cast<SmartFd>(hd_fd);
  }
  return hd_fd;
}

void FileSystemHadoop::create(int &err, SmartFd::Ptr &smartfd, 
                              int32_t bufsz, uint8_t replication, 
                              int64_t blksz) {

  std::string abspath = get_abspath(smartfd->filepath());
  SWC_LOGF(LOG_DEBUG, "create %s bufsz=%d replication=%d blksz=%lld",
            smartfd->to_string().c_str(), 
            bufsz, replication, (Lld)blksz);

  int oflags = O_WRONLY;
  if((smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE) == 0)
    oflags |= O_APPEND;

  if (bufsz == -1)
    bufsz = 0;
  blksz = blksz < 512 ? 0 : (blksz/512+1)*512;

  auto hadoop_fd = get_fd(smartfd);
  /* Open the file */
  if (true) { // (hadoop_fd->file = hdfsOpenFile(m_filesystem, abspath.c_str(), oflags, 
      //                                bufsz, replication, blksz)) == 0) {
    err = errno;
    hadoop_fd->fd(-1);
    SWC_LOGF(LOG_ERROR, "create failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
                
    if(err == EACCES || err == ENOENT)
      err == Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err == Error::FS_PERMISSION_DENIED;
    return;
  }

  hadoop_fd->fd(++m_nxt_fd);
  SWC_LOGF(LOG_DEBUG, "created %s bufsz=%d replication=%d blksz=%lld",
            smartfd->to_string().c_str(), 
            bufsz, replication, (Lld)blksz);
}

void FileSystemHadoop::open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz) {

  std::string abspath = get_abspath(smartfd->filepath());
  SWC_LOGF(LOG_DEBUG, "open %s bufsz=%d",
            smartfd->to_string().c_str(), bufsz);

  int oflags = O_RDONLY;

  auto hadoop_fd = get_fd(smartfd);
  /* Open the file */
  if (true) { // (hadoop_fd->file = hdfsOpenFile(m_filesystem, abspath.c_str(), oflags, 
        //                              bufsz==-1? 0 : bufsz, 0, 0)) == 0) {
    err = errno;
    hadoop_fd->fd(-1);
    SWC_LOGF(LOG_ERROR, "open failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
                
    if(err == EACCES || err == ENOENT)
      err == Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err == Error::FS_PERMISSION_DENIED;
    return;
  }

  hadoop_fd->fd(++m_nxt_fd);
  SWC_LOGF(LOG_DEBUG, "opened %s", smartfd->to_string().c_str());
}
  
size_t FileSystemHadoop::read(int &err, SmartFd::Ptr &smartfd, 
              void *dst, size_t amount) {

  auto hadoop_fd = get_fd(smartfd);
    
  SWC_LOGF(LOG_DEBUG, "read %s amount=%lld file-%lld", 
            hadoop_fd->to_string().c_str(), 
            amount, (size_t) hadoop_fd->file);
  ssize_t nread = 0;
  errno = 0;

  /* 
  uint64_t offset;
  if ((offset = (uint64_t)hdfsTell(m_filesystem, hadoop_fd->file))
                == (uint64_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "read, tell failed: %d(%s), %s offset=%llu", 
              errno, strerror(errno), smartfd->to_string().c_str(), offset);
    return nread;
  }
  */
    
  nread = -1; // (ssize_t)hdfsRead(m_filesystem, hadoop_fd->file, dst, (tSize)amount);
  if (nread == -1) {
    nread = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "read failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  } else {
    if(nread != amount)
      err = Error::FS_EOF;
    hadoop_fd->pos(hadoop_fd->pos()+nread);
    SWC_LOGF(LOG_DEBUG, "read(ed) %s amount=%llu eof=%d", 
              smartfd->to_string().c_str(), nread, err == Error::FS_EOF);
  }
  return nread;
}

  
size_t FileSystemHadoop::pread(int &err, SmartFd::Ptr &smartfd, 
                               uint64_t offset, void *dst, size_t amount) {

  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "pread %s offset=%llu amount=%lld file-%lld", 
            hadoop_fd->to_string().c_str(),
            offset, amount, (size_t) hadoop_fd->file);

  errno = 0;
  ssize_t nread = -1; // (ssize_t)hdfsPread(
    //m_filesystem, hadoop_fd->file, (tOffset)offset, dst, (tSize)amount);
  if (nread == -1) {
    nread = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "pread failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  } else {
    if(nread != amount)
      err = Error::FS_EOF;
    hadoop_fd->pos(offset+nread);
    SWC_LOGF(LOG_DEBUG, "pread(ed) %s amount=%llu eof=%d", 
               smartfd->to_string().c_str(), nread, err == Error::FS_EOF);
  }
  return nread;
}

size_t FileSystemHadoop::append(int &err, SmartFd::Ptr &smartfd, 
                                StaticBuffer &buffer, Flags flags) {

  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "append %s amount=%lld flags=%d", 
            hadoop_fd->to_string().c_str(), buffer.size, flags);
    
  ssize_t nwritten = 0;
  errno = 0;
  /* 
  uint64_t offset;
  if ((offset = (uint64_t)hdfsTell(m_filesystem, hadoop_fd->file))
          == (uint64_t)-1) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "write, tell failed: %d(%s), %s offset=%llu", 
              errno, strerror(errno), smartfd->to_string().c_str(), offset);
    return nwritten;
  }
  */

  if (true) { // (nwritten = (ssize_t)hdfsWrite(m_filesystem, hadoop_fd->file, 
       //                      buffer.base, (tSize)buffer.size)) == -1) {
    nwritten = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "write failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
    return nwritten;
  }
  hadoop_fd->pos(smartfd->pos()+nwritten);

  if (flags == Flags::FLUSH || flags == Flags::SYNC) {
    if (true) { // hdfsFlush(m_filesystem, hadoop_fd->file) != Error::OK) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "write, fsync failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return nwritten;
    }
  }
  SWC_LOGF(LOG_DEBUG, "appended %s amount=%llu", 
            smartfd->to_string().c_str(), nwritten);
  return nwritten;
}

void FileSystemHadoop::seek(int &err, SmartFd::Ptr &smartfd, size_t offset) {

  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "seek %s offset=%llu", 
            hadoop_fd->to_string().c_str(), offset);
    
  errno = 0;
  uint64_t at = -1; // hdfsSeek(m_filesystem, hadoop_fd->file, (tOffset)offset); 
  if (at == (uint64_t)-1 || at != Error::OK || errno != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "seek failed - at=%llu %d(%s) %s", 
              at, err, strerror(errno), smartfd->to_string().c_str());
    return;
  }
  hadoop_fd->pos(offset);
}

void FileSystemHadoop::flush(int &err, SmartFd::Ptr &smartfd) {
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "flush %s", hadoop_fd->to_string().c_str());

  if (true) { // hdfsHFlush(m_filesystem, hadoop_fd->file) != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "flush failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  }
}

void FileSystemHadoop::sync(int &err, SmartFd::Ptr &smartfd) {
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "sync %s", hadoop_fd->to_string().c_str());

  if (true) { // hdfsHSync(m_filesystem, hadoop_fd->file) != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "flush failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  }
}

void FileSystemHadoop::close(int &err, SmartFd::Ptr &smartfd) {
    
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "close %s", hadoop_fd->to_string().c_str());

  if(hadoop_fd->file) {
    if(true) { // hdfsCloseFile(m_filesystem, hadoop_fd->file) != 0) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "close, failed: %d(%s), %s", 
                 errno, strerror(errno), smartfd->to_string().c_str());
    }
  } else 
    err = EBADR;
  smartfd->fd(-1);
  smartfd->pos(0);
}







}} // namespace SWC



extern "C" { 
SWC::FS::FileSystem* fs_make_new_hadoop() {
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemHadoop());
};
void fs_apply_cfg_hadoop(SWC::Env::Config::Ptr env) {
  SWC::Env::Config::set(env);
};
}
