/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_req_RgrGet_Query_h
#define swcdb_db_protocol_req_RgrGet_Query_h


#include "swcdb/db/Protocol/Mngr/req/RgrGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


template<typename QueryT, typename ActionT>
class RgrGet_Query : public RgrGet_Base {
  public:

  typedef std::shared_ptr<RgrGet_Query>           Ptr;

  SWC_CAN_INLINE
  static void request(
        const QueryT& query,
        cid_t cid, rid_t rid, bool next_range,
        const SWC::client::Query::Profiling::Component::Start& profile,
        const uint32_t timeout = 10000) {
    request(
      query,
      Params::RgrGetReq(cid, rid, next_range),
      profile,
      timeout
    );
  }

  SWC_CAN_INLINE
  static void request(
        const QueryT& query,
        const Params::RgrGetReq& params,
        const SWC::client::Query::Profiling::Component::Start& profile,
        const uint32_t timeout = 10000) {
    make(query, params, profile, timeout)->run();
  }

  static Ptr make(
        const QueryT& query,
        const Params::RgrGetReq& params,
        const SWC::client::Query::Profiling::Component::Start& profile,
        const uint32_t timeout = 10000) {
    return Ptr(new RgrGet_Query(query, params, profile, timeout));
  }

  virtual ~RgrGet_Query() { }

  virtual bool valid() override {
    return query->valid();
  }

  protected:

  RgrGet_Query(
        const QueryT& query,
        const Params::RgrGetReq& params,
        const SWC::client::Query::Profiling::Component::Start& profile,
        const uint32_t timeout)
        : RgrGet_Base(params, timeout), query(query), profile(profile) {
  }

  virtual SWC::client::Clients::Ptr& get_clients() noexcept override {
    return query->get_clients();
  }

  virtual void callback(const Params::RgrGetRsp& rsp) override {
    ActionT::callback(query, req(), profile, rsp);
  }

  QueryT                                          query;
  SWC::client::Query::Profiling::Component::Start profile;

};


}}}}}


#endif // swcdb_db_protocol_req_RgrGet_Query_h
