/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_ranger_callbacks_RangeQuerySelect_h
#define swc_ranger_callbacks_RangeQuerySelect_h

#include "swcdb/core/comm/ResponseCallback.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelect : public ReqScan {
  public:

  RangeQuerySelect(ConnHandlerPtr conn, Event::Ptr ev, 
                   const DB::Specs::Interval& spec, 
                   const DB::Cells::ReqScan::Config& cfg,
                   RangePtr range)
                  : ReqScan(conn, ev, spec, cfg), 
                    range(range), cells_count(0) {
  }

  virtual ~RangeQuerySelect() { }

  void ensure_size() {
    if(!cells_count || cells.size >= cfg.buffer)
      return;
    size_t add = (cells.fill() / cells_count) 
                  * (spec.flags.limit ? spec.flags.limit : 3);
    if(cells.size + add < cfg.buffer)
      cells.ensure(add);
  }

  bool reached_limits() override {
    return (spec.flags.limit && 
            spec.flags.limit <= cells_count) || 
           (cfg.buffer && cells_count &&
            cfg.buffer <= cells.fill() + cells.fill() / cells_count);
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    ensure_size();
    ++cells_count;
    cell.write(cells, only_keys);
    return !reached_limits();
  }

  bool add_cell_set_last_and_more(const DB::Cells::Cell& cell) override {
    ensure_size();
    ++cells_count;
    cells.set_mark();
    cell.write(cells, only_keys);

    const uint8_t* ptr = cells.mark + 1; // key at flag offset
    size_t remain = cells.ptr - ptr;
    key_last.decode(&ptr, &remain, false);
    return !reached_limits();
  }

  bool matching_last(const DB::Cell::Key& key) override {
    return cells_count ? key_last.equal(key) : false;
  }

  void response(int &err) override {
      
    if(!err) {
      if(RangerEnv::is_shuttingdown())
        err = Error::SERVER_SHUTTING_DOWN;
      else if(range->deleted())
        err = Error::COLUMN_MARKED_REMOVED;
    }
    if(err == Error::COLUMN_MARKED_REMOVED)
      cells.free();
    
    Protocol::Rgr::Params::RangeQuerySelectRsp params(
      err, err ? false : reached_limits(), offset);

    CommBuf::Ptr cbp;
    if(!cells.empty()) {
      StaticBuffer sndbuf(cells);
      cbp = CommBuf::make(params, sndbuf);
    } else {
      cbp = CommBuf::make(params);
    }
    cbp->header.initialize_from_request_header(m_ev->header);
    
    try {
      m_conn->send_response(cbp);
    }
    catch (Exception &e) {
      SWC_LOG_OUT(LOG_ERROR) << e << SWC_LOG_OUT_END;
    }
    
  }

  std::string to_string() const override {
    std::string s("ReqScan(");
    s.append(ReqScan::to_string());
    s.append(" cells(c=");
    s.append(std::to_string(cells_count));
    s.append(" sz=");
    s.append(std::to_string(cells.fill()));
    s.append(")");
    return s;
  }

  RangePtr  range;

  DynamicBuffer          cells;
  uint64_t               cells_count;
  DB::Cell::Key          key_last;

};


}}}
#endif // swc_ranger_callbacks_RangeLocateScan_h
