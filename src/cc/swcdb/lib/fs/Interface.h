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

/*
#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
#include "Ceph/FileSystem.h"
#endif
*/

#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
#include "Hadoop/FileSystem.h"
#endif

#if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
#include "Broker/FileSystem.h"
#endif



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
      std::string fs_cfg("swc.fs");
#if defined (FS_BROKER_APP)
      fs_cfg.append(".broker.underlying");
#endif 
      m_type = parse_fs_type(Env::Config::settings()->get<String>(fs_cfg));
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

#if !defined (FS_BROKER_APP) // broker shouldn't be running the Fs-Broker
      case Types::Fs::BROKER:{
#if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
        return std::make_shared<FileSystemBroker>();
#endif
        fs_name.append("broker");
        break;
      }
#endif

      case Types::Fs::HADOOP:{
#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
        return std::make_shared<FileSystemHadoop>();
#endif
        fs_name.append("hadoop");
        break;
      }
/* 
      case Types::Fs::CEPH:{
#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
        return std::make_shared<FileSystemCeph>();
#endif
        fs_name.append("ceph");
        break;
      }
*/
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
    if(Env::Config::settings()->has("swc.fs.lib."+fs_name)) {
      fs_lib.append(Env::Config::settings()->get<String>("swc.fs.lib."+fs_name));
    } else {
      fs_lib.append(Env::Config::settings()->install_path);
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
    std::string handler_cfg_name =  "fs_apply_cfg_"+fs_name;
    void* f_cfg_ptr = dlsym(handle, handler_cfg_name.c_str());
    if (err != NULL || f_cfg_ptr == nullptr)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, link(%s) fail: %s handle=%d\n", 
                fs_lib.c_str(), handler_cfg_name.c_str(), err, (size_t)handle);
    ((fs_apply_cfg_t*)f_cfg_ptr)(Env::Config::get());
    
    err = dlerror();
    std::string handler_name =  "fs_make_new_"+fs_name;
    void* f_new_ptr = dlsym(handle, handler_name.c_str());
    if (err != NULL || f_new_ptr == nullptr)
      HT_THROWF(Error::CONFIG_BAD_VALUE, 
                "Shared Lib %s, link(%s) fail: %s handle=%d\n", 
                fs_lib.c_str(), handler_name.c_str(), err, (size_t)handle);
    
    loaded_dl = {.lib=handle, .cfg=f_cfg_ptr, .make=f_new_ptr};
    return FileSystemPtr(((fs_make_new_t*)f_new_ptr)());
  }

  operator InterfacePtr(){ return shared_from_this(); }

  virtual ~Interface(){
    stop();
    m_fs = nullptr;
    if(loaded_dl.lib != nullptr) {
      ((fs_apply_cfg_t*)loaded_dl.cfg)(nullptr);
      dlclose(loaded_dl.lib);
    }
  }

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
    readdir(err, base_path, dirs);

    if(err != Error::OK)
      return;

    for(auto& entry : dirs) {
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
  
  // default form to FS methods

  void readdir(int &err, std::string& base_path, DirentList& dirs) {
    for(;;) {
      err = Error::OK;
      DirentList found_dirs;
      m_fs->readdir(err, base_path, found_dirs);
      if(err == Error::OK || err == EACCES || err == ENOENT
        || err == Error::SERVER_SHUTTING_DOWN){
        dirs = found_dirs;
        return;
      }
      HT_DEBUGF("readdir, retrying to err=%d(%s)", err, Error::get_text(err));
    }
  }

  bool exists(int &err, const String &name) {
    bool state;
    for(;;) {
      err = Error::OK;
      state = m_fs->exists(err, name);
      if(err == Error::OK || err == Error::SERVER_SHUTTING_DOWN)
        break;
      HT_DEBUGF("exists, retrying to err=%d(%s)", err, Error::get_text(err));
    }
    return state;
  }

  void exists(Callback::ExistsCb_t cb, const String &name) {
    Callback::ExistsCb_t cb_wrapper; 
    cb_wrapper = [cb, name, &cb_wrapper, ptr=shared_from_this()]
    (int err, bool state){ 
      if(err == Error::OK || err == Error::SERVER_SHUTTING_DOWN) 
        cb(err, state);
      else 
        ptr->get_fs()->exists(cb_wrapper, name);
    };
    m_fs->exists(cb_wrapper, name);
  }

  void mkdirs(int &err, const String &name) {
    for(;;) {
      err = Error::OK;
      m_fs->mkdirs(err, name);
      if(err == Error::OK || err == EEXIST 
        || err == Error::SERVER_SHUTTING_DOWN)
        return;
      HT_DEBUGF("mkdirs, retrying to err=%d(%s)", err, Error::get_text(err));
    }
  } 

  void rmdir(int &err, const String &name) {
    for(;;) {
      err = Error::OK;
      m_fs->rmdir(err, name);
      if(err == Error::OK || err == EACCES || err == ENOENT
        || err == Error::SERVER_SHUTTING_DOWN)
        return;
      HT_DEBUGF("rmdir, retrying to err=%d(%s)", err, Error::get_text(err));
    }
  }
  void rmdir_incl_opt_subs(int &err, const String &name) {
    rmdir(err, name);
    if(err != Error::OK)
      return;

    const char* p=name.data();
    for(const char* c=p+name.length();c>p;c--){
      if(*c != '/')
        continue;
      std::string base_path(p, c-p);
      DirentList entrs;
      readdir(err, base_path, entrs);
      if(err == Error::OK && entrs.size() == 0)
        rmdir(err, base_path);
        if(err == Error::OK)
          continue;
      break;
    }
    if(err != Error::OK)
      err = Error::OK;
  }

  void remove(int &err, const String &name) {
    for(;;) {
      err = Error::OK;
      m_fs->remove(err, name);
      if(err == Error::OK || err == EACCES || err == ENOENT 
        || err == Error::SERVER_SHUTTING_DOWN)
        return;
      HT_DEBUGF("remove, retrying to err=%d(%s)", err, Error::get_text(err));
    }
  } 

  void write(int &err, SmartFdPtr smartfd,
             int32_t replication, int64_t blksz, 
             StaticBuffer &buffer){
    buffer.own=false;
    for(;;) {
      err = Error::OK;
      m_fs->write(err, smartfd, -1, -1, buffer);
      if (err == Error::OK
          || err == Error::FS_PATH_NOT_FOUND 
          || err == Error::FS_PERMISSION_DENIED
          || err == Error::SERVER_SHUTTING_DOWN)
        break;
      HT_DEBUGF("write, retrying to err=%d(%s)", err, Error::get_text(err));
    }
    buffer.own=true;
  }
  
 
  private:
  Types::Fs     m_type;
  FileSystemPtr m_fs;

  struct LoadedDL{
    void* lib = nullptr;
    void* cfg = nullptr;
    void* make = nullptr;
  };
  LoadedDL loaded_dl;
};



void set_structured_id(std::string number, std::string &s){
  if(number.length() <= id_split_len) {
    s.append(number);
    s.append({id_split_last});
  } else {
    int len = number.length()-id_split_len;
    int n=0;
    for(; n<len;){
      s.append(std::string(number, n, id_split_len));
      s.append("/");
      n += id_split_len;
    }
    s.append(std::string(number, n, id_split_len));
    s.append({id_split_last});
  }
};

} // namespace FS


namespace Env {

class FsInterface;
typedef std::shared_ptr<FsInterface> FsInterfacePtr;

class FsInterface {
  
  public:

  static void init() {
    m_env = std::make_shared<FsInterface>();
  }

  static FsInterfacePtr get(){
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

  FsInterface() : m_interface(std::make_shared<FS::Interface>()) {}
  virtual ~FsInterface(){}

  private:
  FS::InterfacePtr             m_interface = nullptr;
  inline static FsInterfacePtr m_env = nullptr;
};
}

}


#endif  // swc_lib_fs_Interface_h