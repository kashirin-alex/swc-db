/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_ranger_callbacks_RangeQuerySelect_h
#define swc_ranger_callbacks_RangeQuerySelect_h

#include "swcdb/core/comm/ResponseCallback.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelect : public ReqScan {
  public:

  RangeQuerySelect(const ConnHandlerPtr& conn, const Event::Ptr& ev, 
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

    RangerEnv::res().more_mem_usage(size_of());
  }

  virtual ~RangeQuerySelect() { 
    RangerEnv::res().less_mem_usage(size_of());
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
    RangerEnv::res().more_mem_usage(sz);
    return !reached_limits();
  }

  void response(int &err) override {
    if(!err) {
      if(RangerEnv::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      else if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED) {
      RangerEnv::res().less_mem_usage(cells.fill());
      cells.free();
    }
    
    Protocol::Rgr::Params::RangeQuerySelectRsp params(
      err, err ? false : reached_limits(), offset);

    CommBuf::Ptr cbp;
    if(!cells.empty()) {
      RangerEnv::res().less_mem_usage(cells.fill());
      StaticBuffer sndbuf(cells);
      cbp = CommBuf::make(params, sndbuf);
    } else {
      cbp = CommBuf::make(params);
    }
    cbp->header.initialize_from_request_header(m_ev->header);
    
    try {
      m_conn->send_response(cbp);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }

    profile.finished();
    SWC_LOGF(LOG_INFO, "Range(%lu/%lu) err=%d(%s) Select-%s", 
      range->cfg->cid, range->rid, err, Error::get_text(err),
      profile.to_string().c_str());
  }

  std::string to_string() const override {
    std::string s("ReqScan(");
    s.append(ReqScan::to_string());
    s.append(" ");
    s.append(profile.to_string());
    return s;
  }

  RangePtr      range;
  DynamicBuffer cells;

};


}}}
#endif // swc_ranger_callbacks_RangeLocateScan_h
