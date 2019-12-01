/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fsbroker/Settings.h"
#include "swcdb/lib/core/comm/SerializedServer.h"
#include "swcdb/lib/fsbroker/AppContext.h"


namespace SWC {

int run() {
  SWC_TRY_OR_LOG("", 
  
  auto app_ctx = std::make_shared<server::FsBroker::AppContext>();

  auto srv = std::make_shared<server::SerializedServer>(
    "FS-BROKER", 
    Env::Config::settings()->get<int32_t>("swc.FsBroker.reactors"), 
    Env::Config::settings()->get<int32_t>("swc.FsBroker.workers"), 
    "swc.fs.broker.port",
    app_ctx
  );
  ((server::FsBroker::AppContext*)app_ctx.get())->set_srv(srv);
  srv->run();

  return 0);
  return 1;
}

}

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  SWC::Env::Config::settings()->init_process();
  return SWC::run();
}
