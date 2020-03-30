/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CellStore_h
#define swc_ranger_db_CellStore_h

#include "swcdb/ranger/db/CellStoreBlock.h"


namespace SWC { namespace Ranger {

namespace CellStore {

/* file-format: 
    [blocks]: 
      header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
      data:   [cell]
    blocks-index:
      header: i8(encoder), i32(enc-len), vi32(len), vi32(blocks), i32(checksum)
      data:   prev_key_end, [vi64(offset), interval, i32(checksum)]
    trailer : i8(version), i32(blocks-index-len), i32(cell-revs), i32(checksum)
      (trailer-offset) = fileLength - TRAILER_SIZE
      (blocks-index-offset) = (trailer-offset) - (blocks-index-len)
*/


static const uint8_t  TRAILER_SIZE=13;
static const int8_t   VERSION=1;



class Read final {
  public:
  typedef Read*  Ptr;

  inline static Ptr make(int& err, const uint32_t id, 
                         const RangePtr& range, 
                         const DB::Cells::Interval& interval, 
                         bool chk_base=false);

  static bool load_trailer(int& err, FS::SmartFd::Ptr smartfd, 
                           size_t& blks_idx_size, 
                           uint32_t& cell_revs, 
                           uint64_t& blks_idx_offset, 
                           bool close_after=false, bool chk_base=false);

  static void load_blocks_index(int& err, FS::SmartFd::Ptr smartfd, 
                                DB::Cell::Key& prev_key_end,
                                DB::Cells::Interval& interval, 
                                std::vector<Block::Read::Ptr>& blocks, 
                                bool chk_base=false);

  // 
  
  const uint32_t                      id;
  const DB::Cell::Key                 prev_key_end;
  const DB::Cells::Interval           interval;
  const std::vector<Block::Read::Ptr> blocks;

  explicit Read(const uint32_t id,
                const DB::Cell::Key& prev_key_end,
                const DB::Cells::Interval& interval, 
                const std::vector<Block::Read::Ptr>& blocks,
                FS::SmartFd::Ptr smartfd);

  Ptr ptr();

  ~Read();

  void load_cells(BlockLoader* loader);

  void run_queued();

  void get_blocks(int& err, std::vector<Block::Read::Ptr>& to) const;

  size_t release(size_t bytes);

  void close(int &err);

  void remove(int &err);

  const bool processing();

  const size_t size_bytes(bool only_loaded=false) const;

  const size_t blocks_count() const;

  const std::string to_string();

  private:

  const bool _processing() const;

  void _run_queued();

  std::shared_mutex                   m_mutex;
  bool                                m_q_runs = false;
  std::queue<std::function<void()>>   m_queue;
  FS::SmartFd::Ptr                    m_smartfd;

};



class Write : public std::enable_shared_from_this<Write> {
  public:
  typedef std::shared_ptr<Write>  Ptr;

  const uint32_t            id;
  FS::SmartFd::Ptr          smartfd;
  Types::Encoding           encoder;
  uint32_t                  cell_revs;
  size_t                    size;
  DB::Cells::Interval       interval;
  DB::Cell::Key             prev_key_end;
  
  //std::atomic<uint32_t>     completion;
 
  Write(const uint32_t id, const std::string& filepath, uint32_t cell_revs,
        Types::Encoding encoder=Types::Encoding::PLAIN);

  virtual ~Write();

  void create(int& err, 
              int32_t bufsz=-1, uint8_t blk_replicas=0, int64_t blksz=-1);

  void block(int& err, const DB::Cells::Interval& blk_intval, 
             DynamicBuffer& cells_buff, uint32_t cell_count);

  uint32_t write_blocks_index(int& err);

  void write_trailer(int& err);

  void close_and_validate(int& err);

  void finalize(int& err);

  void remove(int &err);

  const std::string to_string();

  private:

  std::vector<Block::Write::Ptr> m_blocks;
};

typedef std::vector<Write::Ptr>   Writers;
typedef std::shared_ptr<Writers>  WritersPtr;




static Read::Ptr 
create_init_read(int& err, Types::Encoding encoding, RangePtr range);



}}}// namespace SWC::Ranger::CellStore

#endif // swc_ranger_db_CellStore_h