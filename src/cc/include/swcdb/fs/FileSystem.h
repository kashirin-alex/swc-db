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
#include "swcdb/fs/Statistics.h"
#include "swcdb/fs/Logger.h"


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
  OPEN_FLAG_VERIFY_CHECKSUM = 0x00000004,
  WRITE_VALIDATE_LENGTH     = 0x00000008
};


enum Flags : uint8_t {
  NONE          = 0x00,
  FLUSH         = 0x01,
  SYNC          = 0x02
};


struct Configurables {
  Config::Settings::Ptr                 settings;
  Config::Property::Value_int32_g::Ptr  cfg_fds_max;
  std::string                           path_root;
  bool                                  stats_enabled;
  Configurables(const Config::Settings::Ptr& a_settings) noexcept
                : settings(a_settings),
                  cfg_fds_max(nullptr), stats_enabled(false) {
  }
  ~Configurables() noexcept { }
};


std::string normalize_pathname(std::string s);

Type fs_type(const std::string& fs_name);

const char* SWC_CONST_FUNC to_string(Type typ) noexcept;


class FileSystem : public std::enable_shared_from_this<FileSystem> {
  public:

  typedef std::shared_ptr<FileSystem> Ptr;

  static const uint32_t OPT_ASYNC_EXISTS      = 0x01;
  static const uint32_t OPT_ASYNC_REMOVE      = OPT_ASYNC_EXISTS << 1;
  static const uint32_t OPT_ASYNC_LENGTH      = OPT_ASYNC_REMOVE << 1;
  static const uint32_t OPT_ASYNC_MKDIRS      = OPT_ASYNC_LENGTH << 1;
  static const uint32_t OPT_ASYNC_READDIR     = OPT_ASYNC_MKDIRS << 1;
  static const uint32_t OPT_ASYNC_RMDIR       = OPT_ASYNC_READDIR << 1;
  static const uint32_t OPT_ASYNC_RENAME      = OPT_ASYNC_RMDIR << 1;
  static const uint32_t OPT_ASYNC_WRITE       = OPT_ASYNC_RENAME << 1;
  static const uint32_t OPT_ASYNC_COMBI_PREAD = OPT_ASYNC_WRITE << 1;
  static const uint32_t OPT_ASYNC_CREATE      = OPT_ASYNC_COMBI_PREAD << 1;
  static const uint32_t OPT_ASYNC_OPEN        = OPT_ASYNC_CREATE << 1;
  static const uint32_t OPT_ASYNC_READ        = OPT_ASYNC_OPEN << 1;
  static const uint32_t OPT_ASYNC_PREAD       = OPT_ASYNC_READ << 1;
  static const uint32_t OPT_ASYNC_APPEND      = OPT_ASYNC_PREAD << 1;
  static const uint32_t OPT_ASYNC_SEEK        = OPT_ASYNC_APPEND << 1;
  static const uint32_t OPT_ASYNC_FLUSH       = OPT_ASYNC_SEEK << 1;
  static const uint32_t OPT_ASYNC_SYNC        = OPT_ASYNC_FLUSH << 1;
  static const uint32_t OPT_ASYNC_CLOSE       = OPT_ASYNC_SYNC << 1;
  static const uint32_t OPT_ASYNC_READALL     = OPT_ASYNC_CLOSE << 1;
  static const uint32_t OPT_ASYNC_ALL         = OPT_ASYNC_READALL |
    OPT_ASYNC_CLOSE | OPT_ASYNC_SYNC | OPT_ASYNC_FLUSH | OPT_ASYNC_SEEK |
    OPT_ASYNC_APPEND | OPT_ASYNC_PREAD | OPT_ASYNC_READ | OPT_ASYNC_OPEN |
    OPT_ASYNC_CREATE | OPT_ASYNC_COMBI_PREAD | OPT_ASYNC_WRITE |
    OPT_ASYNC_RENAME | OPT_ASYNC_RMDIR | OPT_ASYNC_READDIR |
    OPT_ASYNC_MKDIRS | OPT_ASYNC_LENGTH | OPT_ASYNC_REMOVE | OPT_ASYNC_EXISTS;

  const uint32_t    impl_options_async;
  const std::string path_root;
  const std::string path_data;

  const Config::Settings::Ptr                settings;
  const Config::Property::Value_int32_g::Ptr cfg_fds_max;

  Core::AtomicBool  m_run;
  Statistics        statistics;

  FileSystem(const Configurables* config, uint32_t impl_opts_async);

  virtual ~FileSystem() noexcept;

  virtual void stop();

  bool SWC_PURE_FUNC has_option_async(uint32_t opts_async) const noexcept;

  virtual Type SWC_CONST_FUNC get_type() const noexcept;

  virtual Type get_type_underlying() const noexcept;

  virtual std::string to_string() const = 0;

  virtual void get_abspath(const std::string& name, std::string& abspath,
                           size_t reserve=0);

  void fd_open_incr() noexcept;

  void fd_open_decr() noexcept;

  bool need_fds() const noexcept;

