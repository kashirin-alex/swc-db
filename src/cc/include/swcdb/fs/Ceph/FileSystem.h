/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Ceph_FileSystem_h
#define swcdb_fs_Ceph_FileSystem_h

#include "swcdb/fs/FileSystem.h"
#include <cephfs/libcephfs.h>


namespace SWC { namespace FS {


Configurables apply_ceph();


class FileSystemCeph final : public FileSystem {
  public:


  FileSystemCeph();

  void setup_connection();

  bool initialize();

  virtual ~FileSystemCeph();

  void stop() override;

  Type get_type() const noexcept override;

  std::string to_string() const override;




  bool exists(int& err, const std::string& name) override;
  
  void remove(int& err, const std::string& name) override;

  size_t length(int& err, const std::string& name) override;

  void mkdirs(int& err, const std::string& name) override;

  void readdir(int& err, const std::string& name, 
                DirentList& results) override;

  void rmdir(int& err, const std::string& name) override;

  void rename(int& err, const std::string& from, 
                        const std::string& to)  override;


  void create(int& err, SmartFd::Ptr& smartfd, 
              int32_t bufsz, uint8_t replication, int64_t blksz) override;

  void open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz = -1) override;
  
  size_t read(int& err, SmartFd::Ptr& smartfd, 
              void *dst, size_t amount) override;

  size_t pread(int& err, SmartFd::Ptr& smartfd, 
               uint64_t offset, void *dst, size_t amount) override;

  size_t append(int& err, SmartFd::Ptr& smartfd, 
                StaticBuffer& buffer, Flags flags) override;

  void seek(int& err, SmartFd::Ptr& smartfd, size_t offset) override;

  void flush(int& err, SmartFd::Ptr& smartfd) override;

  void sync(int& err, SmartFd::Ptr& smartfd) override;

  void close(int& err, SmartFd::Ptr& smartfd) override;

  private:

  struct ceph_mount_info* m_filesystem;
  UserPerm*               m_perm;

  int ceph_cfg_min_obj_sz = 1048576;

};


}}


extern "C" {
SWC::FS::FileSystem* fs_make_new_ceph();
void fs_apply_cfg_ceph(SWC::Env::Config::Ptr env);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Ceph/FileSystem.cc"
#endif 

#endif // swcdb_fs_Ceph_FileSystem_h