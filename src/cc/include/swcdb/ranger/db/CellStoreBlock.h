/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CellStoreBlock_h
#define swcdb_ranger_db_CellStoreBlock_h

#include "swcdb/ranger/db/CellStoreBlockHeader.h"


namespace SWC { namespace Ranger { namespace CellStore {

// Forward Declaration
class Read;


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), 
              i32(cells), i32(checksum-data), i32(checksum)
      data:   [cell]
*/


class Read final {  
  public:
  typedef Read* Ptr;

  enum State : uint8_t {
    NONE,
    LOADING,
    LOADED
  };

  static const char* to_string(const State state) noexcept;
  
  /*
  static Ptr make(int& err, CellStore::Read* cellstore,
                  const DB::Cells::Interval& interval,
                  const uint64_t offset);
  */

  static void load_header(int& err, FS::SmartFd::Ptr& smartfd, 
                          Header& header);

  const Header         header;
  CellStore::Read*     cellstore;

  explicit Read(const Header& header);

  void init(CellStore::Read* cellstore);

  Read(const Read&) = delete;

  Read(const Read&&) = delete;
  
  Read& operator=(const Read&) = delete;

  ~Read();
  
  size_t size_of() const noexcept;
  
  bool load(BlockLoader* loader);

  void load();

  void load_cells(int& err, Ranger::Block::Ptr cells_block);

  void processing_decrement() noexcept;

  size_t release();

  bool processing();

  int error() const noexcept;

  bool loaded() const noexcept;
  
  bool loaded(int& err) const noexcept;

  size_t size_bytes(bool only_loaded=false) const noexcept;

  size_t size_bytes_enc(bool only_loaded=false) const noexcept;

  void print(std::ostream& out);
  
  private:
  
  void load_open(int err);

  void load_read(int err, const StaticBuffer::Ptr& buffer);

  void load_finish(int err);


  Core::Atomic<State>      m_state;
  Core::Atomic<int>        m_err;
  Core::Atomic<uint32_t>   m_cells_remain;
  Core::Atomic<size_t>     m_processing;

  StaticBuffer             m_buffer;

  Core::MutexSptd          m_mutex;
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