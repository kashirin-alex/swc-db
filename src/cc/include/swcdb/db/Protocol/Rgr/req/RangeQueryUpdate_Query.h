/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_req_RangeQueryUpdate_Query_h
#define swcdb_db_protocol_rgr_req_RangeQueryUpdate_Query_h


#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"
#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


template<typename QueryT, typename ActionT>
class RangeQueryUpdate_Query: public client::ConnQueue::ReqBase {
  public:

  typedef std::shared_ptr<RangeQueryUpdate_Query> Ptr;

  SWC_CAN_INLINE
  static void request(
          const QueryT& query,
          ActionT&& action_data,
          const Params::RangeQueryUpdateReq& params,
          const DynamicBuffer::Ptr& buffer,
          const EndPoints& endpoints,
          const uint32_t timeout = 10000) {
    StaticBuffer snd_buf(buffer->base, buffer->fill(), false);
    query->get_clients()->get_rgr_queue(endpoints)->put(
      Ptr(new RangeQueryUpdate_Query(
        query, std::move(action_data),
        Buffers::make(params, snd_buf, 0, RANGE_QUERY_UPDATE, timeout)
      ))
    );
  }

  virtual ~RangeQueryUpdate_Query() { }

  virtual bool valid() override {
    return query->valid();
  }

  void handle_no_conn() override {
    action_data.callback(
      query, req(), Params::RangeQueryUpdateRsp(Error::COMM_NOT_CONNECTED));
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    ev->type == Event::Type::DISCONNECT
      ? handle_no_conn()
      : action_data.callback(
          query,
          req(),
          Params::RangeQueryUpdateRsp(ev->error, ev->data.base, ev->data.size)
        );
  }

  protected:

  SWC_CAN_INLINE
  RangeQueryUpdate_Query(const QueryT& query,
                         ActionT&& action_data,
                         const Buffers::Ptr& cbp)
                        : client::ConnQueue::ReqBase(false, cbp),
                          query(query),
                          action_data(std::move(action_data)) {
  }

  QueryT   query;
  ActionT  action_data;

};


}}}}}


#endif // swcdb_db_protocol_rgr_req_RangeQueryUpdate_Query_h
