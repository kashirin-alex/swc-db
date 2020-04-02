/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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

  static Ptr make(const std::string& filepath, State state=State::NONE);

  const int64_t         ts;
  DB::Cells::Interval   interval;
  uint32_t              cells_count;

  explicit Fragment(const std::string& filepath, State state=State::NONE);
  
  Ptr ptr();

  ~Fragment();

  void write(int& err, uint8_t blk_replicas, Types::Encoding encoder, 
             DynamicBuffer& cells, uint32_t cell_revs, 
             Semaphore* sem);

  void write(int err, FS::SmartFd::Ptr smartfd, 
             uint8_t blk_replicas, int64_t blksz, StaticBuffer::Ptr buff_write,
             Semaphore* sem);

  void load_header(bool close_after=true);

  void load(const QueueRunnable::Call_t& cb);
  
  void load_cells(int& err, Ranger::Block::Ptr cells_block);
  
  void split(int& err, const DB::Cell::Key& key, 
             AddCell_t& left, AddCell_t& right);

  void processing_decrement();
  
  size_t release();

  bool loaded();

  int error();

  bool loaded(int& err);

  size_t size_bytes(bool only_loaded=false);

  size_t size_bytes_encoded();

  bool processing();

  void remove(int &err);

  std::string to_string();

  private:

  void load_header(int& err, bool close_after=true);

  void load();

  void run_queued();

  void _run_queued();
  
  LockAtomic::Unique m_mutex;
  //std::shared_mutex m_mutex;
  State             m_state;
  FS::SmartFd::Ptr  m_smartfd;
  uint8_t           m_version;
  Types::Encoding   m_encoder;
  size_t            m_size_enc;
  size_t            m_size;
  StaticBuffer      m_buffer;
  uint32_t          m_cell_revs;
  uint32_t          m_cells_offset;
  uint32_t          m_data_checksum;
  size_t            m_processing;
  int               m_err;

  std::atomic<uint32_t> m_cells_remain;
  QueueRunnable         m_queue;
  

};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogFragment_h