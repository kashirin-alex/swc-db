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

inline Types::Fs parse_fs_type(std::string fs_name){
  std::transform(fs_name.begin(), fs_name.end(), fs_name.begin(),
                 [](unsigned char c){ return std::tolower(c); });
    
#if !defined (FS_BROKER_APP)
  if(fs_name.compare("broker") == 0)
    return Types::Fs::BROKER;
#endif

  if(fs_name.compare("local") == 0)
    return Types::Fs::LOCAL;
  if(fs_name.compare("hadoop") == 0)
    return Types::Fs::HADOOP;
  if(fs_name.compare("ceph") == 0)
    return Types::Fs::CEPH;
  if(fs_name.compare("custom") == 0)
    return Types::Fs::CUSTOM;
  else
    HT_THROWF(Error::CONFIG_BAD_VALUE, 
              "Unknown FileSystem name=%s", fs_name.c_str());
  return Types::Fs::NONE;
}

inline std::string type_to_string(Types::Fs typ){
  if(typ == Types::Fs::BROKER)
    return "Broker";
  if(typ == Types::Fs::LOCAL)
    return "Local";
  if(typ == Types::Fs::HADOOP)
    return "Hadoop";
  if(typ == Types::Fs::CEPH)
    return "Ceph";
  if(typ == Types::Fs::CUSTOM)
    return "Custom";
  HT_THROWF(Error::CONFIG_BAD_VALUE, 
            "Unknown FileSystem type=%d", (int)typ);
}


class FileSystem {
  public:

  FileSystem() { }
  
  FileSystem(std::string root, bool setting_applied)
    : path_root(normalize_pathname(root)),
      path_data(normalize_pathname(EnvConfig::settings()->get<String>("swc.fs.path.data")))
  { }

  virtual ~FileSystem() { std::cout << " ~FileSystem() \n"; }

  virtual void stop() {}

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
    int err = Error::OK;
    bool state = exists(err, name);
    cb(err, state);
  }

  // Directory Actions
  virtual void mkdirs(int &err, const std::string &name) = 0;
  virtual 
  void mkdirs(Callback::MkdirsCb_t cb, const String &name) {
    int err = Error::OK;
    mkdirs(err, name);
    cb(err);
  }

  virtual void readdir(int &err, const std::string &name, 
                       DirentList &results) = 0;
  virtual 
  void readdir(Callback::ReaddirCb_t cb, const String &name) {
    int err = Error::OK;
    DirentList listing;
    readdir(err, name, listing);
    cb(err, listing);
  }

  virtual void remove(int &err, const String &name) = 0;
  virtual 
  void remove(Callback::RemoveCb_t cb, const String &name) {
    int err = Error::OK;
    remove(err, name);
    cb(err);
  }

  // File(fd) Actions
  virtual void create(int &err, SmartFdPtr &smartfd,
                      int32_t bufsz, int32_t replication, int64_t blksz) = 0;
  virtual 
  void create(Callback::CreateCb_t cb, SmartFdPtr &smartfd,
              int32_t bufsz, int32_t replication, int64_t blksz) {
    int err = Error::OK;
    create(err, smartfd, bufsz, replication, blksz);
    cb(err, smartfd);
  }

  virtual void open(int &err, SmartFdPtr &smartfd, int32_t bufsz=0) = 0;
  virtual 
  void open(Callback::OpenCb_t cb, SmartFdPtr &smartfd, int32_t bufsz=0) {
    int err = Error::OK;
    open(err, smartfd, bufsz);
    cb(err, smartfd);
  }
  

  virtual size_t read(int &err, SmartFdPtr &smartfd, 
                      void *dst, size_t amount) = 0;
  virtual 
  void read(Callback::ReadCb_t cb, SmartFdPtr &smartfd, size_t amount) {
    int err = Error::OK;
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
    int err = Error::OK;
    size_t len = append(err, smartfd, buffer, flags);
    cb(err, smartfd, len);
  }

  virtual void close(int &err, SmartFdPtr &smartfd) = 0;
  virtual 
  void close(Callback::CloseCb_t cb, SmartFdPtr &smartfd) {
    int err = Error::OK;
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
      
  */



};

typedef std::shared_ptr<FileSystem> FileSystemPtr;




}}

extern "C"{
typedef SWC::FS::FileSystem* fs_make_new_t();
SWC::FS::FileSystem* fs_make_new();

typedef bool fs_apply_cfg_t();
bool fs_apply_cfg();
}

#endif  // swc_lib_fs_FileSystem_h