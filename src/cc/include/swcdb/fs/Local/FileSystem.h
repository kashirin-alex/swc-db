/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Local_FileSystem_h
#define swcdb_fs_Local_FileSystem_h

#include "swcdb/fs/FileSystem.h"

namespace SWC { namespace FS {


Configurables* apply_local();


class FileSystemLocal final : public FileSystem {
  public:

  FileSystemLocal(Configurables* config);

  virtual ~FileSystemLocal();

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
                        const std::string& to) override;

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

  void open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz=0) override;

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
  bool m_directio;
};


}}



extern "C" {
SWC::FS::FileSystem* fs_make_new_local(SWC::FS::Configurables* config);
}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Local/FileSystem.cc"
#endif


#endif // swcdb_fs_Local_FileSystem_h
