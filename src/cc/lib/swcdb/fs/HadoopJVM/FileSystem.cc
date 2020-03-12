/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#include "swcdb/fs/HadoopJVM/FileSystem.h"

#include <iostream>


namespace SWC{ namespace FS {

 
bool apply_hadoop_jvm() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.hadoop_jvm.path.root", str(""), 
      "HadoopJVM FileSystem's base root path")
    ("swc.fs.hadoop_jvm.cfg.dyn", strs(), 
      "Dyn-config file")

    ("swc.fs.hadoop_jvm.namenode", strs(), 
      "Namenode Host + optional(:Port), muliple")
    ("swc.fs.hadoop_jvm.namenode.port", i16(), 
      "Namenode Port")
    ("swc.fs.hadoop_jvm.user", str(), 
      "HadoopJVM user")
  ;

  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.hadoop_jvm.cfg", ""),
    "swc.fs.hadoop_jvm.cfg.dyn"
  );
  return true;
}


SmartFdHadoopJVM::Ptr SmartFdHadoopJVM::make_ptr(const std::string &filepath, 
                                           uint32_t flags) {
  return std::make_shared<SmartFdHadoopJVM>(filepath, flags);
}

SmartFdHadoopJVM::Ptr SmartFdHadoopJVM::make_ptr(SmartFd::Ptr &smart_fd) {
  return std::make_shared<SmartFdHadoopJVM>(
    smart_fd->filepath(), smart_fd->flags(), 
    smart_fd->fd(), smart_fd->pos()
  );
}

SmartFdHadoopJVM::SmartFdHadoopJVM(const std::string &filepath, uint32_t flags,
                             int32_t fd, uint64_t pos)
                            : SmartFd(filepath, flags, fd, pos) { 
}
SmartFdHadoopJVM::~SmartFdHadoopJVM() { }



FileSystemHadoopJVM::FileSystemHadoopJVM() 
    : FileSystem(
        Env::Config::settings()->get_str("swc.fs.hadoop_jvm.path.root"),
        apply_hadoop_jvm()
      ),
      m_run(true), m_nxt_fd(0) { 
  setup_connection();
}

void FileSystemHadoopJVM::setup_connection() {

  uint32_t tries=0; 
  while(m_run.load() && !initialize()) {
    SWC_LOGF(LOG_ERROR, 
      "FS-HadoopJVM, unable to initialize connection to hadoop_jvm, try=%d",
      ++tries);
  }
  
  hdfsSetWorkingDirectory(m_filesystem, get_abspath("").c_str());
    
  /* 
  char* host;    
  uint16_t port;
  hdfsConfGetStr("hdfs.namenode.host", &host);
  hdfsConfGetInt("hdfs.namenode.port", &port);
  SWC_LOGF(LOG_INFO, "FS-HadoopJV<, connected to namenode=[%s]:%d", host, port);
  hdfsConfStrFree(host);
  */

  // status check
  char buffer[256];
  hdfsGetWorkingDirectory(m_filesystem, buffer, 256);
  SWC_LOGF(LOG_DEBUG, "FS-HadoopJVM, working Dir='%s'", buffer);
}

bool FileSystemHadoopJVM::initialize() {
  auto settings = Env::Config::settings();

  if (settings->has("swc.fs.hadoop_jvm.namenode")) {
    for(auto& h : settings->get_strs("swc.fs.hadoop_jvm.namenode")){
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
        
      m_filesystem = hdfsBuilderConnect(bld);
      SWC_LOGF(LOG_DEBUG, "Connecting to namenode=%s", h.c_str());
      // java.lang.IllegalArgumentException: java.net.UnknownHostException:
      if(m_filesystem != nullptr) {
        errno = Error::OK;
        // check status, namenode need to be active
        int64_t sz_used = hdfsGetUsed(m_filesystem); 
        if(sz_used == -1) {
          SWC_LOGF(LOG_ERROR, "hdfsGetUsed('%s') failed - %d(%s)", 
                                h.c_str(), errno, strerror(errno));
          continue;
        }
        SWC_LOGF(LOG_INFO, "Non DFS Used bytes: %ld", sz_used);
          
        sz_used = hdfsGetCapacity(m_filesystem); 
        if(sz_used == -1) {
          SWC_LOGF(LOG_ERROR, "hdfsGetCapacity('%s') failed - %d(%s)", 
                                h.c_str(), errno, strerror(errno));
          continue;
        }
        SWC_LOGF(LOG_INFO, "Configured Capacity bytes: %ld", sz_used);

        return true;
      }
    }

  } else {
	  hdfsBuilder* bld = hdfsNewBuilder();
     //"default" > read hadoop_jvm config from LIBHDFS3_CONF=path
    hdfsBuilderSetNameNode(bld, "default"); 

    m_filesystem = hdfsBuilderConnect(bld);
      
    char* value;
    hdfsConfGetStr("fs.defaultFS", &value);
    SWC_LOGF(LOG_DEBUG, "FS-HadoopJVM, connecting to default namenode=%s", value);
  }
    
  return m_filesystem != nullptr;
}

