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



int main(int argc, char** argv) {
  EnvConfig::init(argc, argv);
  
  EnvClients::init(std::make_shared<client::Clients>(
    nullptr,
    std::make_shared<client::AppContext>()
  ));
  
  std::vector<std::thread*> threads;

  Protocol::Req::MngColumnPtr hdlr = std::make_shared<Protocol::Req::MngColumn>();

  for(int t=1;t<=16;t++) {
    threads.push_back(new std::thread([hdlr, t](){
      for(int i=1; i<= 100000; i++){
        std::string name("column-");
        name.append(std::to_string(i*t));

        hdlr->create(
          DB::Schema::make(0, name),
          [hdlr]
          (Protocol::Req::MngColumn::Req::BasePtr req, int err){
          Protocol::Req::MngColumn::Req::Ptr ptr = std::dynamic_pointer_cast<Protocol::Req::MngColumn::Req>(req);
          if(err == Error::OK || err ==  Error::SCHEMA_COL_NAME_EXISTS) {
            // std::cout << " ColumnReq. name="<< req->schema->col_name 
            //          << " cb, err=" << err << "(" << Error::get_text(err) << ")\n";
          } else {
            // std::cout << " RE-ColumnReq. "<< req->schema->to_string() 
            //          << " cb, err=" << err << "(" << Error::get_text(err) << ")\n"; 
            hdlr->make(ptr);
          }

          std::cout << " pending_writes=" << hdlr->pending_write()
                    << " pending_read=" << hdlr->pending_read()
                    << " name="<< ptr->schema->col_name  
                    << " " << err << "(" << Error::get_text(err) << ")\n"; 
          }
        );
      }
    }));
  }

  for(auto t : threads)t->join();

  
  std::cout << " ### threads EXIT ###, "
                << " pending_writes=" << hdlr->pending_write()
                << " pending_read=" << hdlr->pending_read() << "\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));

  while(hdlr->pending_write() > 0 || hdlr->pending_read() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    std::cout << " pending_writes=" << hdlr->pending_write()
              << " pending_read=" << hdlr->pending_read() << "\n";
  }

  std::cout << " ### EXIT ###, pending_writes=" << hdlr->pending_write()
            << " pending_read=" << hdlr->pending_read() << "\n";
  return 0;
}
