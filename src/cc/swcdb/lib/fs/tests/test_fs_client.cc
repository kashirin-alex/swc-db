/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fs/Settings.h"
#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/RS/Columns.h"
#include "swcdb/lib/db/Columns/MNGR/Columns.h"

#include <iostream>


void SWC::Config::Settings::init_app_options(){
  init_fs_options();
}
void SWC::Config::Settings::init_post_cmd_args(){}

using namespace SWC;

void run(size_t thread_id){

    int err = Error::OK;
    bool exists = EnvFsInterface::fs()->exists(err, "nonexisting/"+std::to_string(thread_id));
    if(exists || err != Error::OK){ 
     std::cerr << "ERROR(nonexisting file) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }
    exists = EnvFsInterface::fs()->exists(err, "");
    if(!exists || err != Error::OK){ 
     std::cerr << "ERROR(existing file) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }
    
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int main(int argc, char** argv) {
  EnvConfig::init(argc, argv);
  EnvIoCtx::init(8);

  EnvFsInterface::init();
    
  for(size_t chk=1;chk<=3;chk++) {
    std::cout << "--1--\n";
    std::vector<std::thread*> threads;
    for(size_t t=1;t<=64;t++)
      threads.push_back(new std::thread([t](){run(t);}));
    
    std::cout << "--2--\n";
    for(auto t : threads) t->join();

    std::cout << "--3--\n";
  
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "--######################-- chk=" <<chk<< "\n";
  }
  exit(0);
}