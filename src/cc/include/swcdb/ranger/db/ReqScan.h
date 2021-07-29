/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_ReqScan_h
#define swcdb_ranger_db_ReqScan_h


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace Ranger {


#define SWC_SCAN_RSVD_BUFFS 2
// 4 == (blk + cs-blk + fragments) + intermediate-buffers


class ReqScan  : public DB::Cells::ReqScan {
  public:

  enum Type : uint8_t {
    QUERY,
    BLK_PRELOAD,
    COMPACTION
  };

  typedef std::shared_ptr<ReqScan>  Ptr;

  SWC_CAN_INLINE
  ReqScan(Type type=Type::QUERY, bool release_block=false,
          uint8_t readahead=1, uint32_t blk_size=0)
          noexcept
          : type(type),
            release_block(release_block), readahead(readahead),
            blk_size(blk_size),
            block(nullptr) {
    Env::Rgr::scan_reserved_bytes_add(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  SWC_CAN_INLINE
  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
          const DB::Cell::Key& range_begin, const DB::Cell::Key& range_end,
          uint32_t blk_size)
          : DB::Cells::ReqScan(conn, ev, range_begin, range_end),
            type(Type::QUERY),
            release_block(false), readahead(0),
            blk_size(blk_size),
            block(nullptr) {
    Env::Rgr::scan_reserved_bytes_add(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  SWC_CAN_INLINE
  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
          DB::Specs::Interval&& spec, uint32_t blk_size)
          : DB::Cells::ReqScan(conn, ev, std::move(spec)),
            type(Type::QUERY),
            release_block(false),
            readahead((!spec.flags.limit || spec.flags.offset)//?>block_cells
                        ? 2 : spec.flags.limit > 1),
            blk_size(blk_size),
            block(nullptr) {
    Env::Rgr::scan_reserved_bytes_add(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  virtual ~ReqScan() {
    Env::Rgr::scan_reserved_bytes_sub(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  SWC_CAN_INLINE
  Ptr get_req_scan() noexcept {
    return std::dynamic_pointer_cast<ReqScan>(shared_from_this());
  }

  virtual bool with_block() const noexcept {
    return false;
  }

  Type            type;
  bool            release_block;
  uint8_t         readahead;
  const uint32_t  blk_size;
  void*           block;
};



class ReqScanBlockLoader : public ReqScan {
  public:
  typedef std::shared_ptr<ReqScanBlockLoader>  Ptr;

  SWC_CAN_INLINE
  ReqScanBlockLoader(uint32_t blk_size) noexcept
      : ReqScan(ReqScan::Type::BLK_PRELOAD, false, 1, blk_size) {
  }

  virtual ~ReqScanBlockLoader() { }

  bool reached_limits() override {
    return true;
  }

  bool add_cell_and_more(const DB::Cells::Cell&) override {
    return !reached_limits();
  }
};



class ReqScanTest : public ReqScan {
  public:

  typedef std::shared_ptr<ReqScanTest>  Ptr;

  static Ptr make() { return Ptr(new ReqScanTest()); }

  ReqScanTest() noexcept { }

  bool reached_limits() override {
    return (spec.flags.limit && spec.flags.limit <= cells.size())   ||
      (spec.flags.max_buffer && spec.flags.max_buffer <= cells.size_bytes());
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    cells.add(cell, only_keys);
    return !reached_limits();
  }

  virtual ~ReqScanTest() { }

  void response(int &err) override {
    cb(err);
  }

  DB::Cells::Result        cells;
  std::function<void(int)> cb;
};

}}

#endif // swcdb_ranger_db_ReqScan_h
