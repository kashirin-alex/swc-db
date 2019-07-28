/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Hadoop_FileSystem_h
#define swc_lib_fs_Hadoop_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"

#include <hdfs.h>

namespace SWC{ namespace FS {

void apply_hadoop(Config::SettingsPtr settings);


class FileSystemHadoop: public FileSystem {
  public:

  FileSystemHadoop(
    Config::SettingsPtr settings) 
    : FileSystem(settings, settings->get<String>("swc.fs.hadoop.path.root")),
      m_run(true)
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
    
    if (settings->has("swc.fs.hadoop.namenode")) {
      for(auto h : settings->get<Strings>("swc.fs.hadoop.namenode")){
	      hdfsBuilder* bld = hdfsNewBuilder();
        hdfsBuilderSetNameNode(bld, h.c_str());

        if (settings->has("swc.fs.hadoop.namenode.port")) 
          hdfsBuilderSetNameNodePort(
            bld, settings->get<int32_t>("swc.fs.hadoop.namenode.port"));

        if (settings->has("swc.fs.hadoop.user")) 
          hdfsBuilderSetUserName(
            bld, settings->get<String>("swc.fs.hadoop.user").c_str());
        
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
    HT_DEBUGF("exists path='%s'", abspath.c_str());

    errno = 0;
    bool state = hdfsExists(m_filesystem, abspath.c_str()) == 0;
    HT_DEBUGF("exists state='%d'", (int)state);
    err = errno;
    return state;
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



  void create(int &err, SmartFdPtr &smartfd, int32_t bufsz,
              int32_t replication, int64_t blksz) override {

  };

	hdfsFS            m_filesystem;
  std::atomic<bool> m_run;
};


}}



#endif  // swc_lib_fs_Hadoop_FileSystem_h