/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_RangeQuerySelect_Query_h
#define swcdb_db_protocol_rgr_req_RangeQuerySelect_Query_h


#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"
#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


template<typename QueryT, typename ActionT>
class RangeQuerySelect_Query: public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeQuerySelect_Query> Ptr;

  SWC_CAN_INLINE
  static void request(
          const QueryT& query,
          const Params::RangeQuerySelectReq& params,
          const EndPoints& endpoints,
          const SWC::client::Query::Profiling::Component::Start& profile,
          const uint32_t timeout = 10000) {
    query->get_clients()->get_rgr_queue(endpoints)->put(
      Ptr(new RangeQuerySelect_Query(query, params, profile, timeout))
    );
  }

  virtual ~RangeQuerySelect_Query() { }

  virtual bool valid() override {
    return query->valid();
  }

  void handle_no_conn() override {
    Params::RangeQuerySelectRsp rsp(Error::COMM_NOT_CONNECTED);
    ActionT::callback(query, req(), profile, rsp);
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    if(ev->type == Event::Type::DISCONNECT)
      return handle_no_conn();

    Params::RangeQuerySelectRsp rsp(
      ev->error, ev->data.base, ev->data.size, ev->data_ext);
    ActionT::callback(query, req(), profile, rsp);
  }

  protected:

  SWC_CAN_INLINE
  RangeQuerySelect_Query(
          const QueryT& query,
          const Params::RangeQuerySelectReq& params,
          const SWC::client::Query::Profiling::Component::Start& profile,
          const uint32_t timeout)
          : client::ConnQueue::ReqBase(
              false,
              Buffers::make(params, 0, RANGE_QUERY_SELECT, timeout)
            ),
            query(query), profile(profile) {
  }

  QueryT                                          query;
  SWC::client::Query::Profiling::Component::Start profile;

};


}}}}}



#endif // swcdb_db_protocol_rgr_req_RangeQuerySelect_Query_h
