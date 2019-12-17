/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Local_FileSystem_h
#define swc_lib_fs_Local_FileSystem_h

#include "swcdb/fs/FileSystem.h"

namespace SWC{ namespace FS {


bool apply_local();


class FileSystemLocal: public FileSystem {
  public:

  FileSystemLocal();

  virtual ~FileSystemLocal();

  Types::Fs get_type() override;

  const std::string to_string() override;


  bool exists(int &err, const std::string &name) override;

  void remove(int &err, const std::string &name) override;
  
  size_t length(int &err, const std::string &name) override;

  void mkdirs(int &err, const std::string &name) override;

  void readdir(int &err, const std::string &name, 
               DirentList &results) override;

  void rmdir(int &err, const std::string &name) override;

  void rename(int &err, const std::string &from, 
                        const std::string &to) override;
  
  void write(int &err, SmartFd::Ptr &smartfd,
             uint8_t replication, int64_t blksz, 
             StaticBuffer &buffer);

  void create(int &err, SmartFd::Ptr &smartfd, 
              int32_t bufsz, uint8_t replication, int64_t blksz) override;

  void open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz=0) override;
  
  size_t read(int &err, SmartFd::Ptr &smartfd, 
              void *dst, size_t amount) override;

  size_t pread(int &err, SmartFd::Ptr &smartfd, 
               uint64_t offset, void *dst, size_t amount) override;

  size_t append(int &err, SmartFd::Ptr &smartfd, 
                StaticBuffer &buffer, Flags flags) override;

  void seek(int &err, SmartFd::Ptr &smartfd, size_t offset) override;

  void flush(int &err, SmartFd::Ptr &smartfd) override;

  void sync(int &err, SmartFd::Ptr &smartfd) override;

  void close(int &err, SmartFd::Ptr &smartfd) override;

  private:
  bool m_directio;
};


}}



extern "C" {
SWC::FS::FileSystem* fs_make_new_local();
void fs_apply_cfg_local(SWC::Env::Config::Ptr env);
}

#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/fs/Local/FileSystem.cc"
#endif 


#endif  // swc_lib_fs_Local_FileSystem_h