/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CommitLogFragment_h
#define swcdb_ranger_db_CommitLogFragment_h

#include "swcdb/core/Semaphore.h"

namespace SWC { namespace Ranger {


//! The SWC-DB CommitLog C++ namespace 'SWC::Ranger::CommitLog'
namespace CommitLog {


class Fragment final : public std::enable_shared_from_this<Fragment> {

  /* file-format:
        header:      i8(version), i32(header_ext-len), i32(checksum)
        header_ext:  interval, i8(encoder), i32(enc-len), i32(len),
                     i32(cell_revs), i32(cells),
                     i32(data-checksum), i32(checksum)
        data:        [cell]
  */

  public:

  typedef std::shared_ptr<Fragment>  Ptr;
  typedef std::function<void(Ptr&&)> LoadCb_t;

  static const uint8_t     HEADER_SIZE = 9;
  static const uint8_t     VERSION = 1;
  static const uint8_t     HEADER_EXT_FIXED_SIZE = 25;

  enum State : uint8_t {
    NONE,
    LOADING,
    LOADED,
    WRITING,
  };

  static const char* to_string(State state) noexcept;


  static Ptr make_read(int& err, std::string&& filepath,
                       const DB::Types::KeySeq key_seq);

  static Ptr make_read(int& err, FS::SmartFd::Ptr& smartfd,
                       const DB::Types::KeySeq key_seq);

  static void load_header(int& err, FS::SmartFd::Ptr& smartfd,
                          uint8_t& version,
                          DB::Cells::Interval& interval,
                          DB::Types::Encoder& encoder,
                          size_t& size_plain, size_t& size_enc,
                          uint32_t& cell_revs, uint32_t& cells_count,
                          uint32_t& data_checksum, uint32_t& offset_data);


  static Ptr make_write(int& err, std::string&& filepath,
                        DB::Cells::Interval&& interval,
                        DB::Types::Encoder encoder,
                        const uint32_t cell_revs,
                        const uint32_t cells_count,
                        DynamicBuffer& cells,
                        StaticBuffer::Ptr& buffer);

  static void write(int& err,
                    const uint8_t version,
                    const DB::Cells::Interval& interval,
                    DB::Types::Encoder& encoder,
                    const size_t size_plain, size_t& size_enc,
                    const uint32_t cell_revs, const uint32_t cells_count,
                    uint32_t& data_checksum, uint32_t& offset_data,
                    DynamicBuffer& cells, StaticBuffer::Ptr& buffer);


  const uint8_t                     version;
  const DB::Cells::Interval         interval;
  const DB::Types::Encoder          encoder;
  const size_t                      size_plain;
  const size_t                      size_enc;
  const uint32_t                    cell_revs;
  const uint32_t                    cells_count;
  const uint32_t                    data_checksum;
  const uint32_t                    offset_data;

  explicit Fragment(const FS::SmartFd::Ptr& smartfd, const uint8_t version,
                    DB::Cells::Interval&& interval,
                    const DB::Types::Encoder encoder,
                    const size_t size_plain, const size_t size_enc,
                    const uint32_t cell_revs, const uint32_t cells_count,
                    const uint32_t data_checksum, const uint32_t offset_data,
                    Fragment::State state) noexcept;

  Fragment(const Fragment&) = delete;

  Fragment(const Fragment&&) = delete;

  Fragment& operator=(const Fragment&) = delete;

  ~Fragment();

  Ptr ptr();

  size_t size_of() const noexcept;

  const std::string& get_filepath() const noexcept;

  void write(int err, uint8_t blk_replicas, int64_t blksz,
             const StaticBuffer::Ptr& buff_write, Core::Semaphore* sem);

  void load(LoadCb_t&& cb);

  void load_cells(int& err, Ranger::Block::Ptr cells_block);

  void load_cells(int& err, DB::Cells::MutableVec& cells);

  void split(int& err, const DB::Cell::Key& key,
             FragmentsPtr log_left, FragmentsPtr log_right);

  void processing_increment() noexcept;

  void processing_decrement() noexcept;

  size_t release();

  bool loaded() const noexcept;

  bool loaded(int& err) noexcept;

  size_t size_bytes() const noexcept;

  size_t size_bytes(bool only_loaded) const noexcept;

  size_t size_bytes_encoded() const noexcept;

  bool processing() noexcept;

  bool marked_removed() const noexcept;

  bool mark_removed() noexcept;

  void remove(int &err);

  void remove(int &err, Core::Semaphore* sem);

  void print(std::ostream& out);

  private:

  struct TaskLoadRead;

  void load_read(int err, const StaticBuffer::Ptr& buffer);

  void load_finish(int err);

  void run_queued();


  Core::MutexSptd                   m_mutex;
  Core::Atomic<State>               m_state;
  Core::Atomic<bool>                m_marked_removed;
  int                               m_err;
  Core::Atomic<size_t>              m_processing;
  Core::Atomic<uint32_t>            m_cells_remain;
  FS::SmartFd::Ptr                  m_smartfd;
  StaticBuffer                      m_buffer;
  std::queue<LoadCb_t>              m_queue;

};


}}} // namespace SWC::Ranger::CommitLog

#endif // swcdb_ranger_db_CommitLogFragment_h
