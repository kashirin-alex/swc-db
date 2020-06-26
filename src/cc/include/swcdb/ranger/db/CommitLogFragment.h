/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_CommitLogFragment_h
#define swc_ranger_db_CommitLogFragment_h

#include "swcdb/core/Semaphore.h"
#include "swcdb/core/QueueRunnable.h"

namespace SWC { namespace Ranger { namespace CommitLog {

  
class Fragment final {

  /* file-format: 
        header:      i8(version), i32(header_ext-len), i32(checksum)
        header_ext:  interval, i8(encoder), i32(enc-len), i32(len), 
                     i32(cell_revs), i32(cells), 
                     i32(data-checksum), i32(checksum)
        data:        [cell]
  */

  public:

  typedef Fragment* Ptr;
  typedef const std::function<void(const DB::Cells::Cell&)> AddCell_t;

  static const uint8_t     HEADER_SIZE = 9;
  static const uint8_t     VERSION = 1;
  static const uint8_t     HEADER_EXT_FIXED_SIZE = 25;
  
  enum State {
    NONE,
    LOADING,
    LOADED,
    WRITING,
  };

  static std::string to_string(State state);


  static Ptr make_read(int& err, const std::string& filepath, 
                       const Types::KeySeq key_seq);

  static void load_header(int& err, FS::SmartFd::Ptr& smartfd, 
                          uint8_t& version,
                          DB::Cells::Interval& interval, 
                          Types::Encoding& encoder,
                          size_t& size_plain, size_t& size_enc,
                          uint32_t& cell_revs, uint32_t& cells_count,
                          uint32_t& data_checksum, uint32_t& offset_data);


  static Ptr make_write(int& err, const std::string& filepath, 
                        const DB::Cells::Interval& interval,
                        Types::Encoding encoder,
                        const uint32_t cell_revs, 
                        const uint32_t cells_count,
                        DynamicBuffer& cells, 
                        StaticBuffer::Ptr& buffer);

  static void write(int& err, FS::SmartFd::Ptr& smartfd, 
                    const uint8_t version,
                    const DB::Cells::Interval& interval, 
                    Types::Encoding& encoder,
                    const size_t size_plain, size_t& size_enc,
                    const uint32_t cell_revs, const uint32_t cells_count,
                    uint32_t& data_checksum, uint32_t& offset_data,
                    DynamicBuffer& cells, StaticBuffer::Ptr& buffer);


  const uint8_t               version;
  const DB::Cells::Interval   interval;
  const Types::Encoding       encoder;
  const size_t                size_plain;
  const size_t                size_enc;
  const uint32_t              cell_revs;
  const uint32_t              cells_count;
  const uint32_t              data_checksum;
  const uint32_t              offset_data;

  explicit Fragment(const FS::SmartFd::Ptr& smartfd, const uint8_t version,
                    const DB::Cells::Interval& interval, 
                    const Types::Encoding encoder,
                    const size_t size_plain, const size_t size_enc,
                    const uint32_t cell_revs, const uint32_t cells_count,
                    const uint32_t data_checksum, const uint32_t offset_data,
                    Fragment::State state);
  
  Fragment(const Fragment&) = delete;

  Fragment(const Fragment&&) = delete;
  
  Fragment& operator=(const Fragment&) = delete;

  Ptr ptr();

  ~Fragment();

  const std::string& get_filepath() const;

  void write(int err, uint8_t blk_replicas, int64_t blksz, 
             const StaticBuffer::Ptr& buff_write, Semaphore* sem);

  void load(const QueueRunnable::Call_t& cb);
  
  void load_cells(int& err, Ranger::Block::Ptr cells_block);
  
  void load_cells(int& err, DB::Cells::MutableVec& cells);

  void split(int& err, const DB::Cell::Key& key, 
             FragmentsPtr log_left, FragmentsPtr log_right);

  void processing_increment();

  void processing_decrement();
  
  size_t release();

  bool loaded();

  int error();

  bool loaded(int& err);

  size_t size_bytes() const;

  size_t size_bytes(bool only_loaded);

  size_t size_bytes_encoded();

  bool processing();

  void remove(int &err);

  std::string to_string();

  private:

  void load();

  void run_queued();

  void _run_queued();
  
  Mutex             m_mutex;
  State             m_state;
  FS::SmartFd::Ptr  m_smartfd;
  
  StaticBuffer      m_buffer;
  size_t            m_processing;
  int               m_err;

  std::atomic<uint32_t> m_cells_remain;
  QueueRunnable         m_queue;
  

};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogFragment_h