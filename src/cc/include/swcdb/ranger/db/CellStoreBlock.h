/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_CellStoreBlock_h
#define swc_ranger_db_CellStoreBlock_h

#include "swcdb/core/QueueRunnable.h"
#include "swcdb/ranger/db/CellStoreBlockHeader.h"


namespace SWC { namespace Ranger { namespace CellStore {


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), 
              i32(cells), i32(checksum-data), i32(checksum)
      data:   [cell]
*/


class Read final {  
  public:
  typedef Read* Ptr;

  enum State {
    NONE,
    LOADING,
    LOADED
  };

  static std::string to_string(const State state);
  
  static Ptr make(int& err, FS::SmartFd::Ptr& smartfd, 
                  const DB::Cells::Interval& interval, const uint64_t offset,
                  const uint32_t cell_revs);

  static void load_header(int& err, FS::SmartFd::Ptr& smartfd, 
                          Header& header);


  const uint32_t             cell_revs;

  const Header               header;

  explicit Read(const uint32_t cell_revs, const Header& header);
  
  Read(const Read&) = delete;

  Read(const Read&&) = delete;
  
  Read& operator=(const Read&) = delete;

  ~Read();
  
  size_t size_of() const;
  
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

  size_t size_bytes_enc(bool only_loaded=false);

  std::string to_string();

  
  private:
  
  void load(int& err, FS::SmartFd::Ptr smartfd);

  void run_queued();

  Mutex                       m_mutex;
  State                       m_state;
  size_t                      m_processing;
  StaticBuffer                m_buffer;
  std::atomic<uint32_t>       m_cells_remain;
  int                         m_err;
  QueueRunnable               m_queue;
};



class Write final {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const Header& header);

  ~Write();

  static void encode(int& err, DynamicBuffer& cells, DynamicBuffer& output, 
                     Header& header);

  std::string to_string();

  const Header  header;

};



}}}} // namespace SWC::Ranger::CellStore::Block



#include "swcdb/ranger/db/CellStoreBlockHeader.cc"

#endif // swc_ranger_db_CellStoreBlock_h