/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/rangeserver/Settings.h"

#include "swcdb/lib/core/comm/SerializedServer.h"

#include "swcdb/lib/rangeserver/AppContext.h"


#include <string>
#include <vector>
#include <iostream>



int main(int argc, char** argv) {

  SWC::Config::settings->init(argc, argv);

  std::shared_ptr<SWC::AppContext> app_ctx 
    = std::make_shared<SWC::server::RS::AppContext>();


  SWC::server::SerializedServer* srv = (new SWC::server::SerializedServer(
    "RANGE-SERVER", 
    SWC::Config::settings->get<int32_t>("swc.rs.reactors"), 
    SWC::Config::settings->get<int32_t>("swc.rs.workers"), 
    "swc.rs.port",
    app_ctx
  ));




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
