/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelect_h
#define swcdb_ranger_callbacks_RangeQuerySelect_h

#include "swcdb/core/comm/ResponseCallback.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelect : public ReqScan {
  public:

  typedef std::shared_ptr<RangeQuerySelect> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelect(const Comm::ConnHandlerPtr& conn,
                   const Comm::Event::Ptr& ev,
                   DB::Specs::Interval&& req_spec,
                   const RangePtr& a_range)
                  : ReqScan(
                      conn, ev,
                      std::move(req_spec), a_range->cfg->block_size()
                    ),
                    range(a_range),
                    cells() {
    if(!spec.values.empty())
      spec.values.col_type = range->cfg->column_type();
    if(!spec.flags.max_versions)
      spec.flags.max_versions = range->cfg->cell_versions();
    if(!spec.flags.max_buffer || spec.flags.max_buffer > blk_size)
      spec.flags.max_buffer = blk_size;

    spec.apply_possible_range_pure();
    /** options not-useful for block-locator or a mid-stop
    if spec.range_begin < range : spec.range_begin.free()
    if spec.range_end > range : spec.range_end.free()
    SWC_LOG_OUT(LOG_INFO, spec.print(SWC_LOG_OSTREAM); );
    */
  }

  virtual ~RangeQuerySelect() noexcept { }

  SWC_CAN_INLINE
  void ensure_size() {
    if(profile.cells_count) {
      size_t avg = cells.fill() / profile.cells_count;
      if(cells.size + avg < spec.flags.max_buffer) {
        //return cells.ensure(spec.flags.max_buffer + (avg << 1));
        //if(!spec.flags.limit)
        //  return cells.ensure(spec.flags.max_buffer + avg);

        size_t add = cells.size + (avg <<= 2);
        cells.ensure((add += add >> 1) < spec.flags.max_buffer
          ? add
          : spec.flags.max_buffer + avg
        );
      }
    }
  }

  bool reached_limits() override {
    return (spec.flags.limit && spec.flags.limit <= profile.cells_count) ||
      (profile.cells_count &&
       spec.flags.max_buffer <= profile.cells_bytes +
                                profile.cells_bytes / profile.cells_count);
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    ensure_size();
    auto sz = cells.fill();
    cell.write(cells, only_keys);
    profile.add_cell(cells.fill() - sz);
    return !reached_limits();
  }

  void response(int &err) override {
    auto bytes_used = cells.size;
    if(!err) {
      if(Env::Rgr::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      else if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED)
      cells.free();

    Comm::Protocol::Rgr::Params::RangeQuerySelectRsp params(
      err, err ? false : reached_limits(), offset);

    Comm::Buffers::Ptr cbp;
    if(!cells.empty()) {
      if(!only_keys && spec.flags.is_only_keys()) {
        // rewrite cells without value
        DynamicBuffer no_value_cells;
        const uint8_t* ptr = cells.base;
        size_t remain = cells.fill();
        no_value_cells.ensure(remain);
        while(remain) {
          DB::Cells::Cell(&ptr, &remain, false).write(no_value_cells, true);
        }
        cells.take_ownership(no_value_cells);
      }
      StaticBuffer sndbuf(cells);
      cbp = Comm::Buffers::make(m_ev, params, sndbuf);
    } else {
      cbp = Comm::Buffers::make(m_ev, params);
    }
    m_conn->send_response(cbp);

    profile.finished();
    SWC_LOG_OUT(LOG_INFO,
      SWC_LOG_OSTREAM
        << "Range(" << range->cfg->cid  << '/' << range->rid << ") ";
      Error::print(SWC_LOG_OSTREAM, err);
      SWC_LOG_OSTREAM << " used-bytes=" << bytes_used;
      profile.print(SWC_LOG_OSTREAM << " Select-");
    );

    if(err == Error::RGR_NOT_LOADED_RANGE)
      range->issue_unload();
  }

  void print(std::ostream& out) const override {
    ReqScan::print(out << "ReqScan(");
    profile.print(out << ' ');
    out << ')';
  }

  RangePtr      range;
  DynamicBuffer cells;

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelect_h
