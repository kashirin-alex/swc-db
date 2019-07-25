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

  DirentList readdir(int &err, const String &name) override {
    std::string abspath = get_abspath(name);
    HT_DEBUGF("Readdir dir='%s'", abspath.c_str());

    DirentList listing;
    Dirent entry;

    errno = 0;
    DIR *dirp = opendir(abspath.c_str());
    if (dirp == 0) {
      err = errno;
      HT_ERRORF("opendir('%s') failed - %s", abspath.c_str(), strerror(errno));
      return listing;
    }

    struct dirent *result;

    String full_entry_path;
    struct stat statbuf;

#if defined(USE_READDIR_R) && USE_READDIR_R
    struct dirent *dp = (struct dirent *)new uint8_t [sizeof(struct dirent)+1025];
    int ret;
#endif

    do{
      errno = 0;
    
#if defined(USE_READDIR_R) && USE_READDIR_R
      ret = readdir_r(dirp, dp, &result)
      if (ret != 0) {
        err = errno;
        HT_ERRORF("readdir('%s') failed - %s", abspath.c_str(), strerror(errno));
        (void)closedir(dirp);
        delete [] (uint8_t *)dp;
        return listing;
      }
      if(result == NULL) break;

#else
      result = ::readdir(dirp);
      if(result == NULL){
        if(errno > 0){
          err = errno;
          HT_ERRORF("readdir('%s') failed - %s", abspath.c_str(), strerror(errno));
          (void)closedir(dirp);
          return listing;
        }
        break;
      }

#endif
      if (result->d_name[0] == '.' || result->d_name[0] == 0)
        continue;

      entry.name.clear();
      entry.name.append(result->d_name);
      entry.is_dir = result->d_type == DT_DIR;
      full_entry_path.clear();
      full_entry_path.append(abspath);
      full_entry_path.append("/");
      full_entry_path.append(entry.name);
      if (stat(full_entry_path.c_str(), &statbuf) == -1) {
        err = errno;
        HT_ERRORF("readdir('%s') failed - %s", abspath.c_str(), strerror(errno));
#if defined(USE_READDIR_R) && USE_READDIR_R
        delete [] (uint8_t *)dp;
#endif
        (void)closedir(dirp);
        return listing;
      }
      entry.length = (uint64_t)statbuf.st_size;
      entry.last_modification_time = statbuf.st_mtime;
      listing.push_back(entry);
      
    } while(1);

#if defined(USE_READDIR_R) && USE_READDIR_R
    delete [] (uint8_t *)dp;
#endif

    (void)closedir(dirp);
    return listing;
}


  
};


}}



#endif  // swc_lib_fs_Local_FileSystem_h