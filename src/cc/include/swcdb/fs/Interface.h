/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Interface_h
#define swc_lib_fs_Interface_h


#include "swcdb/fs/FileSystem.h"

#if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Local/FileSystem.h"
#endif

/*
#if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Ceph/FileSystem.h"
#endif
*/

#if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Hadoop/FileSystem.h"
#endif

#if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
#include "swcdb/fs/Broker/FileSystem.h"
#endif



namespace SWC{ namespace FS {
  
typedef std::vector<int64_t> IdEntries_t;
const int   id_split_len = 3;
const char  id_split_last = 'g';

/// Interface to FileSystems

class Interface {
  public:
  
  typedef Interface* Ptr;

  Interface(Types::Fs typ=Types::Fs::NONE);
  
  FileSystem::Ptr use_filesystem();

  Ptr ptr();

  virtual ~Interface();

  const Types::Fs get_type();

  FileSystem::Ptr get_fs();

  const std::string to_string();

  void stop();

  void get_structured_ids(int &err, std::string base_path, 
                          IdEntries_t &entries, std::string base_id="");
  
  // default form to FS methods

  void readdir(int &err, const std::string& base_path, DirentList& dirs);

  bool exists(int &err, const std::string& name);

  void exists(Callback::ExistsCb_t cb, const std::string &name);

  void mkdirs(int &err, const std::string &name);

  void rmdir(int &err, const std::string &name);

  void rmdir_incl_opt_subs(int &err, const std::string &name, 
                           const std::string &up_to);
                           
  void remove(int &err, const std::string &name);
  
  void rename(int &err, const std::string &from , const std::string &to);

  void write(int &err, SmartFd::Ptr smartfd,
             int32_t replication, int64_t blksz, 
             StaticBuffer &buffer);
  
  bool open(int& err, SmartFd::Ptr smartfd);
  
  bool create(int& err, SmartFd::Ptr smartfd,
              int32_t bufsz, int32_t replication, int64_t blksz);
 
  private:
  Types::Fs       m_type;
  FileSystem::Ptr m_fs;

  struct LoadedDL{
    void* lib = nullptr;
    void* cfg = nullptr;
    void* make = nullptr;
  };
  LoadedDL loaded_dl;
};



void set_structured_id(std::string number, std::string &s);

} // namespace FS


namespace Env {

class FsInterface {
  
  public:
  
  typedef std::shared_ptr<FsInterface> Ptr;

  static void init();

  static Ptr get();

  static FS::Interface::Ptr interface();

  static FS::FileSystem::Ptr fs();

  FsInterface();

  virtual ~FsInterface();

  private:
  FS::Interface::Ptr  m_interface = nullptr;
  inline static Ptr   m_env = nullptr;
};
}

}


#ifdef SWC_IMPL_SOURCE
#include "../../../lib/swcdb/fs/Interface.cc"
#endif 


#endif  // swc_lib_fs_Interface_h