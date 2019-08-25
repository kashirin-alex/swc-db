/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/db/Protocol/req/MngColumn.h"



namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  // file_desc().add_options();
}
void Settings::init_post_cmd_args(){ }
}}

using namespace SWC;

void chk(Protocol::Req::MngColumnPtr hdlr, 
         Protocol::Req::MngColumn::Function func){

  std::vector<std::thread*> threads;
  for(int t=1;t<=1;t++) {
    threads.push_back(new std::thread([hdlr, func, t](){
      for(int i=1; i<= 100000; i++){
        std::string name("column-");
        name.append(std::to_string(i*t));

        hdlr->request(
          func,
          DB::Schema::make(0, name),
          [hdlr]
          (Protocol::Req::MngColumn::Req::BasePtr req, int err){
            Protocol::Req::MngColumn::Req::Ptr ptr = std::dynamic_pointer_cast<Protocol::Req::MngColumn::Req>(req);
            if(err != Error::OK 
              && (
                (ptr->function == Protocol::Req::MngColumn::Function::ADD 
                  && err !=  Error::SCHEMA_COL_NAME_EXISTS ) 
              || 
                (ptr->function == Protocol::Req::MngColumn::Function::DELETE 
                  && err !=  Error::SCHEMA_COL_NAME_NOT_EXISTS )
              ))
              hdlr->make(ptr);

            std::cout << " func="<< ptr->function 
                      << " pending_writes=" << hdlr->pending_write()
                      << " pending_read=" << hdlr->pending_read()
                      << " queue=" << hdlr->queue()
                      << " name="<< ptr->schema->col_name  
                      << " " << err << "(" << Error::get_text(err) << ")\n"; 
          }
        );
      }
    }));
  }

  for(auto t : threads)t->join();

  
  std::cout << " func="<< func 
            << " ### threads EXIT ###, "
            << " pending_writes=" << hdlr->pending_write()
            << " pending_read=" << hdlr->pending_read()
            << " queue=" << hdlr->queue()
            << "\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  
  while(hdlr->due()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << " func="<< func 
              << " pending_writes=" << hdlr->pending_write()
              << " pending_read=" << hdlr->pending_read()
              << " queue=" << hdlr->queue()
              << "\n";
  }
  hdlr->close();
}


int main(int argc, char** argv) {
  EnvConfig::init(argc, argv);
  
  EnvClients::init(std::make_shared<client::Clients>(
    nullptr,
    std::make_shared<client::AppContext>()
  ));
  

  Protocol::Req::MngColumnPtr hdlr = std::make_shared<Protocol::Req::MngColumn>();
  
  chk(hdlr, Protocol::Req::MngColumn::Function::ADD);

  //chk(hdlr, Protocol::Req::MngColumn::Function::DELETE);

  //chk(hdlr, Protocol::Req::MngColumn::Function::ADD);


  EnvIoCtx::io()->stop();
  std::cout << " ### EXIT ###, pending_writes=" << hdlr->pending_write()
            << " pending_read=" << hdlr->pending_read()
            << " queue=" << hdlr->queue()
            << "\n";
  return 0;
}
