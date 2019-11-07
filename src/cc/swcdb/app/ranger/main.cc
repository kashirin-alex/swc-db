/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/ranger/Settings.h"
#include "swcdb/lib/core/comm/SerializedServer.h"
#include "swcdb/lib/ranger/AppContext.h"


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);

  auto app_ctx = std::make_shared<SWC::server::Rgr::AppContext>();

  auto srv = std::make_shared<SWC::server::SerializedServer>(
    "RANGER", 
    SWC::Env::Config::settings()->get<int32_t>("swc.rgr.reactors"), 
    SWC::Env::Config::settings()->get<int32_t>("swc.rgr.workers"), 
    "swc.rgr.port",
    app_ctx
  );
  ((SWC::server::Rgr::AppContext*)app_ctx.get())->set_srv(srv);
  srv->run();

  return 0;
}
