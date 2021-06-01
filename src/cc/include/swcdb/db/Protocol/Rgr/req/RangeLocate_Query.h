/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_protocol_rgr_req_RangeLocate_Query_h
#define swcdb_db_protocol_rgr_req_RangeLocate_Query_h


#include "swcdb/db/Protocol/Rgr/params/RangeLocate.h"
#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


template<typename QueryT, typename ActionT>
class RangeLocate_Query : public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeLocate_Query> Ptr;
  const EndPoints                            endpoints;

  static void request(
        const QueryT& query,
        const Params::RangeLocateReq& params,
        const EndPoints& endpoints,
        const SWC::client::Query::Profiling::Component::Start& profile,
        const uint32_t timeout = 10000) {
    Ptr(new RangeLocate_Query(
      query, params, endpoints, profile, timeout
    ))->run();
  }

  virtual ~RangeLocate_Query() { }

  virtual bool valid() override {
    return query->valid();
  }

  void handle_no_conn() override {
    ActionT::callback(
      query, req(), profile,
      Params::RangeLocateRsp(Error::COMM_NOT_CONNECTED)
    );
  }

  bool run() override {
    query->get_clients()->get_rgr_queue(endpoints)->put(req());
    return true;
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    ev->type == Event::Type::DISCONNECT
      ? handle_no_conn()
      : ActionT::callback(
          query, req(), profile,
          Params::RangeLocateRsp(ev->error, ev->data.base, ev->data.size)
        );
  }

  protected:

  RangeLocate_Query(
        const QueryT& query,
        const Params::RangeLocateReq& params,
        const EndPoints& endpoints,
        const SWC::client::Query::Profiling::Component::Start& profile,
        const uint32_t timeout)
        : client::ConnQueue::ReqBase(
            false,
            Buffers::make(params, 0, RANGE_LOCATE, timeout)
          ),
          endpoints(endpoints), query(query), profile(profile) {
  }

  QueryT                                          query;
  SWC::client::Query::Profiling::Component::Start profile;

};


}}}}}


#endif // swcdb_db_protocol_rgr_req_RangeLocate_Query_h