FileSystemHadoopJVM::~FileSystemHadoopJVM() { }

void FileSystemHadoopJVM::stop() {
  m_run.store(false);
  if(m_filesystem != nullptr)
    hdfsDisconnect(m_filesystem);
}

Types::Fs FileSystemHadoopJVM::get_type() {
  return Types::Fs::HADOOP_JVM;
};

const std::string FileSystemHadoopJVM::to_string() {
  return format(
    "(type=HADOOP_JVM path_root=%s path_data=%s)", 
    path_root.c_str(),
    path_data.c_str()
  );
}



bool FileSystemHadoopJVM::exists(int &err, const std::string &name) {
  // org.apache.hadoop.ipc.StandbyException
  std::string abspath = get_abspath(name);
  errno = 0;
  bool state = hdfsExists(m_filesystem, abspath.c_str()) == 0;
  err = errno==2 ? 0 : errno;
  SWC_LOGF(LOG_DEBUG, "exists state='%d' err='%d' path='%s'", 
            (int)state, err, abspath.c_str());
  return state;
}
  
void FileSystemHadoopJVM::remove(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  if (hdfsDelete(m_filesystem, abspath.c_str(), false) == -1) {
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

size_t FileSystemHadoopJVM::length(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
    
  size_t len = 0; 
  hdfsFileInfo *fileInfo;

  if((fileInfo = hdfsGetPathInfo(m_filesystem, abspath.c_str())) == 0) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "length('%s') failed - %s", 
              abspath.c_str(), strerror(err));
    return len;
  }
  len = fileInfo->mSize;
  hdfsFreeFileInfo(fileInfo, 1);

  SWC_LOGF(LOG_DEBUG, "length len='%lld' path='%s'", len, abspath.c_str());
  return len;
}

void FileSystemHadoopJVM::mkdirs(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", abspath.c_str());
  
  errno = 0;
  hdfsCreateDirectory(m_filesystem, abspath.c_str());
  err = errno;
}

