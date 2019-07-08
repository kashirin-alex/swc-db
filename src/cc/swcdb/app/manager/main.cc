/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/manager/Settings.h"

#include "swcdb/lib/core/comm/SerializedServer.h"

#include "swcdb/lib/manager/AppContext.h"


#include <string>
#include <vector>
#include <iostream>



int main(int argc, char** argv) {

  SWC::Config::settings->init(argc, argv);

  std::shared_ptr<SWC::AppContext> app_ctx 
    = std::make_shared<SWC::server::Mngr::AppContext>();

  std::shared_ptr<SWC::server::SerializedServer> srv 
    = std::make_shared<SWC::server::SerializedServer>(
    "RS-MANAGER", 
    SWC::Config::settings->get<int32_t>("swc.mngr.reactors"), 
    SWC::Config::settings->get<int32_t>("swc.mngr.workers"), 
    "swc.mngr.port",
    app_ctx
  );


  
  std::cout << "srv, 1" << "\n";

  std::thread([srv]{ 
  std::this_thread::sleep_for(std::chrono::milliseconds(15000));
          std::cout << "stop, 1" << "\n";
      srv->stop();
          std::cout << "stop, 2" << "\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }).detach();;

  std::cout << "srv, 2" << "\n";
  
  return 0;
}
