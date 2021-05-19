/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_CellsSelect_h
#define swcdb_broker_Protocol_handlers_CellsSelect_h


#include "swcdb/db/Protocol/Bkr/params/CellsSelect.h"
#include "swcdb/db/client/Query/Select/Handlers/Base.h"
#include "swcdb/db/client/Query/Select/Scanner.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {



class Selector final : public SWC::client::Query::Select::Handlers::Base {
  public:

  typedef std::shared_ptr<Selector>   Ptr;

  ConnHandlerPtr  conn;
  Event::Ptr      ev;

  Selector(const ConnHandlerPtr& conn, const Event::Ptr& ev)
          : SWC::client::Query::Select::Handlers::Base(Env::Clients::get()),
            conn(conn), ev(ev) {
  }

  virtual ~Selector() { }

  bool valid() noexcept override  {
    return !state_error && !ev->expired() && conn->is_open();
  }

  void error(const cid_t, int err) override {
    SWC::client::Query::Select::Handlers::Base::error(err);
  }

  size_t get_size_bytes() noexcept override {
    return 0;
  }

  bool add_cells(const cid_t cid, StaticBuffer& buffer,
                 bool reached_limit, DB::Specs::Interval& interval) override {
    bool more = true;
    size_t recved = 0;
    DB::Cells::Cell last;
    const uint8_t* ptr = buffer.base;
    size_t remain = buffer.size;
    while(remain) {
      last.read(&ptr, &remain);
      ++recved;
    }

    if(interval.flags.limit) {
      if(interval.flags.limit <= recved) {
        interval.flags.limit = 0;
        more = false;
        goto _send_partial;
      }
      interval.flags.limit -= recved;
    }
    if(reached_limit) {
      interval.offset_key.copy(last.key);
      interval.offset_rev = last.get_timestamp();
    }

    _send_partial:
      auto cbp = Buffers::make(
        ev, Params::CellsSelectRsp(Error::OK, cid, true), buffer);
      cbp->header.flags |= Comm::Header::FLAG_RESPONSE_PARTIAL_BIT;
      conn->send_response(cbp);

    return more;
  }

  void response(int err) override {
    SWC::client::Query::Select::Handlers::Base::error(err);

    profile.finished();

    if(!ev->expired() && conn->is_open()) {
      conn->send_response(
        Buffers::make(ev, Params::CellsSelectRsp(state_error)));
    }
    Env::Bkr::in_process(-1);
  }
};




void cells_select(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::CellsSelectReq params;
    params.decode(&ptr, &remain);

    auto hdlr = std::make_shared<Selector>(conn, ev);
    SWC::client::Query::Select::scan(err, hdlr, std::move(params.specs));
    if(!err)
      return;

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  conn->send_response(Buffers::make(ev, Params::CellsSelectRsp(err)));
  Env::Bkr::in_process(-1);
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_CellsSelect_h
