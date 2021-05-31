/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Interface_h
#define swcdb_fs_Interface_h


#include "swcdb/fs/FileSystem.h"

#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Local/FileSystem.h"
#endif


#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Ceph/FileSystem.h"
#endif


#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Hadoop/FileSystem.h"
#endif

#if defined (BUILTIN_FS_HADOOP_JVM) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/HadoopJVM/FileSystem.h"
#endif


#if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Broker/FileSystem.h"
#endif



namespace SWC { namespace FS {

typedef std::vector<uint64_t> IdEntries_t;
const int   ID_SPLIT_LEN  = 3;
const char  ID_SPLIT_LAST = 'g';

/// Interface to FileSystems

class Interface final : public std::enable_shared_from_this<Interface> {
  public:

  typedef std::shared_ptr<Interface> Ptr;

  Interface(const Config::Settings::Ptr& settings, Type typ);

  FileSystem::Ptr use_filesystem(const Config::Settings::Ptr& settings);

  Ptr ptr() noexcept {
    return shared_from_this();
  }

  ~Interface();

  Type get_type_underlying() const noexcept;

  FileSystem::Ptr& get_fs()  noexcept {
    return m_fs;
  }

  std::string to_string() const;

  bool need_fds() const noexcept;

  void stop();

  void get_structured_ids(int& err, const std::string& base_path,
                          IdEntries_t& entries,
                          const std::string& base_id="");

  // default form to FS methods

  void readdir(int& err, const std::string& base_path, DirentList& dirs);

  bool exists(int& err, const std::string& name);

  void exists(Callback::ExistsCb_t&& cb, const std::string& name);

  void mkdirs(int& err, const std::string& name);

  void rmdir(int& err, const std::string& name);

  void rmdir_incl_opt_subs(int& err, const std::string& name,
                           const std::string& up_to);

  void remove(int& err, const std::string& name);

  void remove(Callback::RemoveCb_t&& cb, const std::string& name);

  void rename(int& err, const std::string& from , const std::string& to);

  size_t length(int& err, const std::string& name);

  void read(int& err, const std::string& name, StaticBuffer* dst);

  void write(int& err, SmartFd::Ptr smartfd,
             uint8_t replication, int64_t blksz,
             StaticBuffer& buffer);

  bool open(int& err, SmartFd::Ptr& smartfd);

  bool create(int& err, SmartFd::Ptr& smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz);

  void close(int& err, SmartFd::Ptr& smartfd);

  void close(Callback::CloseCb_t&& cb, SmartFd::Ptr& smartfd);

  private:
  Type            m_type;
  FileSystem::Ptr m_fs;

  struct LoadedDL final {
    void* lib = nullptr;
    void* cfg = nullptr;
    void* make = nullptr;
  };
  LoadedDL loaded_dl;
};



void set_structured_id(const std::string& number, std::string& s);

} // namespace FS



namespace Env {

class FsInterface final {

  public:

  typedef std::shared_ptr<FsInterface> Ptr;

  static void init(const SWC::Config::Settings::Ptr& settings, FS::Type typ);

  static Ptr& get() noexcept {
    return m_env;
  }

  static FS::Interface::Ptr& interface() noexcept {
    return m_env->m_interface;
  }

  static FS::FileSystem::Ptr& fs() noexcept {
    return m_env->m_interface->get_fs();
  }

  static void reset() noexcept {
    m_env = nullptr;
  }

  FsInterface(const SWC::Config::Settings::Ptr& settings, FS::Type typ);

  //~FsInterface() { }

  private:
  FS::Interface::Ptr  m_interface = nullptr;
  inline static Ptr   m_env = nullptr;
};


} // Env

} // SWC


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Interface.cc"
#endif


#endif // swcdb_fs_Interface_h
