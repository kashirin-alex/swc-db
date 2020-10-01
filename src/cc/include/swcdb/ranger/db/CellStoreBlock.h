/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CellStoreBlock_h
#define swcdb_ranger_db_CellStoreBlock_h

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
  
  bool load(BlockLoader* loader);

  void load(FS::SmartFd::Ptr smartfd, BlockLoader* loader);

  void load_cells(int& err, Ranger::Block::Ptr cells_block);

  void processing_decrement();

  size_t release();

  bool processing();

  int error();

  bool loaded();
  
  bool loaded(int& err);

  size_t size_bytes(bool only_loaded=false);

  size_t size_bytes_enc(bool only_loaded=false);

  void print(std::ostream& out);
  
  private:
  
  void _load(int& err, FS::SmartFd::Ptr smartfd);

  void _run_queued();

  Mutex                    m_mutex;
  State                    m_state;
  size_t                   m_processing;
  StaticBuffer             m_buffer;
  std::atomic<uint32_t>    m_cells_remain;
  int                      m_err;
  std::queue<BlockLoader*> m_queue;

};



class Write final {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const Header& header);

  ~Write();

  static void encode(int& err, DynamicBuffer& cells, DynamicBuffer& output, 
                     Header& header);

  void print(std::ostream& out) const;

  const Header  header;
  bool          released;

};



}}}} // namespace SWC::Ranger::CellStore::Block



#include "swcdb/ranger/db/CellStoreBlockHeader.cc"

#endif // swcdb_ranger_db_CellStoreBlock_h