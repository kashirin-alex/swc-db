/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_HadoopJVM_FileSystem_h
#define swc_fs_HadoopJVM_FileSystem_h

#include "swcdb/fs/FileSystem.h"
#include <hdfs.h>

namespace SWC{ namespace FS {

bool apply_hadoop_jvm();



struct SmartFdHadoopJVM : public SmartFd {
  public:
  
  typedef std::shared_ptr<SmartFdHadoopJVM> Ptr;
  
  static Ptr make_ptr(const std::string &filepath, uint32_t flags);

  static Ptr make_ptr(SmartFd::Ptr &smart_fd);

  SmartFdHadoopJVM(const std::string &filepath, uint32_t flags,
                int32_t fd=-1, uint64_t pos=0);

  virtual ~SmartFdHadoopJVM();

  hdfsFile file = 0;
};


 
class FileSystemHadoopJVM: public FileSystem {
  public:

  FileSystemHadoopJVM();

  void setup_connection();

  bool initialize();

  virtual ~FileSystemHadoopJVM();

  void stop() override;

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
                        const std::string &to)  override;

  SmartFdHadoopJVM::Ptr get_fd(SmartFd::Ptr &smartfd);

  void create(int &err, SmartFd::Ptr &smartfd, 
              int32_t bufsz, uint8_t replication, int64_t blksz) override;

  void open(int &err, SmartFd::Ptr &smartfd, int32_t bufsz = -1) override;
  
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
	hdfsFS                m_filesystem;
  std::atomic<bool>     m_run;
  std::atomic<int32_t>  m_nxt_fd;
};


}}



extern "C" { 
SWC::FS::FileSystem* fs_make_new_hadoop_jvm();
void fs_apply_cfg_hadoop_jvm(SWC::Env::Config::Ptr env);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/HadoopJVM/FileSystem.cc"
#endif 


#endif  // swc_fs_HadoopJVM_FileSystem_h