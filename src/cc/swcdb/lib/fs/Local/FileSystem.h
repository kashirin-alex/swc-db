/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Local_FileSystem_h
#define swc_lib_fs_Local_FileSystem_h

#include <iostream>
#include "swcdb/lib/fs/FileSystem.h"
#include "swcdb/lib/core/FileUtils.h"

namespace SWC{ namespace FS {

void apply_local(Config::SettingsPtr settings);


class FileSystemLocal: public FileSystem {
  public:

  FileSystemLocal(
    Config::SettingsPtr settings) 
    : FileSystem(settings, settings->get<String>("swc.fs.local.path.root"))
  { }

  virtual ~FileSystemLocal(){}

  Types::Fs get_type() override;

  const std::string to_string() override {
    return format(
      "(type=LOCAL, path_root=%s, path_data=%s)", 
      path_root.c_str(),
      path_data.c_str()
    );
  }




  bool exists(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("exists path='%s'", abspath.c_str());
    
    errno = 0;
    bool state = FileUtils::exists(abspath);
    err = errno;
    return state;
  }

  void mkdirs(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("mkdirs path='%s'", abspath.c_str());
    
    errno = 0;
    FileUtils::mkdirs(abspath);
    err = errno;
  }

  void readdir(int &err, const String &name, DirentList &results) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("Readdir dir='%s'", abspath.c_str());

    std::vector<struct dirent> listing;
    errno = 0;
    FileUtils::readdir(abspath, "", listing);
    if (errno > 0) {
      err = errno;
      HT_ERRORF("FileUtils::readdir('%s') failed - %s", 
                abspath.c_str(), strerror(errno));
      return;
    }
    
    Dirent entry;
    String full_entry_path;
    struct stat statbuf;

    for(auto result : listing){
      if (result.d_name[0] == '.' || result.d_name[0] == 0)
        continue;

      entry.name.clear();
      entry.name.append(result.d_name);
      entry.is_dir = result.d_type == DT_DIR;

      full_entry_path.clear();
      full_entry_path.append(abspath);
      full_entry_path.append("/");
      full_entry_path.append(entry.name);
      if (stat(full_entry_path.c_str(), &statbuf) == -1) {
        err = errno;
        HT_ERRORF("readdir('%s') stat failed - %s", abspath.c_str(), strerror(errno));
        return;
      }
      entry.length = (uint64_t)statbuf.st_size;
      entry.last_modification_time = statbuf.st_mtime;
      results.push_back(entry);
    }
  }

  void create(int &err, SmartFdPtr &smartfd, int32_t bufsz,
              int32_t replication, int64_t blksz) override {

  };

  
};


}}



#endif  // swc_lib_fs_Local_FileSystem_h