/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_Query_Update_Committer_h
#define swcdb_db_client_Query_Update_Committer_h


#include "swcdb/db/client/Query/Update/Handlers/Base.h"
#include "swcdb/db/Types/Range.h"
#include "swcdb/db/Protocol/Mngr/params/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"


namespace SWC { namespace client { namespace Query {

//! The SWC-DB Query Update C++ namespace 'SWC::client::Query::Update'
namespace Update {



class Committer final : public std::enable_shared_from_this<Committer> {
  public:

  SWC_CAN_INLINE
  static void execute(const Handlers::Base::Ptr& hdlr,
                      Handlers::Base::Column* colp) {
    Ptr(new Committer(
      DB::Types::Range::MASTER,
      colp->get_cid(), colp, colp->get_first_key(),
      hdlr
    ))->locate_on_manager();
  }

  typedef std::shared_ptr<Committer>  Ptr;
  const DB::Types::Range              type;
  const cid_t                         master_cid;
  Core::CompletionCounter<uint32_t>   workload;
  const cid_t                         cid;
  Handlers::Base::Column*             colp;
  DB::Cell::Key::Ptr                  key_start;
  Handlers::Base::Ptr                 hdlr;
  ReqBase::Ptr                        parent;
  const rid_t                         rid;
  const DB::Cell::Key                 key_finish;

  Committer(const DB::Types::Range type,
            const cid_t cid,
            Handlers::Base::Column* colp,
            const DB::Cell::Key::Ptr& key_start,
            const Handlers::Base::Ptr& hdlr,
            const ReqBase::Ptr& parent=nullptr,
            const rid_t rid=0) noexcept;

  Committer(const DB::Types::Range type,
            const cid_t cid,
            Handlers::Base::Column* colp,
            const DB::Cell::Key::Ptr& key_start,
            const Handlers::Base::Ptr& hdlr,
            const ReqBase::Ptr& parent,
            const rid_t rid,
            const DB::Cell::Key& key_finish);

  ~Committer() noexcept;

  SWC_CAN_INLINE
  bool valid() noexcept {
    return hdlr->valid();
  }

  SWC_CAN_INLINE
  Clients::Ptr& get_clients() noexcept {
    return hdlr->clients;
  }

  SWC_CAN_INLINE
  Profiling& get_profile() noexcept {
    return hdlr->profile;
  }

  void print(std::ostream& out);

  void locate_on_manager();

  private:

  void located_on_manager(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);

  void locate_on_ranger(Comm::EndPoints&& endpoints, int64_t revision=0);

  void located_on_ranger(
      Comm::EndPoints&& endpoints,
      const ReqBase::Ptr& base,
      const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp);

  void resolve_on_manager();

  void located_ranger(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);

  void proceed_on_ranger(
      const ReqBase::Ptr& base,
      Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);

  void commit_data(
      const Comm::EndPoints& endpoints,
      const ReqBase::Ptr& base);

  void committed_data(
      const DynamicBuffer& cells_buff,
      const ReqBase::Ptr& base,
      const ReqBase::Ptr& req,
      const Comm::Protocol::Rgr::Params::RangeQueryUpdateRsp& rsp);

};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update/Committer.cc"
#endif


#endif // swcdb_db_client_Query_Update_Committer_h
