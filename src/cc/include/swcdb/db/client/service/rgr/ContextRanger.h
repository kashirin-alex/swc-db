/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_rgr_ContextRanger_h
#define swcdb_db_client_rgr_ContextRanger_h

#include "swcdb/db/client/Settings.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace client {


class ContextRanger : public Comm::AppContext {
  public:

  typedef std::shared_ptr<ContextRanger> Ptr;

  ContextRanger(const Config::Settings& settings);

  virtual ~ContextRanger() { }

  void handle_established(Comm::ConnHandlerPtr) override { }

  void handle_disconnect(Comm::ConnHandlerPtr) override { }

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

};


}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/service/rgr/ContextRanger.cc"
#endif

#endif // swcdb_db_client_rgr_ContextRanger_h
