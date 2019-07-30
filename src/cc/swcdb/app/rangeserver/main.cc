/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/rangeserver/Settings.h"
#include "swcdb/lib/core/comm/SerializedServer.h"
#include "swcdb/lib/rangeserver/AppContext.h"


int main(int argc, char** argv) {
  SWC::EnvConfig::init(argc, argv);

  std::shared_ptr<SWC::AppContext> app_ctx 
    = std::make_shared<SWC::server::RS::AppContext>();

  SWC::server::SerializedServerPtr srv = std::make_shared<SWC::server::SerializedServer>(
    "RANGE-SERVER", 
    SWC::EnvConfig::settings()->get<int32_t>("swc.rs.reactors"), 
    SWC::EnvConfig::settings()->get<int32_t>("swc.rs.workers"), 
    "swc.rs.port",
    app_ctx
  );
  ((SWC::server::RS::AppContext*)app_ctx.get())->set_srv(srv);
  srv->run();

  return 0;
}
