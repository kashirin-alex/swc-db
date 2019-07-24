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
    : FileSystem(settings),
      path_root(
        normalize_pathname(settings->get<String>("swc.fs.hadoop.path.root")))
  { 

    
    m_builder = hdfsNewBuilder();
    //hdfsBuilderConfSetStr(m_builder, const char *key, const char *val);
    //m_namenode_host = props->get_str("FsBroker.Hdfs.NameNode.Host");
    //m_namenode_port = props->get_i16("FsBroker.Hdfs.NameNode.Port");
    hdfsBuilderSetNameNode(m_builder, "default");  //"default" > read from hadoop XML config from LIBHDFS3_CONF=path
    //hdfsBuilderSetNameNodePort(m_builder, m_namenode_port);
    m_filesystem = hdfsBuilderConnect(m_builder);

    // status check
    int64_t sz_used = hdfsGetUsed(m_filesystem); 
    HT_INFOF("Non DFS Used bytes: %ld", sz_used);
    sz_used = hdfsGetCapacity(m_filesystem); 
    HT_INFOF("Configured Capacity bytes: %ld", sz_used);
  
  }

  virtual ~FileSystemHadoop(){ }







  Types::Fs get_type() override;

  const std::string to_string() override {
    return format(
      "(type=HADOOP, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }

  const std::string path_root;
  
	hdfsBuilder	*m_builder;
	hdfsFS       m_filesystem;
};


}}



#endif  // swc_lib_fs_Hadoop_FileSystem_h