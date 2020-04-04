/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CellStoreBlock_h
#define swc_ranger_db_CellStoreBlock_h

#include "swcdb/core/QueueRunnable.h"


namespace SWC { namespace Ranger { namespace CellStore {


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), 
              i32(cells), i32(checksum-data), i32(checksum)
      data:   [cell]
*/

static const uint8_t HEADER_SIZE=21;



class Read final {  
  public:
  typedef Read* Ptr;

  enum State {
    NONE,
    LOADING,
    LOADED
  };

  static std::string to_string(const State state);
  
  static Ptr make(const uint64_t offset, 
                  const DB::Cells::Interval& interval, 
                  uint32_t cell_revs);

  const uint64_t             offset;
  const DB::Cells::Interval  interval;
  const uint32_t             cell_revs;

  explicit Read(const uint64_t offset, const DB::Cells::Interval& interval, 
                uint32_t cell_revs);
  
  Ptr ptr();

  ~Read();
  
  bool load(const QueueRunnable::Call_t& cb);

  void load(FS::SmartFd::Ptr smartfd, const QueueRunnable::Call_t& cb);

  void load_cells(int& err, Ranger::Block::Ptr cells_block);

  void processing_decrement();

  size_t release();

  bool processing();

  int error();

  bool loaded();
  
  bool loaded(int& err);

  size_t size_bytes(bool only_loaded=false);

  std::string to_string();

  
  private:
  
  void load(int& err, FS::SmartFd::Ptr smartfd);

  void run_queued();

  std::mutex                  m_mutex;
  State                       m_state;
  size_t                      m_processing;
  bool                        m_loaded_header;
  Types::Encoding             m_encoder;
  size_t                      m_size;
  size_t                      m_sz_enc;
  uint32_t                    m_cells_count;
  uint32_t                    m_checksum_data;
  StaticBuffer                m_buffer;
  std::atomic<uint32_t>       m_cells_remain;
  int                         m_err;
  QueueRunnable               m_queue;
};



class Write final {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const uint64_t offset, const DB::Cells::Interval& interval, 
        const uint32_t cell_count);

  ~Write();

  void write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
             DynamicBuffer& output);

  std::string to_string();

  const uint64_t            offset;
  const DB::Cells::Interval interval;
  const uint32_t            cell_count;

};



}}}} // namespace SWC::Ranger::CellStore::Block

#endif // swc_ranger_db_CellStoreBlock_h