void FileSystemHadoopJVM::readdir(int &err, const std::string &name, 
                               DirentList &results) {
  std::string abspath = get_abspath(name);
  SWC_LOGF(LOG_DEBUG, "Readdir dir='%s'", abspath.c_str());

  hdfsFileInfo *fileInfo;
  int numEntries;

  errno = 0;
  if ((fileInfo = hdfsListDirectory(
                    m_filesystem, abspath.c_str(), &numEntries)) == 0) {
    if(errno) {
      err = errno;
      SWC_LOGF(LOG_ERROR, "readdir('%s') failed - %s", 
                abspath.c_str(), strerror(errno)); 
    }
    return;
  }

  for (int i=0; i<numEntries; i++) {
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

void FileSystemHadoopJVM::rmdir(int &err, const std::string &name) {
  std::string abspath = get_abspath(name);
  errno = 0;
  if (hdfsDelete(m_filesystem, abspath.c_str(), true) == -1) {
    err = errno == 5? 2: errno; // io error(not-exists)
    if(err != 2) {
      SWC_LOGF(LOG_ERROR, "rmdir('%s') failed - %s", 
                abspath.c_str(), strerror(errno));
      return;
    }
  }
  SWC_LOGF(LOG_DEBUG, "rmdir('%s')", abspath.c_str());
}

void FileSystemHadoopJVM::rename(int &err, const std::string &from, 
                              const std::string &to)  {
  std::string abspath_from = get_abspath(from);
  std::string abspath_to = get_abspath(to);
  errno = 0;
  if (hdfsRename(m_filesystem, abspath_from.c_str(), abspath_to.c_str())
       == -1) {
    SWC_LOGF(LOG_ERROR, "rename('%s' to '%s') failed - %s", 
              abspath_from.c_str(), abspath_to.c_str(), strerror(errno));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rename('%s' to '%s')", 
            abspath_from.c_str(), abspath_to.c_str());
}

SmartFdHadoopJVM::Ptr FileSystemHadoopJVM::get_fd(SmartFd::Ptr &smartfd){
  auto hd_fd = std::dynamic_pointer_cast<SmartFdHadoopJVM>(smartfd);
  if(!hd_fd){
    hd_fd = SmartFdHadoopJVM::make_ptr(smartfd);
    smartfd = std::static_pointer_cast<SmartFd>(hd_fd);
  }
  return hd_fd;
}

void FileSystemHadoopJVM::create(int &err, SmartFd::Ptr &smartfd, 
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
  if ((hadoop_fd->file = hdfsOpenFile(m_filesystem, abspath.c_str(), oflags, 
                                      bufsz, replication, blksz)) == 0) {
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

void FileSystemHadoopJVM::open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz) {

  std::string abspath = get_abspath(smartfd->filepath());
  SWC_LOGF(LOG_DEBUG, "open %s bufsz=%d",
            smartfd->to_string().c_str(), bufsz);

  int oflags = O_RDONLY;

  auto hadoop_fd = get_fd(smartfd);
  /* Open the file */
  if ((hadoop_fd->file = hdfsOpenFile(m_filesystem, abspath.c_str(), oflags, 
                                      bufsz==-1? 0 : bufsz, 0, 0)) == 0) {
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
  
size_t FileSystemHadoopJVM::read(int &err, SmartFd::Ptr &smartfd, 
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
    
  nread = (ssize_t)hdfsRead(m_filesystem, hadoop_fd->file, dst, (tSize)amount);
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

  
size_t FileSystemHadoopJVM::pread(int &err, SmartFd::Ptr &smartfd, 
                               uint64_t offset, void *dst, size_t amount) {

  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "pread %s offset=%llu amount=%lld file-%lld", 
            hadoop_fd->to_string().c_str(),
            offset, amount, (size_t) hadoop_fd->file);

  errno = 0;
  ssize_t nread = (ssize_t)hdfsPread(
    m_filesystem, hadoop_fd->file, (tOffset)offset, dst, (tSize)amount);
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

size_t FileSystemHadoopJVM::append(int &err, SmartFd::Ptr &smartfd, 
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

  if ((nwritten = (ssize_t)hdfsWrite(m_filesystem, hadoop_fd->file, 
                             buffer.base, (tSize)buffer.size)) == -1) {
    nwritten = 0;
    err = errno;
    SWC_LOGF(LOG_ERROR, "write failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
    return nwritten;
  }
  hadoop_fd->pos(smartfd->pos()+nwritten);

  if (flags == Flags::FLUSH || flags == Flags::SYNC) {
    if (hdfsFlush(m_filesystem, hadoop_fd->file) != Error::OK) {
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

void FileSystemHadoopJVM::seek(int &err, SmartFd::Ptr &smartfd, size_t offset) {

  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "seek %s offset=%llu", 
            hadoop_fd->to_string().c_str(), offset);
    
  errno = 0;
  uint64_t at = hdfsSeek(m_filesystem, hadoop_fd->file, (tOffset)offset); 
  if (at == (uint64_t)-1 || at != Error::OK || errno != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "seek failed - at=%llu %d(%s) %s", 
              at, err, strerror(errno), smartfd->to_string().c_str());
    return;
  }
  hadoop_fd->pos(offset);
}

void FileSystemHadoopJVM::flush(int &err, SmartFd::Ptr &smartfd) {
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "flush %s", hadoop_fd->to_string().c_str());

  if (hdfsHFlush(m_filesystem, hadoop_fd->file) != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "flush failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  }
}

void FileSystemHadoopJVM::sync(int &err, SmartFd::Ptr &smartfd) {
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "sync %s", hadoop_fd->to_string().c_str());

  if (hdfsHSync(m_filesystem, hadoop_fd->file) != Error::OK) {
    err = errno;
    SWC_LOGF(LOG_ERROR, "flush failed: %d(%s), %s", 
              errno, strerror(errno), smartfd->to_string().c_str());
  }
}

void FileSystemHadoopJVM::close(int &err, SmartFd::Ptr &smartfd) {
    
  auto hadoop_fd = get_fd(smartfd);
  SWC_LOGF(LOG_DEBUG, "close %s", hadoop_fd->to_string().c_str());

  if(hadoop_fd->file) {
    if(hdfsCloseFile(m_filesystem, hadoop_fd->file) != 0) {
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
SWC::FS::FileSystem* fs_make_new_hadoop_jvm() {
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemHadoopJVM());
};
void fs_apply_cfg_hadoop_jvm(SWC::Env::Config::Ptr env) {
  SWC::Env::Config::set(env);
};
}
