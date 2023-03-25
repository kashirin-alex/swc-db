/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_ReqScan_h
#define swcdb_ranger_db_ReqScan_h


#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace Ranger {


#define SWC_SCAN_RSVD_BUFFS 3
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
  ReqScan(Type a_type=Type::QUERY, bool a_release_block=false,
          uint8_t a_readahead=1, uint32_t a_blk_size=0)
          noexcept
          : type(a_type),
            release_block(a_release_block),
            readahead(a_readahead),
            blk_size(a_blk_size),
            block(nullptr) {
    Env::Rgr::scan_reserved_bytes_add(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  SWC_CAN_INLINE
  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
          const DB::Cell::Key& range_begin, const DB::Cell::Key& range_end,
          uint32_t a_blk_size)
          : DB::Cells::ReqScan(conn, ev, range_begin, range_end),
            type(Type::QUERY),
            release_block(false), readahead(0),
            blk_size(a_blk_size),
            block(nullptr) {
    Env::Rgr::scan_reserved_bytes_add(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  SWC_CAN_INLINE
  ReqScan(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
          DB::Specs::Interval&& a_spec, uint32_t a_blk_size)
          : DB::Cells::ReqScan(conn, ev, std::move(a_spec)),
            type(Type::QUERY),
            release_block(false),
            readahead((!spec.flags.limit || spec.flags.offset)//?>block_cells
                        ? 2 : spec.flags.limit > 1),
            blk_size(a_blk_size),
            block(nullptr) {
    Env::Rgr::scan_reserved_bytes_add(blk_size * SWC_SCAN_RSVD_BUFFS);
  }

  SWC_CAN_INLINE
  ReqScan(ReqScan&&) = delete;
  ReqScan(const ReqScan&) = delete;
  ReqScan& operator=(const ReqScan&) = delete;
  ReqScan& operator=(ReqScan&&) = delete;

  virtual ~ReqScan() noexcept {
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
  ReqScanBlockLoader(uint32_t a_blk_size) noexcept
      : ReqScan(ReqScan::Type::BLK_PRELOAD, false, 1, a_blk_size) {
  }

  virtual ~ReqScanBlockLoader() noexcept { }

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

  ReqScanTest() noexcept : cells(), cb() { }

  bool reached_limits() override {
    return (spec.flags.limit && spec.flags.limit <= cells.size())   ||
      (spec.flags.max_buffer && spec.flags.max_buffer <= cells.size_bytes());
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    cells.add(cell, only_keys);
    return !reached_limits();
  }

  virtual ~ReqScanTest() noexcept { }

  void response(int &err) override {
    cb(err);
  }

  DB::Cells::Result        cells;
  std::function<void(int)> cb;
};

}}

#endif // swcdb_ranger_db_ReqScan_h
