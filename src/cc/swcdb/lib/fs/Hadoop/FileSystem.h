/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Hadoop_FileSystem_h
#define swc_lib_fs_Hadoop_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"

#include <hdfs.h>

namespace SWC{ namespace FS {

bool apply_hadoop();

struct SmartFdHadoop;
typedef std::shared_ptr<SmartFdHadoop> SmartFdHadoopPtr;

struct SmartFdHadoop : public SmartFd {
  public:
  
  static SmartFdHadoopPtr make_ptr(const String &filepath, uint32_t flags){
    return std::make_shared<SmartFdHadoop>(filepath, flags);
  }

  static SmartFdHadoopPtr make_ptr(SmartFdPtr &smart_fd){
    return std::make_shared<SmartFdHadoop>(
      smart_fd->filepath(), smart_fd->flags(), 
      smart_fd->fd(), smart_fd->pos()
    );
  }

  SmartFdHadoop(const String &filepath, uint32_t flags,
                int32_t fd=-1, uint64_t pos=0)
               : SmartFd(filepath, flags, fd, pos) { }
  virtual ~SmartFdHadoop() { }

  hdfsFile file = 0;
};



class FileSystemHadoop: public FileSystem {
  public:

  FileSystemHadoop() 
    : FileSystem(
        EnvConfig::settings()->get<String>("swc.fs.hadoop.path.root"),
        apply_hadoop()
      ),
      m_run(true), m_nxt_fd(0)
  { 
    setup_connection();
  }

  void setup_connection(){

    uint32_t tries=0; 
    while(m_run.load() && !initialize()) {
      HT_ERRORF("FS-Hadoop, unable to initialize connection to hadoop, try=%d", ++tries);
    }
    hdfsSetWorkingDirectory(m_filesystem, get_abspath("").c_str());
    
    /* 
    char* host;
    int32_t port;
    hdfsConfGetStr("hdfs.namenode.host", &host);
    hdfsConfGetInt("hdfs.namenode.port", &port);
    HT_INFOF("FS-Hadoop, connected to namenode=[%s]:%d", host, port);
    hdfsConfStrFree(host);
    */

    // status check
    char buffer[256];
    hdfsGetWorkingDirectory(m_filesystem, buffer, 256);
    HT_DEBUGF("FS-Hadoop, working Dir='%s'", buffer);

    int64_t sz_used = hdfsGetUsed(m_filesystem); 
    HT_INFOF("Non DFS Used bytes: %ld", sz_used);
    sz_used = hdfsGetCapacity(m_filesystem); 
    HT_INFOF("Configured Capacity bytes: %ld", sz_used);
  }

  bool initialize(){
    
    if (EnvConfig::settings()->has("swc.fs.hadoop.namenode")) {
      for(auto h : EnvConfig::settings()->get<Strings>("swc.fs.hadoop.namenode")){
	      hdfsBuilder* bld = hdfsNewBuilder();
        hdfsBuilderSetNameNode(bld, h.c_str());

        if (EnvConfig::settings()->has("swc.fs.hadoop.namenode.port")) 
          hdfsBuilderSetNameNodePort(
            bld, EnvConfig::settings()->get<int32_t>("swc.fs.hadoop.namenode.port"));

        if (EnvConfig::settings()->has("swc.fs.hadoop.user")) 
          hdfsBuilderSetUserName(
            bld, EnvConfig::settings()->get<String>("swc.fs.hadoop.user").c_str());
        
        m_filesystem = hdfsBuilderConnect(bld);
        HT_DEBUGF("Connecting to namenode=%s", h.c_str());

        if(m_filesystem != nullptr) return true;
      }

    } else {
	    hdfsBuilder* bld = hdfsNewBuilder();
       //"default" > read hadoop config from LIBHDFS3_CONF=path
      hdfsBuilderSetNameNode(bld, "default"); 

      m_filesystem = hdfsBuilderConnect(bld);
      
      char* value;
      hdfsConfGetStr("fs.defaultFS", &value);
      HT_DEBUGF("FS-Hadoop, connecting to default namenode=%s", value);
    }
    
    return m_filesystem != nullptr;
  }

  virtual ~FileSystemHadoop(){ std::cout << " ~FileSystemHadoop() \n"; }

  void stop() override {
    m_run.store(false);
    if(m_filesystem != nullptr)
      hdfsDisconnect(m_filesystem);
  }

  Types::Fs get_type() override;

