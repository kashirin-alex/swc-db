/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Interface_h
#define swc_lib_fs_Interface_h

#include <algorithm>
#include <dlfcn.h>

#include "Settings.h"

#include "SmartFd.h"
#include "Dirent.h"
#include "FileSystem.h"

#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
#include "Local/FileSystem.h"
#endif

#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
#include "Ceph/FileSystem.h"
#endif

#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
#include "Hadoop/FileSystem.h"
#endif

/* 
#if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
#include "Broker/FileSystem.h"
#endif
*/



namespace SWC{ namespace FS {

/// Interface to FileSystems

class Interface;
typedef std::shared_ptr<Interface> InterfacePtr;

class Interface : std::enable_shared_from_this<Interface>{
  public:

  Interface(Types::Fs typ=Types::Fs::NONE): m_type(typ) {

    if(m_type == Types::Fs::NONE){
      std::string fs_name;
      fs_name.append(Config::settings->get<String>("swc.fs"));
      std::transform(fs_name.begin(), fs_name.end(), fs_name.begin(),
                     [](unsigned char c){ return std::tolower(c); });
                   
      if(fs_name.compare("local") == 0)
        m_type = Types::Fs::LOCAL;
      else if(fs_name.compare("hadoop") == 0)
        m_type = Types::Fs::HADOOP;
      else if(fs_name.compare("ceph") == 0)
        m_type = Types::Fs::CEPH;
      else if(fs_name.compare("broker") == 0)
        m_type = Types::Fs::BROKER;
      else if(fs_name.compare("custom") == 0)
        m_type = Types::Fs::CUSTOM;
      else
        HT_THROWF(Error::CONFIG_BAD_VALUE, 
          "Unknown FileSystem name=%s", fs_name.c_str());
    }
    
    m_fs = use_filesystem();

    HT_INFOF("INIT-%s", to_string().c_str());
  }
  
  FileSystemPtr use_filesystem(){
    std::string fs_name;
    
    switch(m_type){

      case Types::Fs::LOCAL:{
#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
        apply_local(Config::settings);
        return std::make_shared<FileSystemLocal>(Config::settings);
#endif
        fs_name.append("local");
        break;
      }

      case Types::Fs::HADOOP:{
#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
        apply_hadoop(Config::settings);
        return std::make_shared<FileSystemHadoop>(Config::settings);
#endif
        fs_name.append("hadoop");
        break;
      }

      case Types::Fs::CEPH:{
#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
        apply_ceph(Config::settings);
        return std::make_shared<FileSystemCeph>(Config::settings);
#endif
        fs_name.append("ceph");
        break;
      }

      case Types::Fs::CUSTOM: {
        fs_name.append("custom");
        break;
      }

      default:
        HT_THROWF(Error::CONFIG_BAD_VALUE, 
          "Unimplemented FileSystem name=%s type=%d", 
          fs_name.c_str(), (int)m_type);
    }
    
    std::string fs_lib;
    if(Config::settings->has("swc.fs.lib."+fs_name)) {
      fs_lib.append(Config::settings->get<String>("swc.fs.lib."+fs_name));
    } else {
      fs_lib.append(Config::settings->install_path);
      fs_lib.append("/lib/libswcdb_fs_"); // (./lib/libswcdb_fs_local.so)
      fs_lib.append(fs_name);
      fs_lib.append(".so");
    }

    const char* err = dlerror();
    void* handle = dlopen(fs_lib.c_str(), RTLD_NOW | RTLD_LAZY | RTLD_LOCAL);
    if (handle == NULL || err != NULL)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, open fail: %s\n", 
                fs_lib.c_str(), err);

    err = dlerror();
    void* f_cfg_ptr = dlsym(handle, "fs_apply_cfg");
    if (err != NULL || f_cfg_ptr == nullptr)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, link(fs_apply_cfg) fail: %s handle=%d\n", 
                fs_lib.c_str(), err, (size_t)handle);
    ((fs_apply_cfg_t*)f_cfg_ptr)(Config::settings);

    err = dlerror();
    void* f_new_ptr = dlsym(handle, "fs_make_new");
    if (err != NULL || f_new_ptr == nullptr)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, link(fs_make_new) fail: %s handle=%d\n", 
                fs_lib.c_str(), err, (size_t)handle);

    return FileSystemPtr(((fs_make_new_t*)f_new_ptr)(Config::settings));
  }

  operator InterfacePtr(){ return shared_from_this(); }

  virtual ~Interface(){}


  const Types::Fs get_type(){
    return m_fs->get_type();
  }

  FileSystemPtr get_fs(){
    return m_fs;
  }

  const String to_string(){
    return format("FS::Interface(type=%d, details=%s)", 
                  (int)m_type, m_fs==nullptr?"NULL":m_fs->to_string().c_str());
  }

  void stop() { 
    m_fs->stop();
  }



  private:
  Types::Fs     m_type;
  FileSystemPtr m_fs;
};


}}


#endif  // swc_lib_fs_Interface_h