/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Interface_h
#define swc_fs_Interface_h


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
  
typedef std::vector<int64_t> IdEntries_t;
const int   id_split_len = 3;
const char  id_split_last = 'g';

/// Interface to FileSystems

class Interface {
  public:

  typedef Interface* Ptr;

  Interface(Type typ);
  
  FileSystem::Ptr use_filesystem();

  Ptr ptr();

  virtual ~Interface();

  Type get_type();

  FileSystem::Ptr get_fs();

  std::string to_string();

  bool need_fds() const;
  
  void stop();

  void get_structured_ids(int& err, const std::string& base_path, 
                          IdEntries_t& entries, 
                          const std::string& base_id="");
  
  // default form to FS methods

  void readdir(int& err, const std::string& base_path, DirentList& dirs);

  bool exists(int& err, const std::string& name);

  void exists(const Callback::ExistsCb_t& cb, const std::string& name);

  void mkdirs(int& err, const std::string& name);

  void rmdir(int& err, const std::string& name);

  void rmdir_incl_opt_subs(int& err, const std::string& name, 
                           const std::string& up_to);
                           
  void remove(int& err, const std::string& name);
  
  void rename(int& err, const std::string& from , const std::string& to);

  size_t length(int& err, const std::string& name);
  
  void read(int& err, const std::string& name, StaticBuffer* dst);

  void write(int& err, SmartFd::Ptr smartfd,
             uint8_t replication, int64_t blksz, 
             StaticBuffer& buffer);
  
  bool open(int& err, SmartFd::Ptr& smartfd);
  
  bool create(int& err, SmartFd::Ptr& smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz);
              
  void close(int& err, SmartFd::Ptr smartfd);
 
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

class FsInterface {
  
  public:
  
  typedef std::shared_ptr<FsInterface> Ptr;

  static void init(FS::Type typ);

  static Ptr get();

  static FS::Interface::Ptr interface();

  static FS::FileSystem::Ptr fs();

  static void reset();

  FsInterface(FS::Type typ);

  virtual ~FsInterface();

  private:
  FS::Interface::Ptr  m_interface = nullptr;
  inline static Ptr   m_env = nullptr;
};
}

}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Interface.cc"
#endif 


#endif  // swc_fs_Interface_h