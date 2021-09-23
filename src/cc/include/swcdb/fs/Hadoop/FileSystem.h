/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Hadoop_FileSystem_h
#define swcdb_fs_Hadoop_FileSystem_h


#include "swcdb/fs/FileSystem.h"
#include "hdfspp/hdfspp.h"
#include <condition_variable>


namespace SWC { namespace FS {


Configurables* apply_hadoop(Configurables* config);


class FileSystemHadoop final : public FileSystem {
  public:

  struct Service {
    typedef std::shared_ptr<Service> Ptr;

    Service(hdfs::FileSystem* srv) noexcept : srv(srv) { }

    ~Service() noexcept {
      //if(srv) hdfsDisconnect(srv);
    }

    hdfs::FileSystem*     srv;
  };

  FileSystemHadoop(Configurables* config);

  virtual ~FileSystemHadoop() noexcept;

  Type SWC_CONST_FUNC get_type() const noexcept override;

  std::string to_string() const override;

  void stop() override;

  Service::Ptr setup_connection();

  bool initialize(Service::Ptr& fs);

  Service::Ptr get_fs(int& err);

  void need_reconnect(int& err, Service::Ptr& fs);


  bool exists(int& err, const std::string& name) override;

  void remove(int& err, const std::string& name) override;

  size_t length(int& err, const std::string& name) override;

  void mkdirs(int& err, const std::string& name) override;

  void readdir(int& err, const std::string& name,
                DirentList& results) override;

  void rmdir(int& err, const std::string& name) override;

  void rename(int& err, const std::string& from,
                        const std::string& to)  override;

  void write(int& err, SmartFd::Ptr& smartfd,
             uint8_t replication, int64_t blksz,
             StaticBuffer& buffer) override {
    default_write(err, smartfd, replication, blksz, buffer);
  }

  void read(int& err, const std::string& name, StaticBuffer* dst) override {
    default_read(err, name, dst);
  }

  void combi_pread(int& err, SmartFd::Ptr& smartfd,
                   uint64_t offset, uint32_t amount,
                   StaticBuffer* dst) override {
    default_combi_pread(err, smartfd, offset, amount, dst);
  }

  void create(int& err, SmartFd::Ptr& smartfd,
              int32_t bufsz, uint8_t replication, int64_t blksz) override;

  void open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz = -1) override;

  size_t read(int& err, SmartFd::Ptr& smartfd,
              void *dst, size_t amount) override;

  size_t read(int& err, SmartFd::Ptr& smartfd,
              StaticBuffer* dst, size_t amount) override {
    return default_read(err, smartfd, dst, amount);
  }

  size_t pread(int& err, SmartFd::Ptr& smartfd,
               uint64_t offset, void *dst, size_t amount) override;

  size_t pread(int& err, SmartFd::Ptr& smartfd, uint64_t offset,
               StaticBuffer* dst, size_t amount) override {
    return default_pread(err, smartfd, offset, dst, amount);
  }

  size_t append(int& err, SmartFd::Ptr& smartfd,
                StaticBuffer& buffer, Flags flags) override;

  void seek(int& err, SmartFd::Ptr& smartfd, size_t offset) override;

  void flush(int& err, SmartFd::Ptr& smartfd) override;

  void sync(int& err, SmartFd::Ptr& smartfd) override;

  void close(int& err, SmartFd::Ptr& smartfd) override;

  private:

  struct SmartFdHadoop final : public SmartFd {
    public:

    typedef std::shared_ptr<SmartFdHadoop> Ptr;

    static Ptr make_ptr(const std::string& filepath, uint32_t flags);

    static Ptr make_ptr(SmartFd::Ptr& smart_fd);

    SmartFdHadoop(const std::string& filepath, uint32_t flags,
                  int32_t fd=-1, uint64_t pos=0);

    virtual ~SmartFdHadoop() noexcept;

    hdfs::FileHandle* file() const;

    void file(hdfs::FileHandle* file);

    private:

    std::atomic<hdfs::FileHandle*> m_file;
  };

  SmartFdHadoop::Ptr get_fd(SmartFd::Ptr& smartfd);

  Core::Atomic<int32_t>   m_nxt_fd;

  std::mutex              m_mutex;
  std::condition_variable m_cv;
  bool                    m_connecting;

  Service::Ptr            m_fs;

  int hdfs_cfg_min_blk_sz = 1048576;
};


}}



extern "C" {
SWC::FS::FileSystem* fs_make_new_hadoop(SWC::FS::Configurables* config);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Hadoop/FileSystem.cc"
#endif


#endif // swcdb_fs_Hadoop_FileSystem_h
