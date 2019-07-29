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


enum OpenFlags {
  OPEN_FLAG_DIRECTIO = 0x00000001,
  OPEN_FLAG_OVERWRITE = 0x00000002,
  OPEN_FLAG_VERIFY_CHECKSUM = 0x00000004
};

enum Flags : uint8_t {
  NONE=0,
  FLUSH=1,
  SYNC=2
};

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
  virtual void create(int &err, SmartFdPtr &smartfd,
                      int32_t bufsz, int32_t replication, int64_t blksz) = 0;
  virtual 
  void create(Callback::CreateCb_t cb, SmartFdPtr &smartfd,
              int32_t bufsz, int32_t replication, int64_t blksz) {
    int err = 0;
    create(err, smartfd, bufsz, replication, blksz);
    cb(err, smartfd);
  }

  virtual void open(int &err, SmartFdPtr &smartfd, int32_t bufsz=0) = 0;
  virtual 
  void open(Callback::OpenCb_t cb, SmartFdPtr &smartfd, int32_t bufsz=0) {
    int err = 0;
    open(err, smartfd, bufsz);
    cb(err, smartfd);
  }
  

  virtual size_t read(int &err, SmartFdPtr &smartfd, 
                      void *dst, size_t amount) = 0;
  virtual 
  void read(Callback::ReadCb_t cb, SmartFdPtr &smartfd, size_t amount) {
    int err = 0;
    StaticBuffer dst(amount);
    size_t nread = read(err, smartfd, dst.base, amount);
    
    dst.size = nread;
    cb(err, smartfd, dst);
  }

  virtual size_t append(int &err, SmartFdPtr &smartfd, 
                        StaticBuffer &buffer, Flags flags) = 0;
  virtual 
  void append(Callback::AppendCb_t cb, SmartFdPtr &smartfd, 
              StaticBuffer &buffer, Flags flags) {
    int err = 0;
    size_t len = append(err, smartfd, buffer, flags);
    cb(err, smartfd, len);
  }

  virtual void close(int &err, SmartFdPtr &smartfd) = 0;
  virtual 
  void close(Callback::CloseCb_t cb, SmartFdPtr &smartfd) {
    int err = 0;
    close(err, smartfd);
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