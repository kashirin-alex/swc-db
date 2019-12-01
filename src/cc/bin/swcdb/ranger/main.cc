/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/ranger/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"
#include "swcdb/ranger/AppContext.h"


namespace SWC {

int run() {
  SWC_TRY_OR_LOG("", 
  
  auto app_ctx = std::make_shared<server::Rgr::AppContext>();

  auto srv = std::make_shared<server::SerializedServer>(
    "RANGER", 
    Env::Config::settings()->get<int32_t>("swc.rgr.reactors"), 
    Env::Config::settings()->get<int32_t>("swc.rgr.workers"), 
    "swc.rgr.port",
    app_ctx
  );
  ((server::Rgr::AppContext*)app_ctx.get())->set_srv(srv);
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
