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

  RangeQuerySelect(const Comm::ConnHandlerPtr& conn, 
                   const Comm::Event::Ptr& ev, 
                   const DB::Specs::Interval& req_spec,
                   const RangePtr& range)
                  : ReqScan(conn, ev, req_spec), 
                    range(range) {
    if(!spec.value.empty())
      spec.col_type = range->cfg->column_type();
    if(!spec.flags.max_versions)
      spec.flags.max_versions = range->cfg->cell_versions();
    if(!spec.flags.max_buffer || 
       spec.flags.max_buffer > range->cfg->block_size())
      spec.flags.max_buffer = range->cfg->block_size();

    Env::Rgr::res().more_mem_usage(size_of());
  }

  virtual ~RangeQuerySelect() { 
    Env::Rgr::res().less_mem_usage(size_of());
  }

  size_t size_of() const {
    return sizeof(*this) + sizeof(Ptr) + spec.size_of_internal();
  }

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
    profile.add_cell((sz = cells.fill() - sz));
    Env::Rgr::res().more_mem_usage(sz);
    return !reached_limits();
  }

  void response(int &err) override {
    if(!err) {
      if(Env::Rgr::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      else if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED) {
      Env::Rgr::res().less_mem_usage(cells.fill());
      cells.free();
    }
    
    Comm::Protocol::Rgr::Params::RangeQuerySelectRsp params(
      err, err ? false : reached_limits(), offset);

    Comm::Buffers::Ptr cbp;
    if(!cells.empty()) {
      Env::Rgr::res().less_mem_usage(cells.fill());
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
      profile.print(SWC_LOG_OSTREAM << " Select-");
    );
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
#endif // swcdb_ranger_callbacks_RangeLocateScan_h