  size_t fds_open() const noexcept;

  virtual bool exists(int& err, const std::string& name) = 0;
  virtual void exists(Callback::ExistsCb_t&& cb,
                      const std::string& name);

  virtual void remove(int& err, const std::string& name) = 0;
  virtual void remove(Callback::RemoveCb_t&& cb,
                      const std::string& name);

  virtual size_t length(int& err, const std::string& name) = 0;
  virtual void length(Callback::LengthCb_t&& cb,
                      const std::string& name);


  // Directory Actions
  virtual void mkdirs(int& err, const std::string& name) = 0;
  virtual void mkdirs(Callback::MkdirsCb_t&& cb,
                      const std::string& name);

  virtual void readdir(int& err, const std::string& name,
                       DirentList& results) = 0;
  virtual void readdir(Callback::ReaddirCb_t&& cb,
                       const std::string& name);

  virtual void rmdir(int& err, const std::string& name) = 0;
  virtual void rmdir(Callback::RmdirCb_t&& cb,
                     const std::string& name);

  virtual void rename(int& err, const std::string& from,
                                const std::string& to) = 0;
  virtual void rename(Callback::RmdirCb_t&& cb,
                      const std::string& from, const std::string& to);

  // File(fd) Actions
  virtual void write(int& err, SmartFd::Ptr& smartfd,
                     uint8_t replication, StaticBuffer& buffer) = 0;
  void default_write(int& err, SmartFd::Ptr& smartfd,
                     uint8_t replication, StaticBuffer& buffer);
  virtual void write(Callback::WriteCb_t&& cb, SmartFd::Ptr& smartfd,
                     uint8_t replication, StaticBuffer& buffer);

  virtual void read(int& err, const std::string& name, StaticBuffer* dst) = 0;
  void default_read(int& err, const std::string& name, StaticBuffer* dst);
  virtual void read(Callback::ReadAllCb_t&& cb,
                    const std::string& name);

  virtual void combi_pread(int& err, SmartFd::Ptr& smartfd,
                           uint64_t offset, uint32_t amount,
                           StaticBuffer* dst) = 0;
  void default_combi_pread(int& err, SmartFd::Ptr& smartfd,
                           uint64_t offset, uint32_t amount,
                           StaticBuffer* dst);
  virtual void combi_pread(Callback::CombiPreadCb_t&& cb,
                           SmartFd::Ptr& smartfd,
                           uint64_t offset, uint32_t amount);

  virtual void create(int& err, SmartFd::Ptr& smartfd,
                      uint8_t replication) = 0;
  virtual void create(Callback::CreateCb_t&& cb, SmartFd::Ptr& smartfd,
                      uint8_t replication);

  virtual void open(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void open(Callback::OpenCb_t&& cb, SmartFd::Ptr& smartfd);


  virtual size_t read(int& err, SmartFd::Ptr& smartfd,
                      void *dst, size_t amount) = 0;
  virtual size_t read(int& err, SmartFd::Ptr& smartfd,
                      StaticBuffer* dst, size_t amount) = 0;
  size_t default_read(int& err, SmartFd::Ptr& smartfd,
                      StaticBuffer* dst, size_t amount);
  virtual void read(Callback::ReadCb_t&& cb, SmartFd::Ptr& smartfd,
                    size_t amount);

  virtual size_t pread(int& err, SmartFd::Ptr& smartfd,
                       uint64_t offset, void *dst, size_t amount) = 0;
  virtual size_t pread(int& err, SmartFd::Ptr& smartfd,
                       uint64_t offset, StaticBuffer* dst, size_t amount) = 0;
  size_t default_pread(int& err, SmartFd::Ptr& smartfd,
                       uint64_t offset, StaticBuffer* dst, size_t amount);
  virtual void pread(Callback::PreadCb_t&& cb, SmartFd::Ptr& smartfd,
                     uint64_t offset, size_t amount);

  virtual size_t append(int& err, SmartFd::Ptr& smartfd,
                        StaticBuffer& buffer, Flags flags) = 0;
  virtual void append(Callback::AppendCb_t&& cb, SmartFd::Ptr& smartfd,
                      StaticBuffer& buffer, Flags flags);

  virtual void seek(int& err, SmartFd::Ptr& smartfd, size_t offset) = 0;
  virtual void seek(Callback::CloseCb_t&& cb, SmartFd::Ptr& smartfd,
                    size_t offset);

  virtual void flush(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void flush(Callback::FlushCb_t&& cb, SmartFd::Ptr& smartfd);

  virtual void sync(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void sync(Callback::SyncCb_t&& cb, SmartFd::Ptr& smartfd);

  virtual void close(int& err, SmartFd::Ptr& smartfd) = 0;
  virtual void close(Callback::CloseCb_t&& cb, SmartFd::Ptr& smartfd);

};

}}


extern "C"{
typedef SWC::FS::FileSystem* fs_make_new_t(SWC::FS::Configurables*);
}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/FileSystem.cc"
#endif


#endif // swcdb_fs_FileSystem_h
