/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_ColumnUpdate_h
#define swcdb_manager_Protocol_handlers_ColumnUpdate_h

#include "swcdb/manager/Protocol/Mngr/params/ColumnUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


struct ColumnUpdate {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  ColumnUpdate(const Comm::ConnHandlerPtr& conn,
               const Comm::Event::Ptr& ev) noexcept
              : conn(conn), ev(ev) {
  }

  void operator()() {
    if(ev->expired())
      return;

    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      Params::ColumnUpdate params;
      params.decode(&ptr, &remain);

      conn->response_ok(ev);

      params.function == Params::ColumnMng::Function::INTERNAL_EXPECT
        ? Env::Mngr::mngd_columns()->set_expect(
            params.cid_begin, params.cid_end, params.total,
            std::move(params.columns), false
          )
        : Env::Mngr::mngd_columns()->update_status(
            params.function, params.schema, params.err, params.id
          ); // +?timeout
    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      conn->send_error(e.code(), "", ev);
    }
  }

};


}}}}}

#endif // swcdb_manager_Protocol_handlers_ColumnUpdate_h
