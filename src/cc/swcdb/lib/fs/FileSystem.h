/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_FileSystem_h
#define swc_lib_fs_FileSystem_h

#include "Settings.h"
#include "SmartFd.h"
#include "Dirent.h"
#include "Callbacks.h"

#include "swcdb/lib/core/comm/AppContext.h"

namespace SWC{ namespace Types {
enum Fs {
  NONE,
  CUSTOM,
  LOCAL,
  HADOOP,
  CEPH,
  BROKER
};
}}



namespace SWC { namespace FS {
  

inline std::string normalize_pathname(std::string s){
  if(*(--s.end()) != '/') 
    s.append("/"); 
  return s;
}


class FileSystem {
  public:

  FileSystem(Config::SettingsPtr settings, std::string root)
    : settings(settings),
      path_root(normalize_pathname(root)),
      path_data(normalize_pathname(settings->get<String>("swc.fs.path.data")))
  { }

  virtual ~FileSystem() { std::cout << " ~FileSystem() \n"; }

  virtual void stop() {}

  Config::SettingsPtr settings;
  const std::string path_root;
  const std::string path_data;

  virtual Types::Fs get_type() {
    return Types::Fs::NONE;
  }
  
  virtual const std::string to_string() {
    return format(
      "(type=NONE, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }

  virtual const std::string get_abspath(const std::string &name) {
    std::string abspath;
    abspath.append(path_root);
    abspath.append(path_data);
    abspath.append(name);
    return abspath;
  }

  
  virtual bool exists(int &err, const String &name) = 0;
  virtual 
  void exists(Callback::ExistsCb_t cb, const String &name) {
    int err = 0;
    bool state = exists(err, name);
    cb(err, state);
  }

  // Directory Actions
  virtual void mkdirs(int &err, const std::string &name) = 0;
  virtual 
  void mkdirs(Callback::MkdirsCb_t cb, const String &name) {
    int err = 0;
    mkdirs(err, name);
    cb(err);
  }

  virtual void readdir(int &err, const std::string &name, 
                       DirentList &results) = 0;
  virtual 
  void readdir(Callback::ReaddirCb_t cb, const String &name) {
    int err = 0;
    DirentList listing;
    readdir(err, name, listing);
    cb(err, listing);
  }


  // File Actions
  virtual void create(int &err, SmartFdPtr &smartfd, int32_t bufsz,
                      int32_t replication, int64_t blksz) = 0;
  virtual 
  void create(Callback::CreateCb_t cb, SmartFdPtr &smartfd, int32_t bufsz,
              int32_t replication, int64_t blksz) {
    int err = 0;
    create(err, smartfd, bufsz, replication, blksz);
    cb(err, smartfd);
  }

  /* 

  virtual void rmdir(const String &name, bool force = true) = 0;
  virtual void rmdir(const String &name, DispatchHandler *handler) = 0;

  virtual void rename(const String &src, const String &dst) = 0;
  virtual void rename(const String &src, const String &dst,
      DispatchHandler *handler) = 0;
      
  virtual int64_t length(const String &name, bool accurate = true) = 0;
  virtual void length(const String &name, bool accurate,
      DispatchHandler *handler) = 0;
      
  virtual void remove(const String &name, bool force = true) = 0;
  virtual void remove(const String &name, DispatchHandler *handler) = 0;
  */



};

typedef std::shared_ptr<FileSystem> FileSystemPtr;




}}

extern "C"{
typedef SWC::FS::FileSystem* fs_make_new_t(SWC::Config::SettingsPtr);
SWC::FS::FileSystem* fs_make_new(SWC::Config::SettingsPtr settings);

typedef void fs_apply_cfg_t(SWC::Config::SettingsPtr);
void fs_apply_cfg(SWC::Config::SettingsPtr settings);
}

#endif  // swc_lib_fs_FileSystem_h