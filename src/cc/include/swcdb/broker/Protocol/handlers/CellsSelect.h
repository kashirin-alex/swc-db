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
  bool            sent;

  SWC_CAN_INLINE
  Selector(const ConnHandlerPtr& a_conn, const Event::Ptr& a_ev) noexcept
          : SWC::client::Query::Select::Handlers::Base(Env::Clients::get()),
            conn(a_conn), ev(a_ev), sent(false) {
  }

  virtual ~Selector() noexcept { }

  bool valid() noexcept override  {
    return !state_error && !ev->expired() && conn->is_open() &&
            Env::Bkr::is_accepting();
  }

  void error(const cid_t, int err) override {
    SWC::client::Query::Select::Handlers::Base::error(err);
  }

  size_t get_size_bytes() noexcept override {
    return 0;
  }

  bool add_cells(const cid_t, StaticBuffer& buffer,
                 bool, DB::Specs::Interval& interval) override {
    if(!ev->expired() && conn->is_open()) {
      conn->send_response(Buffers::make(
        ev,
        Params::CellsSelectRsp(
          state_error == Error::CLIENT_STOPPING
            ? Error::SERVER_SHUTTING_DOWN
            : state_error,
          true,
          interval.flags.offset
        ),
        buffer
      ));
    }
    sent = true;
    return false;
  }

  void response(int err) override {
    SWC::client::Query::Select::Handlers::Base::error(err);

    if(!sent && !ev->expired() && conn->is_open()) {
      if(state_error == Error::CLIENT_STOPPING || !Env::Bkr::is_accepting())
         state_error.store(Error::SERVER_SHUTTING_DOWN);
      conn->send_response(
        Buffers::make(ev, Params::CellsSelectRsp(state_error, false)));
    }
    profile.finished();

    SWC_LOG_OUT(LOG_DEBUG,
      profile.print(SWC_LOG_OSTREAM << "Select-");
      Error::print(SWC_LOG_OSTREAM << ' ', state_error);
    );

    Env::Bkr::processed();
  }
};




void cells_select(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;

    Params::CellsSelectReq params;
    params.decode(&ptr, &remain);

    auto schema = Env::Clients::get()->get_schema(err, params.cid);
    if(!err) {
      Selector::Ptr(new Selector(conn, ev))
        ->scan(schema, std::move(params.interval));
      return;
    }
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  conn->send_response(Buffers::make(ev, Params::CellsSelectRsp(err)));
  Env::Bkr::processed();
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_CellsSelect_h
