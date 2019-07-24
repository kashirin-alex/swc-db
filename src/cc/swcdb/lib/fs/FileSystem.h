/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_FileSystem_h
#define swc_lib_fs_FileSystem_h

#include "Settings.h"
#include "SmartFd.h"
#include "Dirent.h"

#include "swcdb/lib/core/comm/AppContext.h"

namespace SWC{ namespace Types {
enum Fs {
  NONE,
  CUSTOM,
  LOCAL,
  HADOOP,
  CEPH,
  HTFSBROKER
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

  FileSystem(Config::SettingsPtr settings)
    : path_data(normalize_pathname(settings->get<String>("swc.fs.path.data")))
  { }

  virtual ~FileSystem() {}
  
  const std::string path_data;
  Config::SettingsPtr settings;

  virtual Types::Fs get_type() {
    return Types::Fs::NONE;
  };
  
  virtual const std::string to_string(){
    return format("(type=NONE, path_data=%s)", path_data.c_str());
  }
  /* 
  virtual void mkdirs(const std::string &name) = 0;
  virtual void mkdirs(const String &name, DispatchHandler *handler) = 0;

  virtual void rmdir(const String &name, bool force = true) = 0;
  virtual void rmdir(const String &name, DispatchHandler *handler) = 0;
    
  virtual void readdir(const String &name, std::vector<Dirent> &listing) = 0;
  virtual void readdir(const String &name, DispatchHandler *handler) = 0;
    
  virtual bool exists(const String &name) = 0;
  virtual void exists(const String &name, DispatchHandler *handler) = 0;

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