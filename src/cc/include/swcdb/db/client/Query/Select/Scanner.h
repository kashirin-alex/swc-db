/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Scanner_h
#define swcdb_db_client_Query_Select_Scanner_h


#include "swcdb/db/client/Query/Select/Handlers/Base.h"
#include "swcdb/db/client/Query/Select/Handlers/BaseUnorderedMap.h"

#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"


namespace SWC { namespace client { namespace Query {

//! The SWC-DB Query Select C++ namespace 'SWC::client::Query::Select'
namespace Select {



class Scanner final : public std::enable_shared_from_this<Scanner> {
  public:

  SWC_CAN_INLINE
  static void execute(const Handlers::Base::Ptr& hdlr,
                      DB::Types::KeySeq key_seq, cid_t cid,
                      const DB::Specs::Interval& intval) {
    Ptr(new Scanner(
      hdlr, key_seq, intval, cid))->mngr_locate_master();
  }

  SWC_CAN_INLINE
  static void execute(const Handlers::Base::Ptr& hdlr,
                      DB::Types::KeySeq key_seq, cid_t cid,
                      DB::Specs::Interval&& intval) {
    Ptr(new Scanner(
      hdlr, key_seq, std::move(intval), cid))->mngr_locate_master();
  }


  typedef std::shared_ptr<Scanner>        Ptr;
  Core::CompletionCounter<uint64_t>       completion;

  Handlers::Base::Ptr                     selector;
  const DB::Types::KeySeq                 col_seq;
  DB::Specs::Interval                     interval;

  const cid_t                             master_cid;
  const cid_t                             meta_cid;
  const cid_t                             data_cid;

  cid_t                                   master_rid;
  cid_t                                   meta_rid;
  cid_t                                   data_rid;

  int64_t                                 master_mngr_revision;
  bool                                    master_mngr_is_end;
  bool                                    master_mngr_next;

  bool                                    master_rgr_next;
  bool                                    meta_next;
  uint8_t                                 retry_point;
  bool                                    need_data_cid_check;

  ReqBase::Ptr                            master_rgr_req_base;
  ReqBase::Ptr                            meta_req_base;
  ReqBase::Ptr                            data_req_base;

  DB::Cell::Key                           master_mngr_offset;
  DB::Cell::Key                           master_rgr_offset;
  DB::Cell::Key                           meta_offset;
  DB::Cell::Key                           meta_end;

  Comm::EndPoints                         master_rgr_endpoints;
  Comm::EndPoints                         meta_endpoints;
  Comm::EndPoints                         data_endpoints;

  Scanner(const Handlers::Base::Ptr& hdlr,
          const DB::Types::KeySeq col_seq,
          const DB::Specs::Interval& interval,
          const cid_t cid);

  Scanner(const Handlers::Base::Ptr& hdlr,
          const DB::Types::KeySeq col_seq,
          DB::Specs::Interval&& interval,
          const cid_t cid) noexcept;

  ~Scanner() noexcept;

  SWC_CAN_INLINE
  bool valid() noexcept {
    return selector->valid();
  }

  SWC_CAN_INLINE
  Clients::Ptr& get_clients() noexcept {
    return selector->clients;
  }

  SWC_CAN_INLINE
  Profiling& get_profile() noexcept {
    return selector->profile;
  }

  void debug_res_cache(const char* msg, cid_t cid, rid_t rid,
                       const Comm::EndPoints& endpoints);

  void print(std::ostream& out);

  void response_if_last();

  void next_call();


  void mngr_locate_master();

  bool mngr_located_master(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);


  void rgr_locate_master();

  void rgr_located_master(
      const ReqBase::Ptr& base,
      const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp);


  void mngr_resolve_rgr_meta();

  bool mngr_resolved_rgr_meta(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);


  void rgr_locate_meta();

  void rgr_located_meta(
      const ReqBase::Ptr& base,
      const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp);


  void mngr_resolve_rgr_select();

  bool mngr_resolved_rgr_select(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);


  void rgr_select();

  void rgr_selected(
      const ReqBase::Ptr& req,
      Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp);

};


}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Select/Scanner.cc"
#endif


#endif // swcdb_db_client_Query_Select_Scanner_h
