/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fsbroker/Settings.h"
#include "swcdb/lib/core/comm/SerializedServer.h"
#include "swcdb/lib/fsbroker/AppContext.h"


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);

  std::shared_ptr<SWC::AppContext> app_ctx 
    = std::make_shared<SWC::server::FsBroker::AppContext>();

  SWC::server::SerializedServerPtr srv
   = std::make_shared<SWC::server::SerializedServer>(
      "FS-BROKER", 
      SWC::Env::Config::settings()->get<int32_t>("swc.FsBroker.reactors"), 
      SWC::Env::Config::settings()->get<int32_t>("swc.FsBroker.workers"), 
      "swc.fs.broker.port",
      app_ctx
    );
  ((SWC::server::FsBroker::AppContext*)app_ctx.get())->set_srv(srv);
  srv->run();

  return 0;
}
