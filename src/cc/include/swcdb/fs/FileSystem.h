/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_FileSystem_h
#define swcdb_fs_FileSystem_h

#include "swcdb/fs/Settings.h"
#include "swcdb/fs/SmartFd.h"
#include "swcdb/fs/Dirent.h"
#include "swcdb/fs/Callbacks.h"


namespace SWC { namespace FS {


enum Type : uint8_t {

  UNKNOWN     = 0x00,
  LOCAL       = 0x01,
  BROKER      = 0x02,
  CUSTOM      = 0x03,
  HADOOP      = 0x04,
  HADOOP_JVM  = 0x05,
  CEPH        = 0x06,

};


enum OpenFlags {
  OPEN_FLAG_DIRECTIO        = 0x00000001,
  OPEN_FLAG_OVERWRITE       = 0x00000002,
  OPEN_FLAG_VERIFY_CHECKSUM = 0x00000004
};


enum Flags : uint8_t {
  NONE  = 0x00,
  FLUSH = 0x01,
  SYNC  = 0x02
};


struct Configurables {
  Config::Property::V_GINT32::Ptr cfg_fds_max = nullptr;
  std::string                     path_root;
};


std::string normalize_pathname(std::string s);

Type fs_type(std::string fs_name);

const char* to_string(Type typ) noexcept;


class FileSystem : public std::enable_shared_from_this<FileSystem> {
  public:

  typedef std::shared_ptr<FileSystem> Ptr;

  const std::string path_root;
  const std::string path_data;

  const Config::Property::V_GINT32::Ptr cfg_fds_max;

  Core::Atomic<size_t>  fds_count;
  Core::AtomicBool      m_run;
  
  FileSystem(const Configurables& config);

  virtual ~FileSystem();

  virtual void stop();

  virtual Type get_type() const noexcept;

  virtual std::string to_string() const;

  virtual void get_abspath(const std::string& name, std::string& abspath);

  void fd_open_incr();

  void fd_open_decr();
  
  bool need_fds() const;

  size_t fds_open() const;
  
  virtual bool exists(int& err, const std::string& name) = 0;
  virtual void exists(const Callback::ExistsCb_t& cb, 
                      const std::string& name);

  virtual void remove(int& err, const std::string& name) = 0;
  virtual void remove(const Callback::RemoveCb_t& cb, 
                      const std::string& name);

  virtual size_t length(int& err, const std::string& name) = 0;
  virtual void length(const Callback::LengthCb_t& cb, 
                      const std::string& name);


  // Directory Actions
  virtual void mkdirs(int& err, const std::string& name) = 0;
  virtual void mkdirs(const Callback::MkdirsCb_t& cb, 
                      const std::string& name);

  virtual void readdir(int& err, const std::string& name, 
                       DirentList& results) = 0;
  virtual void readdir(const Callback::ReaddirCb_t& cb, 
                       const std::string& name);

  virtual void rmdir(int& err, const std::string& name) = 0;
  virtual void rmdir(const Callback::RmdirCb_t& cb, 
                     const std::string& name);
  
  virtual void rename(int& err, const std::string& from, 
                                const std::string& to) = 0;
  virtual void rename(const Callback::RmdirCb_t& cb, 
                      const std::string& from, const std::string& to);

  // File(fd) Actions
  virtual void write(int& err, SmartFd::Ptr& smartfd,
                     uint8_t replication, int64_t blksz, 
                     StaticBuffer& buffer);
  virtual void write(const Callback::WriteCb_t& cb, SmartFd::Ptr& smartfd,
                     uint8_t replication, int64_t blksz, 
                     StaticBuffer& buffer);

  virtual void read(int& err, const std::string& name, StaticBuffer* dst);
  virtual void read(const Callback::ReadAllCb_t& cb, 
                    const std::string& name);

  virtual void combi_pread(int& err, SmartFd::Ptr& smartfd,
                           uint64_t offset, uint32_t amount,
                           StaticBuffer* dst);
  virtual void combi_pread(const Callback::CombiPreadCb_t& cb,
                           SmartFd::Ptr& smartfd,
                           uint64_t offset, uint32_t amount);

  virtual void create(int& err, SmartFd::Ptr& smartfd,
                      int32_t bufsz, uint8_t replication, int64_t blksz) = 0;
  virtual void create(const Callback::CreateCb_t& cb, SmartFd::Ptr& smartfd,
                      int32_t bufsz, uint8_t replication, int64_t blksz);

  virtual void open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz=0) = 0;
  virtual void open(const Callback::OpenCb_t& cb, 
                    SmartFd::Ptr& smartfd, int32_t bufsz=0);
  

  virtual size_t read(int& err, SmartFd::Ptr& smartfd, 
                      void *dst, size_t amount) = 0;
  virtual size_t read(int& err, SmartFd::Ptr& smartfd, 
                      StaticBuffer* dst, size_t amount);
  virtual void read(const Callback::ReadCb_t& cb, SmartFd::Ptr& smartfd, 
                    size_t amount);
  
  virtual size_t pread(int& err, SmartFd::Ptr& smartfd, 
                       uint64_t offset, void *dst, size_t amount) = 0;
  virtual size_t pread(int& err, SmartFd::Ptr& smartfd, 
                       uint64_t offset, StaticBuffer* dst, size_t amount);
  virtual void pread(const Callback::PreadCb_t& cb, SmartFd::Ptr& smartfd, 
                     uint64_t offset, size_t amount);

  virtual size_t append(int& err, SmartFd::Ptr& smartfd, 
                        StaticBuffer& buffer, Flags flags) = 0;
  virtual void append(const Callback::AppendCb_t& cb, SmartFd::Ptr& smartfd, 
                      StaticBuffer& buffer, Flags flags);

  virtual void seek(int& err, SmartFd::Ptr& smartfd, size_t offset) = 0;
  virtual void seek(const Callback::CloseCb_t& cb, SmartFd::Ptr& smartfd, 
                    size_t offset);

  virtual void flush(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void flush(const Callback::FlushCb_t& cb, SmartFd::Ptr& smartfd);

  virtual void sync(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void sync(const Callback::SyncCb_t& cb, SmartFd::Ptr& smartfd);

  virtual void close(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void close(const Callback::CloseCb_t& cb, SmartFd::Ptr& smartfd);

};

}}


extern "C"{
typedef SWC::FS::FileSystem* fs_make_new_t();

typedef void fs_apply_cfg_t(SWC::Env::Config::Ptr env);
}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/FileSystem.cc"
#endif 


#endif // swcdb_fs_FileSystem_h