  const std::string to_string() override {
    return format(
      "(type=HADOOP, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }




  bool exists(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    bool state = hdfsExists(m_filesystem, abspath.c_str()) == 0;
    err = errno==2?0:errno;
    HT_DEBUGF("exists state='%d' err='%d' path='%s'", 
              (int)state, err, abspath.c_str());
    return state;
  }
  
  void remove(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    if (hdfsDelete(m_filesystem, abspath.c_str(), false) == -1) {
      err = errno == 5? 2: errno;
      HT_ERRORF("remove('%s') failed - %s", abspath.c_str(), strerror(err));
      return;
    }
    HT_DEBUGF("remove('%s')", abspath.c_str());
  }

  size_t length(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    
    size_t len = 0; 
    hdfsFileInfo *fileInfo;

    if((fileInfo = hdfsGetPathInfo(m_filesystem, abspath.c_str())) == 0) {
      err = errno;
      HT_ERRORF("length('%s') failed - %s", abspath.c_str(), strerror(err));
      return len;
    }
    len = fileInfo->mSize;
    hdfsFreeFileInfo(fileInfo, 1);

    HT_DEBUGF("length len='%d' path='%s'", len, abspath.c_str());
    return len;
  }

  void mkdirs(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("mkdirs path='%s'", abspath.c_str());
  
    errno = 0;
    hdfsCreateDirectory(m_filesystem, abspath.c_str());
    err = errno;
  }

  void readdir(int &err, const String &name, DirentList &results) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("Readdir dir='%s'", abspath.c_str());

    Dirent entry;
    hdfsFileInfo *fileInfo;
    int numEntries;

    errno = 0;
    if ((fileInfo = hdfsListDirectory(
                      m_filesystem, abspath.c_str(), &numEntries)) == 0) {
      if(errno != 0) {
        err = errno;
        HT_ERRORF("readdir('%s') failed - %s", abspath.c_str(), strerror(errno)); 
      }
      return;
    }

    for (int i=0; i<numEntries; i++) {
      if (fileInfo[i].mName[0] == '.' || fileInfo[i].mName[0] == 0)
        continue;
      const char *ptr;
      if ((ptr = strrchr(fileInfo[i].mName, '/')))
	      entry.name = (String)(ptr+1);
      else
	      entry.name = (String)fileInfo[i].mName;

      entry.length = fileInfo[i].mSize;
      entry.last_modification_time = fileInfo[i].mLastMod;
      entry.is_dir = fileInfo[i].mKind == kObjectKindDirectory;
      results.push_back(entry);      
    }

    hdfsFreeFileInfo(fileInfo, numEntries);
  }

  void rmdir(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    errno = 0;
    if (hdfsDelete(m_filesystem, abspath.c_str(), true) == -1) {
      err = errno;
      HT_ERRORF("rmdir('%s') failed - %s", abspath.c_str(), strerror(errno));
      return;
    }
    HT_DEBUGF("rmdir('%s')", abspath.c_str());
  }

  SmartFdHadoopPtr get_fd(SmartFdPtr &smartfd){
    SmartFdHadoopPtr hd_fd = std::dynamic_pointer_cast<SmartFdHadoop>(smartfd);
    if(!hd_fd){
      hd_fd = SmartFdHadoop::make_ptr(smartfd);
      smartfd = std::static_pointer_cast<SmartFd>(hd_fd);
    }
    return hd_fd;
  }

  void create(int &err, SmartFdPtr &smartfd, 
              int32_t bufsz, int32_t replication, int64_t blksz) override {

    std::string abspath = get_abspath(smartfd->filepath());
    HT_DEBUGF("create %s bufsz=%d replication=%d blksz=%lld",
              smartfd->to_string().c_str(), 
              bufsz, (int)replication, (Lld)blksz);

    int oflags = O_WRONLY;
    if((smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE) == 0)
      oflags |= O_APPEND;

    if (bufsz == -1)
      bufsz = 0;
    if (replication == -1)
      replication = 0;
    if (blksz == -1)
      blksz = 0;

    SmartFdHadoopPtr hadoop_fd = get_fd(smartfd);
    /* Open the file */
    if ((hadoop_fd->file = hdfsOpenFile(m_filesystem, abspath.c_str(), oflags, 
                                        bufsz, replication, blksz)) == 0) {
      err = errno;
      hadoop_fd->fd(-1);
      HT_ERRORF("create failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return;
    }

    hadoop_fd->fd(++m_nxt_fd);
    HT_DEBUGF("created %s bufsz=%d replication=%d blksz=%lld",
              smartfd->to_string().c_str(), 
              bufsz, (int)replication, (Lld)blksz);
  }

  void open(int &err, SmartFdPtr &smartfd, int32_t bufsz = -1) override {

    std::string abspath = get_abspath(smartfd->filepath());
    HT_DEBUGF("open %s bufsz=%d",
              smartfd->to_string().c_str(), bufsz);

    int oflags = O_RDONLY;

    SmartFdHadoopPtr hadoop_fd = get_fd(smartfd);
    /* Open the file */
    if ((hadoop_fd->file = hdfsOpenFile(m_filesystem, abspath.c_str(), oflags, 
                                        bufsz==-1? 0 : bufsz, 0, 0)) == 0) {
      err = errno;
      hadoop_fd->fd(-1);
      HT_ERRORF("open failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return;
    }

    hadoop_fd->fd(++m_nxt_fd);
    HT_DEBUGF("opened %s", smartfd->to_string().c_str());
  }
  
  size_t read(int &err, SmartFdPtr &smartfd, 
              void *dst, size_t amount) override {

    SmartFdHadoopPtr hadoop_fd = get_fd(smartfd);
    
    HT_DEBUGF("read %s amount=%d file-%lld", hadoop_fd->to_string().c_str(), 
              amount, (size_t) hadoop_fd->file);
    ssize_t nread = 0;
    errno = 0;

    /* 
    uint64_t offset;
    if ((offset = (uint64_t)hdfsTell(m_filesystem, hadoop_fd->file)) == (uint64_t)-1) {
      err = errno;
      HT_ERRORF("read, tell failed: %d(%s), %s offset=%d", 
                errno, strerror(errno), smartfd->to_string().c_str(), offset);
      return nread;
    }
    */
    
    nread = (ssize_t)hdfsRead(m_filesystem, hadoop_fd->file, dst, (tSize)amount);
    if (nread == -1) {
      nread = 0;
      err = errno;
      HT_ERRORF("read failed: %d(%s), %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
    } else {

      hadoop_fd->pos(nread);
      HT_DEBUGF("read(ed) %s amount=%d", smartfd->to_string().c_str(), nread);
    }
    return nread;
  }

  size_t append(int &err, SmartFdPtr &smartfd, 
                StaticBuffer &buffer, Flags flags) override {
    
    SmartFdHadoopPtr hadoop_fd = get_fd(smartfd);
    HT_DEBUGF("append %s amount=%d flags=%d", 
              hadoop_fd->to_string().c_str(), buffer.size, flags);
    
    ssize_t nwritten = 0;
    errno = 0;
    /* 
    uint64_t offset;
    if ((offset = (uint64_t)hdfsTell(m_filesystem, hadoop_fd->file)) == (uint64_t)-1) {
      err = errno;
      HT_ERRORF("write, tell failed: %d(%s), %s offset=%d", 
                errno, strerror(errno), smartfd->to_string().c_str(), offset);
      return nwritten;
    }
    */

    if ((nwritten = (ssize_t)hdfsWrite(m_filesystem, hadoop_fd->file, 
                             buffer.base, (tSize)buffer.size)) == -1) {
      nwritten = 0;
      err = errno;
      HT_ERRORF("write failed: %d(%s),  %s", 
                errno, strerror(errno), smartfd->to_string().c_str());
      return nwritten;
    }
    hadoop_fd->pos(nwritten);

    if (flags == Flags::FLUSH || flags == Flags::SYNC) {
      if (hdfsFlush(m_filesystem, hadoop_fd->file)) {
        err = errno;
        HT_ERRORF("write, fsync failed: %d(%s),  %s", 
                  errno, strerror(errno), smartfd->to_string().c_str());
        return nwritten;
      }
    }
    HT_DEBUGF("appended %s amount=%d", 
              smartfd->to_string().c_str(), nwritten);
    return nwritten;
  }

  void seek(int &err, SmartFdPtr &smartfd, size_t offset) override {

    SmartFdHadoopPtr hadoop_fd = get_fd(smartfd);
    HT_DEBUGF("seek %s offset=%d", hadoop_fd->to_string().c_str(), offset);
    
    errno = 0;
    uint64_t at = hdfsSeek(m_filesystem, hadoop_fd->file, (tOffset)offset); 
    if (at == (uint64_t)-1 || at != Error::OK || errno != Error::OK) {
      err = errno;
      HT_ERRORF("seek failed - at=%d %d(%s) %s", 
                at, err, strerror(errno), smartfd->to_string().c_str());
      return;
    }
    hadoop_fd->pos(offset);
  }

  void close(int &err, SmartFdPtr &smartfd) {
    
    SmartFdHadoopPtr hadoop_fd = get_fd(smartfd);
    HT_DEBUGF("close %s", hadoop_fd->to_string().c_str());

    if(hadoop_fd->file != 0 
       && hdfsCloseFile(m_filesystem, hadoop_fd->file) != 0) {
      HT_ERRORF("close, failed: %d(%s),  %s", 
                 errno, strerror(errno), smartfd->to_string().c_str());
    }
    smartfd->fd(-1);
    smartfd->pos(0);
  }

  private:
	hdfsFS                m_filesystem;
  std::atomic<bool>     m_run;
  std::atomic<int32_t>  m_nxt_fd;
};


}}



#endif  // swc_lib_fs_Hadoop_FileSystem_h