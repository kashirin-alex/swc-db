/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_CellStore_h
#define swc_ranger_db_CellStore_h

#include "swcdb/ranger/db/CellStoreBlock.h"


namespace SWC { namespace Ranger {

namespace CellStore {

/* file-format: 
    [blocks]: 
      header: i8(encoder), i32(enc-len), i32(len), i32(cells), 
              i32(checksum-data), i32(checksum)
      data:   [cell]
    [idx-blocks]:
      header:   i8(encoder), i32(enc-len), i32(len), 
                i32(checksum-data), i32(checksum) 
      idx-data: (1st ? prev_key_end,), vi32(blocks)
                [blocks]:
                  vi64(offset), interval, 
                  i8(encoder), vi32(enc-len), vi32(len), 
                  vi32(cells), vi32(checksum-data)
    trailer : i8(version), i32(cell-revs), i32(idx-blocks), i64(idx-offset), 
              i32(checksum)
      (trailer-offset) = fileLength - TRAILER_SIZE
*/


static const int8_t   VERSION = 1;
static const uint8_t  TRAILER_SIZE = 21;
static const uint8_t  IDX_BLKS_HEADER_SIZE = 17;


class Read final {
  public:
  typedef Read*  Ptr;

  inline static Ptr make(int& err, const csid_t csid, 
                         const RangePtr& range, 
                         const DB::Cells::Interval& interval, 
                         bool chk_base=false);

  static bool load_trailer(int& err, FS::SmartFd::Ptr& smartfd, 
                           uint32_t& cell_revs, 
                           uint32_t& blks_idx_count, 
                           uint64_t& blks_idx_offset, 
                           bool close_after=false, bool chk_base=false);

  static void load_blocks_index(int& err, FS::SmartFd::Ptr& smartfd, 
                                DB::Cell::Key& prev_key_end,
                                DB::Cells::Interval& interval, 
                                std::vector<Block::Read::Ptr>& blocks, 
                                bool chk_base=false);

  // 
  
  const csid_t                        csid;
  const DB::Cell::Key                 prev_key_end;
  const DB::Cells::Interval           interval;
  const std::vector<Block::Read::Ptr> blocks;

  explicit Read(const csid_t csid,
                const DB::Cell::Key& prev_key_end,
                const DB::Cells::Interval& interval, 
                const std::vector<Block::Read::Ptr>& blocks,
                const FS::SmartFd::Ptr& smartfd);

  Read(const Read&) = delete;

  Read(const Read&&) = delete;
  
  Read& operator=(const Read&) = delete;

  ~Read();

  size_t size_of() const;

  void load_cells(BlockLoader* loader);

  void run_queued();

  void get_blocks(int& err, std::vector<Block::Read::Ptr>& to) const;

  size_t release(size_t bytes);

  void release_fd();

  void close(int &err);

  void remove(int &err);

  bool processing() const;

  size_t size_bytes(bool only_loaded=false) const;

  size_t size_bytes_enc(bool only_loaded=false) const;

  size_t blocks_count() const;

  std::string to_string() const;

  private:

  void _run_queued();

  FS::SmartFd::Ptr     m_smartfd;
  QueueRunnable        m_queue;

};



class Write final {
  public:
  typedef std::shared_ptr<Write>  Ptr;

  const csid_t              csid;
  FS::SmartFd::Ptr          smartfd;
  Types::Encoding           encoder;
  uint32_t                  block_size;
  uint32_t                  cell_revs;
  size_t                    size;
  DB::Cells::Interval       interval;
  DB::Cell::Key             prev_key_end;
  
  //std::atomic<uint32_t>     completion;
 
  Write(const csid_t csid, const std::string& filepath, 
        const RangePtr& range, uint32_t cell_revs);

  ~Write();

  void create(int& err, 
              int32_t bufsz=-1, uint8_t blk_replicas=0, int64_t blksz=-1);

  void block_encode(int& err, DynamicBuffer& cells_buff, 
                    Block::Header& header);

  void block_write(int& err, DynamicBuffer& blk_buff, 
                   Block::Header& header);

  void finalize(int& err);

  void remove(int &err);

  std::string to_string();

  private:
  
  void block(int& err, DynamicBuffer& blk_buff);

  void write_blocks_index(int& err, uint32_t& blks_idx_count);

  void write_trailer(int& err);

  void close_and_validate(int& err);

  std::vector<Block::Write::Ptr> m_blocks;
};

typedef std::vector<Write::Ptr>   Writers;
typedef std::shared_ptr<Writers>  WritersPtr;




static Read::Ptr create_initial(int& err, const RangePtr& range);



}}}// namespace SWC::Ranger::CellStore

#endif // swc_ranger_db_CellStore_h