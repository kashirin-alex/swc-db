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
  
typedef std::vector<int64_t> IdEntries_t;
const int id_split_len = 3;
const char id_split_last = 'g';

/// Interface to FileSystems

class Interface;
typedef std::shared_ptr<Interface> InterfacePtr;

class Interface : std::enable_shared_from_this<Interface>{
  public:

  Interface(Types::Fs typ=Types::Fs::NONE): m_type(typ) {

    if(m_type == Types::Fs::NONE){
      std::string fs_name;
      fs_name.append(EnvConfig::settings()->get<String>("swc.fs"));
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
        return std::make_shared<FileSystemLocal>();
#endif
        fs_name.append("local");
        break;
      }

      case Types::Fs::HADOOP:{
#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
        return std::make_shared<FileSystemHadoop>();
#endif
        fs_name.append("hadoop");
        break;
      }

      case Types::Fs::CEPH:{
#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
        return std::make_shared<FileSystemCeph>();
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
    if(EnvConfig::settings()->has("swc.fs.lib."+fs_name)) {
      fs_lib.append(EnvConfig::settings()->get<String>("swc.fs.lib."+fs_name));
    } else {
      fs_lib.append(EnvConfig::settings()->install_path);
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
    /* 
    err = dlerror();
    void* f_cfg_ptr = dlsym(handle, "fs_apply_cfg");
    if (err != NULL || f_cfg_ptr == nullptr)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, link(fs_apply_cfg) fail: %s handle=%d\n", 
                fs_lib.c_str(), err, (size_t)handle);
    ((fs_apply_cfg_t*)f_cfg_ptr)();
    */
    err = dlerror();
    void* f_new_ptr = dlsym(handle, "fs_make_new");
    if (err != NULL || f_new_ptr == nullptr)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, link(fs_make_new) fail: %s handle=%d\n", 
                fs_lib.c_str(), err, (size_t)handle);

    return FileSystemPtr(((fs_make_new_t*)f_new_ptr)());
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

  void get_structured_ids(int &err, std::string base_path, 
                          IdEntries_t &entries, std::string base_id=""){
    //path(base_path) .../111/222/333/444f >> IdEntries_t{(int64_t)111222333444}
    
    DirentList dirs;
    m_fs->readdir(err, base_path, dirs);

    if(err != Error::OK)
      return;

    for(auto entry : dirs) {
      // std::cout << entry.to_string();
      if(!entry.is_dir) continue;

      std::string id_name = base_id;
      if(entry.name.back() == id_split_last){
        id_name.append(entry.name.substr(0, entry.name.length()-1));
        try {
          entries.push_back((int64_t)strtoll(id_name.c_str(), NULL, 0));
        } catch(...){
          HT_ERRORF("Error converting id_name=%s to int64", id_name.c_str());
        }
        continue;
      }
      
      std::string new_base_path = base_path;
      new_base_path.append("/");
      new_base_path.append(entry.name);
      id_name.append(entry.name);
      get_structured_ids(err, new_base_path, entries, id_name);
      if(err != Error::OK)
        return;
    }
  }

  private:
  Types::Fs     m_type;
  FileSystemPtr m_fs;
};



void set_structured_id(std::string number, std::string &s){
  if(number.length() <= id_split_len) {
    s.append(number);
    s.append({id_split_last});
  } else {
    int len = number.length()-id_split_len;
    for(int n=0; n<len;){
      s.append(std::string(number, n, id_split_len));
      s.append("/");
      n += id_split_len;
    }
    s.append(std::string(number, len, id_split_len));
    s.append({id_split_last});
  }
};

} // namespace FS


class EnvFsInterface;
typedef std::shared_ptr<EnvFsInterface> EnvFsInterfacePtr;

class EnvFsInterface {
  
  public:

  static void init() {
    m_env = std::make_shared<EnvFsInterface>();
  }

  static EnvFsInterfacePtr get(){
    return m_env;
  }

  static FS::InterfacePtr interface(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_interface;
  }

  static FS::FileSystemPtr fs(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_interface->get_fs();
  }

  EnvFsInterface() : m_interface(std::make_shared<FS::Interface>()) {}
  virtual ~EnvFsInterface(){}

  private:
  FS::InterfacePtr                m_interface = nullptr;
  inline static EnvFsInterfacePtr m_env = nullptr;
};

}


#endif  // swc_lib_fs_Interface_h