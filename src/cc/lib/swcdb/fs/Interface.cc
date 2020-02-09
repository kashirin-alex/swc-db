/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/fs/Interface.h"
#include <dlfcn.h>

namespace SWC{ namespace FS {

Interface::Interface(Types::Fs typ) : m_type(typ), m_fs(use_filesystem()) {

  SWC_LOGF(LOG_INFO, "INIT-%s", to_string().c_str());
}
  
FileSystem::Ptr Interface::use_filesystem(){
  std::string fs_name;
    
  switch(m_type){

    case Types::Fs::LOCAL:{
#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
      return std::make_shared<FileSystemLocal>();
#endif
      fs_name.append("local");
      break;
    }

    case Types::Fs::BROKER:{
#if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
      return std::make_shared<FileSystemBroker>();
#endif
      fs_name.append("broker");
      break;
    }

    case Types::Fs::HADOOP: {
#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
      return std::make_shared<FileSystemHadoop>();
#endif
      fs_name.append("hadoop");
      break;
    }

    case Types::Fs::HADOOP_JVM: {
#if defined (BUILTIN_FS_HADOOP_JVM) || defined (BUILTIN_FS_ALL)
      return std::make_shared<FileSystemHadoopJVM>();
#endif
      fs_name.append("hadoop_jvm");
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
      SWC_THROWF(Error::CONFIG_BAD_VALUE, 
        "Unimplemented FileSystem name=%s type=%d", 
        fs_name.c_str(), (int)m_type);
  }
    
  std::string fs_lib;
  if(Env::Config::settings()->has("swc.fs.lib."+fs_name)) {
    fs_lib.append(
      Env::Config::settings()->get<std::string>("swc.fs.lib."+fs_name));
  } else {
    fs_lib.append(Env::Config::settings()->install_path);
    fs_lib.append("/lib/libswcdb_fs_"); // (./lib/libswcdb_fs_local.so)
    fs_lib.append(fs_name);
    fs_lib.append(".so");
  }

  const char* err = dlerror();
  void* handle = dlopen(fs_lib.c_str(), RTLD_NOW | RTLD_LAZY | RTLD_LOCAL);
  if (handle == NULL || err != NULL)
    SWC_THROWF(Error::CONFIG_BAD_VALUE, 
              "Shared Lib %s, open fail: %s\n", 
              fs_lib.c_str(), err);

  err = dlerror();
  std::string handler_name =  "fs_apply_cfg_"+fs_name;
  void* f_cfg_ptr = dlsym(handle, handler_name.c_str());
  if (err != NULL || f_cfg_ptr == nullptr)
    SWC_THROWF(Error::CONFIG_BAD_VALUE, 
              "Shared Lib %s, link(%s) fail: %s handle=%d\n", 
              fs_lib.c_str(), handler_name.c_str(), err, (size_t)handle);
  ((fs_apply_cfg_t*)f_cfg_ptr)(Env::Config::get());
    
  err = dlerror();
  handler_name =  "fs_make_new_"+fs_name;
  void* f_new_ptr = dlsym(handle, handler_name.c_str());
  if (err != NULL || f_new_ptr == nullptr)
    SWC_THROWF(Error::CONFIG_BAD_VALUE, 
              "Shared Lib %s, link(%s) fail: %s handle=%d\n", 
              fs_lib.c_str(), handler_name.c_str(), err, (size_t)handle);
    
  loaded_dl = {.lib=handle, .cfg=f_cfg_ptr, .make=f_new_ptr};
  return FileSystem::Ptr(((fs_make_new_t*)f_new_ptr)());
}

Interface::Ptr Interface::ptr(){ 
  return this; 
}

Interface::~Interface(){
  stop();
  m_fs = nullptr;
  if(loaded_dl.lib != nullptr) {
    ((fs_apply_cfg_t*)loaded_dl.cfg)(nullptr);
    dlclose(loaded_dl.lib);
  }
}

const Types::Fs Interface::get_type(){
  return m_fs->get_type();
}

FileSystem::Ptr Interface::get_fs(){
  return m_fs;
}

const std::string Interface::to_string() {
  return format("FS::Interface(type=%d, details=%s)", 
                (int)m_type, m_fs==nullptr?"NULL":m_fs->to_string().c_str());
}

void Interface::stop() { 
  m_fs->stop();
}

void Interface::get_structured_ids(int &err, std::string base_path, 
                                  IdEntries_t &entries, std::string base_id) {
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
        SWC_LOGF(LOG_ERROR, "Error converting id_name=%s to int64", id_name.c_str());
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

void Interface::readdir(int &err, const std::string& base_path, 
                        DirentList& dirs) {
  for(;;) {
    err = Error::OK;
    DirentList found_dirs;
    m_fs->readdir(err, base_path, found_dirs);
    if(err == Error::OK || err == EACCES || err == ENOENT
      || err == Error::SERVER_SHUTTING_DOWN){
      dirs = found_dirs;
      return;
    }
    SWC_LOGF(LOG_DEBUG, "readdir, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
}

bool Interface::exists(int &err, const std::string& name) {
  bool state;
  for(;;) {
    err = Error::OK;
    state = m_fs->exists(err, name);
    if(err == Error::OK || err == Error::SERVER_SHUTTING_DOWN)
      break;
    SWC_LOGF(LOG_DEBUG, "exists, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
  return state;
}

void Interface::exists(Callback::ExistsCb_t cb, const std::string &name) {
  Callback::ExistsCb_t cb_wrapper; 
  cb_wrapper = [cb, name, &cb_wrapper, ptr=ptr()]
    (int err, bool state){ 
      if(err == Error::OK || err == Error::SERVER_SHUTTING_DOWN) 
        cb(err, state);
      else 
        ptr->get_fs()->exists(cb_wrapper, name);
    };
  m_fs->exists(cb_wrapper, name);
}

void Interface::mkdirs(int &err, const std::string &name) {
  for(;;) {
    err = Error::OK;
    m_fs->mkdirs(err, name);
    if(err == Error::OK || err == EEXIST 
      || err == Error::SERVER_SHUTTING_DOWN)
      return;
    SWC_LOGF(LOG_DEBUG, "mkdirs, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
} 

void Interface::rmdir(int &err, const std::string &name) {
  for(;;) {
    err = Error::OK;
    m_fs->rmdir(err, name);
    if(err == Error::OK || err == EACCES || err == ENOENT
      || err == Error::SERVER_SHUTTING_DOWN)
      return;
    SWC_LOGF(LOG_DEBUG, "rmdir, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
}

void Interface::rmdir_incl_opt_subs(int &err, const std::string &name, 
                                    const std::string &up_to) {
  rmdir(err, name);
  if(err != Error::OK)
    return;

  const char* p=name.data();
  std::string base_path;
  for(const char* c=p+name.length();c>p;c--){
    if(*c != '/')
      continue;
    base_path = std::string(p, c-p);
    if(up_to.compare(base_path) == 0)
      break;
    DirentList entrs;
    readdir(err, base_path, entrs);
    if(!err && !entrs.size())
      rmdir(err, base_path);
      if(err == Error::OK)
        continue;
    break;
  }
  if(err != Error::OK)
    err = Error::OK;
}

void Interface::remove(int &err, const std::string &name) {
  for(;;) {
    err = Error::OK;
    m_fs->remove(err, name);
    if(err == Error::OK || err == EACCES || err == ENOENT 
      || err == Error::SERVER_SHUTTING_DOWN)
      return;
    SWC_LOGF(LOG_DEBUG, "remove, retrying to err=%d(%s)",
              err, Error::get_text(err));
  }
} 
  
void Interface::rename(int &err, const std::string &from , 
                       const std::string &to) {
  for(;;) {
    err = Error::OK;
    m_fs->rename(err, from, to);
    if(err == Error::OK || err == EACCES || err == ENOENT 
      || err == Error::SERVER_SHUTTING_DOWN)
      return;
    SWC_LOGF(LOG_DEBUG, "rename, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
} 

void Interface::write(int &err, SmartFd::Ptr smartfd,
                      uint8_t replication, int64_t blksz, 
                      StaticBuffer &buffer) {
  buffer.own=false;
  for(;;) {
    err = Error::OK;
    m_fs->write(err, smartfd, replication, blksz, buffer);
    if (err == Error::OK
        || err == Error::FS_PATH_NOT_FOUND 
        || err == Error::FS_PERMISSION_DENIED
        || err == Error::SERVER_SHUTTING_DOWN)
      break;
    SWC_LOGF(LOG_DEBUG, "write, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
  buffer.own=true;
}
    
void Interface::read(int& err, const std::string& name, StaticBuffer* dst) {
  do {
    if(err)
      SWC_LOGF(LOG_DEBUG, "read-all, retrying to err=%d(%s)", 
               err, Error::get_text(err));
    m_fs->read(err = Error::OK, name, dst);
  } while(err && 
          err != Error::FS_EOF &&
          err != Error::SERVER_SHUTTING_DOWN &&
          err != Error::FS_PATH_NOT_FOUND &&
          err != Error::FS_PERMISSION_DENIED);
}

bool Interface::open(int& err, SmartFd::Ptr& smartfd) {
  m_fs->open(err, smartfd);
  if(err == Error::FS_PATH_NOT_FOUND ||
     err == Error::FS_PERMISSION_DENIED ||
     err == Error::SERVER_SHUTTING_DOWN)
    return false;
  if(!smartfd->valid())
    return true;
      
  if(err != Error::OK) {
    int tmperr = Error::OK;
    m_fs->close(tmperr, smartfd);
    return true;
  }
  return false;
}
  
bool Interface::create(int& err, SmartFd::Ptr& smartfd,
                       int32_t bufsz, uint8_t replication, int64_t blksz) {
  m_fs->create(err, smartfd, bufsz, replication, blksz);
  if(err == Error::FS_PATH_NOT_FOUND ||
     err == Error::FS_PERMISSION_DENIED ||
     err == Error::SERVER_SHUTTING_DOWN)
    return false;
  if(!smartfd->valid())
    return true;
      
  if(err != Error::OK) {
    int tmperr = Error::OK;
    m_fs->close(tmperr, smartfd);
    return true;
  }
  return false;
}

void Interface::close(int& err, SmartFd::Ptr smartfd) {
  for(;smartfd->valid();err = Error::OK) {
    m_fs->close(err, smartfd);
    if(err == Error::OK || err == EACCES || err == ENOENT 
      || err == Error::SERVER_SHUTTING_DOWN)
      break;
    SWC_LOGF(LOG_DEBUG, "close, retrying to err=%d(%s)", 
              err, Error::get_text(err));
  }
}



void set_structured_id(std::string number, std::string &s) {
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
}

} // namespace FS


namespace Env {

void FsInterface::init(Types::Fs typ) {
  m_env = std::make_shared<FsInterface>(typ);
}

FsInterface::Ptr FsInterface::get(){
  return m_env;
}

FS::Interface::Ptr FsInterface::interface(){
  SWC_ASSERT(m_env != nullptr);
  return m_env->m_interface;
}

FS::FileSystem::Ptr FsInterface::fs(){
  SWC_ASSERT(m_env != nullptr);
  return m_env->m_interface->get_fs();
}

void FsInterface::reset() {
  m_env = nullptr;
}

FsInterface::FsInterface(Types::Fs typ) 
                        : m_interface(new FS::Interface(typ)) {}

FsInterface::~FsInterface(){
  if(m_interface != nullptr)
    delete m_interface;
}
}

} // namespace SWC