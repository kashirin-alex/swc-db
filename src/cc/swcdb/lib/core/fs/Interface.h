/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_fs_Interface_h
#define swc_core_fs_Interface_h

#include <algorithm>
#include <dlfcn.h>
#include "swcdb/lib/core/fs/Settings.h"

#include "SmartFd.h"
#include "FileSystem.h"

#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
#include "swcdb/lib/fs/Local/FileSystem.h"
#endif

#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
#include "swcdb/lib/fs/Ceph/FileSystem.h"
#endif
/* 
#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
#include "swcdb/lib/fs/Hadoop/FileSystem.h"
#endif

#if defined (BUILTIN_FS_HTFSBROKER) || defined (BUILTIN_FS_ALL)
#include "swcdb/lib/fs/htFsBroker/FileSystem.h"
#endif
*/



namespace SWC{ namespace FS {

/// Interface to FileSystems

class Interface;
typedef std::shared_ptr<Interface> InterfacePtr;

class Interface : std::enable_shared_from_this<Interface>{
  public:

  Interface() {

    m_path_root = Config::settings->get<String>("swc.fs.root");

    std::string fs_name = Config::settings->get<String>("swc.fs");
    std::transform(fs_name.begin(), fs_name.end(), fs_name.begin(),
                   [](unsigned char c){ return std::tolower(c); });
                   
    if(fs_name.compare("local") == 0)
      m_type = Types::Fs::LOCAL;
    else if(fs_name.compare("hadoop") == 0)
      m_type = Types::Fs::HADOOP;
    else if(fs_name.compare("ceph") == 0)
      m_type = Types::Fs::CEPH;
    else if(fs_name.compare("htfsbroker") == 0)
      m_type = Types::Fs::HTFSBROKER;
    else if(fs_name.compare("custom") == 0)
      m_type = Types::Fs::CUSTOM;
    else
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
        "Unknown FileSystem name=%s", fs_name.c_str());

    
    std::string fs_lib;
    if(Config::settings->has("swc.fs.lib")) {
      fs_lib = Config::settings->get<String>("swc.fs.lib");
    } else {
      fs_lib.append(Config::settings->install_path);
      fs_lib.append("/lib/libswcdb_fs_"); // (./lib/libswcdb_fs_local.so)
      fs_lib.append(fs_name);
      fs_lib.append(".so");
    }
    
    bool fs_set = false;
    switch(m_type){

      case Types::Fs::LOCAL:{
#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
        m_fs = std::make_shared<FileSystemLocal>();
        fs_set = true;
#endif
        break;
      }

      case Types::Fs::CEPH:{
#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
        m_fs = std::make_shared<FileSystemCeph>();
        fs_set = true;
#endif
        break;
      }

      case Types::Fs::CUSTOM:
        break;

      default:
        HT_THROWF(Error::CONFIG_BAD_VALUE, 
          "Unimplemented FileSystem name=%s  type=%d", 
          fs_name.c_str(), (int)m_type);
    }
    
    if(!fs_set){

      const char* err = dlerror();
      void* handle = dlopen(fs_lib.c_str(), RTLD_NOW | RTLD_LAZY | RTLD_LOCAL);
      if (handle == NULL || err != NULL)
        HT_THROWF(Error::CONFIG_BAD_VALUE, 
                  "Shared Lib %s, open fail: %s\n", 
                  fs_lib.c_str(), err);

      err = dlerror();
      void* fptr = dlsym(handle, "make_new_fs");
      if (err != NULL || fptr == nullptr)
        HT_THROWF(Error::CONFIG_BAD_VALUE, 
                  "Shared Lib %s, link fail: %s handle=%d\n", 
                  fs_lib.c_str(), err, (size_t)handle);
      m_fs = FileSystemPtr((FileSystem*)((make_new_fs_t*)fptr)());
    }

    HT_INFOF("INIT-%s", to_string().c_str());
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
    return format("FS::Interface(type=%d, path_root=%s, details=%s)", 
                  (int)m_type, m_path_root.c_str(), 
                  m_fs==nullptr?"NULL":m_fs->to_string().c_str());
  }

  private:
  Types::Fs     m_type;
  std::string   m_path_root;
  FileSystemPtr m_fs;
};


}}


#endif  // swc_core_fs_Interface_h