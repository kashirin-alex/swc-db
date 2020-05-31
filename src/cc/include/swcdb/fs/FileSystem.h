/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_FileSystem_h
#define swc_fs_FileSystem_h

#include "swcdb/fs/Settings.h"
#include "swcdb/fs/SmartFd.h"
#include "swcdb/fs/Dirent.h"
#include "swcdb/fs/Callbacks.h"


namespace SWC { 

namespace Types {
enum Fs {
  NONE,
  CUSTOM,
  LOCAL,
  HADOOP,
  HADOOP_JVM,
  CEPH,
  BROKER
};   
}

namespace FS {


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

std::string normalize_pathname(std::string s);

Types::Fs fs_type(std::string fs_name);

std::string type_to_string(Types::Fs typ);


class FileSystem : public std::enable_shared_from_this<FileSystem> {
  public:

  typedef std::shared_ptr<FileSystem> Ptr;

  const std::string path_root;
  const std::string path_data;

  const Property::V_GINT32::Ptr cfg_fds_max;

  std::atomic<size_t> fds_count;
  
  FileSystem(const Property::V_GINT32::Ptr cfg_fds_max, 
              bool setting_applied);
  
  FileSystem(const std::string& root,  
             const Property::V_GINT32::Ptr cfg_fds_max, 
             bool setting_applied);

  virtual ~FileSystem();

  virtual void stop();

  virtual Types::Fs get_type();
  
  virtual std::string to_string();

  virtual void get_abspath(const std::string &name, std::string& abspath);

  void fd_open_incr();

  void fd_open_decr();
  
  bool need_fds() const;

  size_t fds_open() const;
  
  virtual bool exists(int &err, const std::string &name) = 0;
  virtual void exists(Callback::ExistsCb_t cb, const std::string &name);

  virtual void remove(int &err, const std::string &name) = 0;
  virtual void remove(Callback::RemoveCb_t cb, const std::string &name);

  virtual size_t length(int &err, const std::string &name) = 0;
  virtual void length(Callback::LengthCb_t cb, const std::string &name);


  // Directory Actions
  virtual void mkdirs(int &err, const std::string &name) = 0;
  virtual void mkdirs(Callback::MkdirsCb_t cb, const std::string &name);

  virtual void readdir(int &err, const std::string &name, 
                       DirentList &results) = 0;
  virtual void readdir(Callback::ReaddirCb_t cb, const std::string &name);

  virtual void rmdir(int &err, const std::string &name) = 0;
  virtual void rmdir(Callback::RmdirCb_t cb, const std::string &name);
  
  virtual void rename(int &err, const std::string &from, 
                                const std::string &to) = 0;
  virtual void rename(Callback::RmdirCb_t cb, const std::string &from, 
                                      const std::string &to);

  // File(fd) Actions
  virtual void write(int &err, SmartFd::Ptr &smartfd,
                     uint8_t replication, int64_t blksz, 
                     StaticBuffer &buffer);
  virtual void write(Callback::WriteCb_t cb, SmartFd::Ptr &smartfd,
                     uint8_t replication, int64_t blksz, 
                     StaticBuffer &buffer);

  virtual void read(int &err, const std::string &name, StaticBuffer* dst);
  virtual void read(Callback::ReadAllCb_t cb, const std::string &name);

  virtual void create(int &err, SmartFd::Ptr &smartfd,
                      int32_t bufsz, uint8_t replication, int64_t blksz) = 0;
  virtual void create(Callback::CreateCb_t cb, SmartFd::Ptr &smartfd,
                      int32_t bufsz, uint8_t replication, int64_t blksz);

  virtual void open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz=0) = 0;
  virtual void open(Callback::OpenCb_t cb, SmartFd::Ptr &smartfd, int32_t bufsz=0);
  

  virtual size_t read(int &err, SmartFd::Ptr &smartfd, 
                      void *dst, size_t amount) = 0;
  virtual size_t read(int &err, SmartFd::Ptr &smartfd, 
                      StaticBuffer* dst, size_t amount);
  virtual void read(Callback::ReadCb_t cb, SmartFd::Ptr &smartfd, 
                    size_t amount);
  
  virtual size_t pread(int &err, SmartFd::Ptr &smartfd, 
                       uint64_t offset, void *dst, size_t amount) = 0;
  virtual size_t pread(int &err, SmartFd::Ptr &smartfd, 
                       uint64_t offset, StaticBuffer* dst, size_t amount);
  virtual void pread(Callback::PreadCb_t cb, SmartFd::Ptr &smartfd, 
                     uint64_t offset, size_t amount);

  virtual size_t append(int &err, SmartFd::Ptr &smartfd, 
                        StaticBuffer &buffer, Flags flags) = 0;
  virtual void append(Callback::AppendCb_t cb, SmartFd::Ptr &smartfd, 
                      StaticBuffer &buffer, Flags flags);

  virtual void seek(int &err, SmartFd::Ptr &smartfd, size_t offset) = 0;
  virtual void seek(Callback::CloseCb_t cb, SmartFd::Ptr &smartfd, 
                    size_t offset);

  virtual void flush(int &err, SmartFd::Ptr &smartfd) = 0;
  virtual void flush(Callback::FlushCb_t cb, SmartFd::Ptr &smartfd);

  virtual void sync(int &err, SmartFd::Ptr &smartfd) = 0;
  virtual void sync(Callback::SyncCb_t cb, SmartFd::Ptr &smartfd);

  virtual void close(int &err, SmartFd::Ptr &smartfd) = 0;
  virtual void close(Callback::CloseCb_t cb, SmartFd::Ptr &smartfd);

};

}}


extern "C"{
typedef SWC::FS::FileSystem* fs_make_new_t();

typedef void fs_apply_cfg_t(SWC::Env::Config::Ptr env);
}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/FileSystem.cc"
#endif 


#endif  // swc_fs_FileSystem_h