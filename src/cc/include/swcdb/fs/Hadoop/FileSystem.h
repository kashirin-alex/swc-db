/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Hadoop_FileSystem_h
#define swc_fs_Hadoop_FileSystem_h

#include <asio.hpp>
#include "swcdb/fs/FileSystem.h"
#include "hdfspp/hdfspp.h"

namespace SWC{ namespace FS {

bool apply_hadoop();



struct SmartFdHadoop : public SmartFd {
  public:
  
  typedef std::shared_ptr<SmartFdHadoop> Ptr;
  
  static Ptr make_ptr(const std::string &filepath, uint32_t flags);

  static Ptr make_ptr(SmartFd::Ptr &smart_fd);

  SmartFdHadoop(const std::string &filepath, uint32_t flags,
                int32_t fd=-1, uint64_t pos=0);

  virtual ~SmartFdHadoop();

  hdfs::FileHandle* file = nullptr;
};


 
class FileSystemHadoop: public FileSystem {
  public:

  FileSystemHadoop();

  void setup_connection();

  bool initialize();

  virtual ~FileSystemHadoop();

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

  SmartFdHadoop::Ptr get_fd(SmartFd::Ptr &smartfd);

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
	hdfs::FileSystem*     m_filesystem;
  std::atomic<bool>     m_run;
  std::atomic<int32_t>  m_nxt_fd;
};


}}



extern "C" { 
SWC::FS::FileSystem* fs_make_new_hadoop();
void fs_apply_cfg_hadoop(SWC::Env::Config::Ptr env);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Hadoop/FileSystem.cc"
#endif 


#endif  // swc_fs_Hadoop_FileSystem_h