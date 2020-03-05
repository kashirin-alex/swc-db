/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/fsbroker/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"
#include "swcdb/fsbroker/AppContext.h"


namespace SWC {

int run() {
  SWC_TRY_OR_LOG("", 
  
  auto app_ctx = std::make_shared<server::FsBroker::AppContext>();

  auto srv = std::make_shared<server::SerializedServer>(
    "FS-BROKER", 
    Env::Config::settings()->get_i32("swc.FsBroker.reactors"), 
    Env::Config::settings()->get_i32("swc.FsBroker.workers"), 
    "swc.fs.broker.port",
    app_ctx
  );
  app_ctx->set_srv(srv);
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
