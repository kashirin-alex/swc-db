/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/manager/Settings.h"
#include "swcdb/lib/core/comm/SerializedServer.h"
#include "swcdb/lib/manager/AppContext.h"


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);

  auto app_ctx  = std::make_shared<SWC::server::Mngr::AppContext>();

  auto srv = std::make_shared<SWC::server::SerializedServer>(
    "MANAGER", 
    SWC::Env::Config::settings()->get<int32_t>("swc.mngr.reactors"), 
    SWC::Env::Config::settings()->get<int32_t>("swc.mngr.workers"), 
    "swc.mngr.port",
    app_ctx
  );
  ((SWC::server::Mngr::AppContext*)app_ctx.get())->set_srv(srv);
  srv->run();

  return 0;
}
