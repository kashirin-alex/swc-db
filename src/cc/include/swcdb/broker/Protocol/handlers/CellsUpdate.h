/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_Protocol_handlers_CellsUpdate_h
#define swcdb_broker_Protocol_handlers_CellsUpdate_h


#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"
#include "swcdb/db/client/Query/Update/Handlers/BaseSingleColumn.h"
#include "swcdb/db/client/Query/Update/Committer.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Handler {


class Updater final
    : public SWC::client::Query::Update::Handlers::BaseSingleColumn {
  public:
  typedef std::shared_ptr<Updater> Ptr;

  ConnHandlerPtr  conn;
  Event::Ptr      ev;

  SWC_CAN_INLINE
  Updater(const DB::Schema::Ptr& schema,
          const ConnHandlerPtr& conn, const Event::Ptr& ev)
          : SWC::client::Query::Update::Handlers::BaseSingleColumn(
              Env::Clients::get(),
              schema->cid, schema->col_seq,
              schema->cell_versions, schema->cell_ttl, schema->col_type,
              ev->data_ext
            ),
            conn(conn), ev(ev) {
    ev->data_ext.free();
  }

  virtual ~Updater() { }

  bool valid() noexcept override {
    return !error() && !ev->expired() && conn->is_open();
  }

  void response(int err=Error::OK) override {
    if(!completion.is_last())
      return;

    if(!err && requires_commit()) {
      commit(&column);
      return;
    }

    if(err)
      error(err);
    else if(!empty())
      error(Error::CLIENT_DATA_REMAINED);

    if(!ev->expired() && conn->is_open()) {
      conn->send_response(Buffers::make(
        ev,
        Params::CellsUpdateRsp(
          error() == Error::CLIENT_STOPPING
            ? Error::SERVER_SHUTTING_DOWN
            : error()
        )
      ));
    }
    profile.finished();

    SWC_LOG_OUT(LOG_DEBUG,
      SWC_LOG_OSTREAM << "Column(" << column.cid << ")";
      profile.print(SWC_LOG_OSTREAM << " Update-");
      Error::print(SWC_LOG_OSTREAM << ' ', error());
    );

    Env::Bkr::processed();
  }

};


void cells_update(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Params::CellsUpdateReq params;

  try {
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    if(!ev->data_ext.size) {
      err = Error::INVALID_ARGUMENT;
    } else {
      auto schema = Env::Clients::get()->get_schema(err, params.cid);
      if(!err) {
        Updater::Ptr hdlr(new Updater(schema, conn, ev));
        hdlr->commit(&hdlr->column);
        return;
      }
    }

  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  conn->send_response(Buffers::make(ev, Params::CellsUpdateRsp(err)));
  Env::Bkr::processed();
}


}}}}}

#endif // swcdb_broker_Protocol_handlers_CellsUpdate_h
