/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeLocateScan_h
#define swcdb_ranger_callbacks_RangeLocateScan_h

#include "swcdb/ranger/db/ReqScan.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"

namespace SWC { namespace Ranger { namespace Callback {


class RangeLocateScan : public ReqScan {
  public:

  typedef std::shared_ptr<RangeLocateScan> Ptr;

  SWC_CAN_INLINE
  RangeLocateScan(const Comm::ConnHandlerPtr& conn,
                  const Comm::Event::Ptr& ev,
                  const DB::Cell::Key& a_range_begin,
                  const DB::Cell::Key& a_range_end,
                  const RangePtr& a_range,
                  uint8_t a_flags)
                  : ReqScan(
                      conn, ev,
                      a_range_begin, a_range_end,
                      a_range->cfg->block_size()
                    ),
                    range(a_range), flags(a_flags),
                    any_is(range->cfg->range_type != DB::Types::Range::DATA),
                    range_begin(a_range_begin, false) {
    auto c = range->known_interval_count();
    spec.range_begin.remove(c ? c : uint24_t(1), true);
    if(flags & Comm::Protocol::Rgr::Params::RangeLocateReq::KEY_EQUAL)
      spec.set_opt__key_equal();
    if(flags & Comm::Protocol::Rgr::Params::RangeLocateReq::RANGE_END_REST ||
       a_range_end.count == any_is)
      spec.set_opt__range_end_rest();
    /*
    SWC_PRINT << "--------------------- "
      << range->cfg->cid << '/' << range->rid
      << "\n\t" << spec.to_string() << SWC_PRINT_CLOSE;
    */
  }

  virtual ~RangeLocateScan() noexcept { }

  bool selector(const DB::Types::KeySeq key_seq,
                const DB::Cells::Cell& cell, bool& stop) override {
    //SWC_PRINT << "---------------------"
    //  << "  cell.key: " << cell.key.to_string() << SWC_PRINT_CLOSE;
    if(any_is &&
       DB::KeySeq::compare_upto(key_seq, spec.range_begin, cell.key, any_is)
        != Condition::EQ)
      return false;

    if(flags & Comm::Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE &&
        DB::KeySeq::compare(key_seq, spec.offset_key, cell.key)
          != Condition::GT)
      return false;

    if(flags & Comm::Protocol::Rgr::Params::RangeLocateReq::CURRENT_RANGE &&
        DB::KeySeq::compare(key_seq, spec.offset_key, cell.key)
          == Condition::LT)
      return false;

    if(cell.key.count > any_is && spec.range_end.count > any_is &&
       !spec.is_matching_end(key_seq, cell.key)) {
      stop = true;
      //SWC_PRINT << "-- KEY-BEGIN NO MATCH STOP --" << SWC_PRINT_CLOSE;
      return false;
    }

    StaticBuffer v;
    cell.get_value(v);
    const uint8_t* ptr = v.base;
    size_t remain = v.size;

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    DB::Cell::Key key_end;
    key_end.decode(&ptr, &remain, false);
    if(key_end.count > any_is && range_begin.count > any_is &&
       !spec.is_matching_begin(key_seq, key_end)) {
      //SWC_PRINT << "-- KEY-END NO MATCH --" << SWC_PRINT_CLOSE;
      return false;
    }
    //return true; // without aligned min-max

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    rid_t rid = Serialization::decode_vi64(&ptr, &remain);

    DB::Cell::Key dekey;
    // aligned_min
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    dekey.decode(&ptr, &remain, false);
    if(spec.range_end.count != any_is && !dekey.empty() &&
       !DB::KeySeq::compare(key_seq,
                            spec.range_end, dekey,
                            Condition::GT, spec.range_end.count, true)) {
      //SWC_PRINT << "-- ALIGNED MIN NO MATCH  --" << SWC_PRINT_CLOSE;
      return false;
    }

    // aligned_max
    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    dekey.decode(&ptr, &remain, false);
    if(range_begin.count != any_is && !dekey.empty() &&
       !DB::KeySeq::compare(key_seq,
                            range_begin, dekey,
                            Condition::LT, range_begin.count, true)) {
      //SWC_PRINT << "-- ALIGNED MAX NO MATCH --" << SWC_PRINT_CLOSE;
      return false;
    }
    //SWC_PRINT << "-- ALIGNED MATCH -------" << SWC_PRINT_CLOSE;
    uint8_t after_idx = range->cfg->range_type == DB::Types::Range::MASTER;
    params.range_begin.copy(after_idx, cell.key);
    params.range_end.copy(after_idx, key_end);
    params.rid = rid;
    return true;
  }

  bool reached_limits() override {
    return params.cid && params.rid;
  }

  bool add_cell_and_more(const DB::Cells::Cell& cell) override {
    profile.add_cell(cell.encoded_length());

    std::string id_name(cell.key.get_string(0));
    params.cid = strtoull(id_name.c_str(), nullptr, 0);
    return false;
  }


  void response(int &err) override {

    if(err)
      params.err = err;
    else if(Env::Rgr::is_shuttingdown())
      params.err = Error::SERVER_SHUTTING_DOWN;
    else if(range->deleted())
      params.err = Error::COLUMN_MARKED_REMOVED;
    else if(!params.cid || !params.rid)
      params.err = Error::RANGE_NOT_FOUND;

    m_conn->send_response(Comm::Buffers::make(m_ev, params));

    profile.finished();
    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM
        << "Range(" << range->cfg->cid  << '/' << range->rid << ") ";
      params.print(SWC_LOG_OSTREAM);
      profile.print(SWC_LOG_OSTREAM << " flags=" << int(flags) << " Locate-");
      if(params.err) {
        spec.print(SWC_LOG_OSTREAM << ' ');
        range_begin.print(SWC_LOG_OSTREAM << " range_begin=");
      }
    );

    if(err == Error::RGR_NOT_LOADED_RANGE)
      range->issue_unload();
  }

  RangePtr              range;
  uint8_t               flags;
  uint32_t              any_is;
  const DB::Cell::Key   range_begin;

  Comm::Protocol::Rgr::Params::RangeLocateRsp params;

};


}}}
#endif // swcdb_ranger_callbacks_RangeLocateScan_h
