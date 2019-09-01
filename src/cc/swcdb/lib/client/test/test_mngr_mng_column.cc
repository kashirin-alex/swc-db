/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/db/Protocol/req/MngColumn.h"

#include "swcdb/lib/db/Stats/Stat.h"


namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  // file_desc().add_options();
}
void Settings::init_post_cmd_args(){ }
}}

using namespace SWC;


void chk(Protocol::Req::MngColumnPtr hdlr, 
         Protocol::Req::MngColumn::Function func, 
         int threads_num=1, int checks=1, bool verbose=false) {
  std::atomic<int> chks = checks+1;
  std::shared_ptr<Stats::Stat> latency = std::make_shared<Stats::Stat>();

  std::vector<std::thread*> threads;
  for(int t=1;t<=threads_num;t++) {
    threads.push_back(new std::thread([hdlr, func, t, latency, verbose, &chks](){
      int n;
      for(;;) {
        n = --chks;
        if(n <= 0)
          break;
        std::string name("column-");
        name.append(std::to_string(n));

        hdlr->request(
          func,
          DB::Schema::make(0, name),
          [hdlr, latency, verbose, start_ts=std::chrono::system_clock::now()]
          (Protocol::Req::MngColumn::Req::BasePtr req, int err){
            Protocol::Req::MngColumn::Req::Ptr ptr 
              = std::dynamic_pointer_cast<Protocol::Req::MngColumn::Req>(req);

            uint64_t took 
              = std::chrono::duration_cast<std::chrono::nanoseconds>(
                  std::chrono::system_clock::now() - start_ts).count();

            if(err != Error::OK 
              && (
                (ptr->function == Protocol::Req::MngColumn::Function::CREATE 
                  && err !=  Error::SCHEMA_COL_NAME_EXISTS ) 
              || 
                (ptr->function == Protocol::Req::MngColumn::Function::DELETE 
                  && err !=  Error::SCHEMA_COL_NAME_NOT_EXISTS )
              )){
              hdlr->make(ptr);

            } else
              latency->add(took);
            
            if(verbose)
              std::cout << " func="<< ptr->function 
                        << " err="<<err<< "(" << Error::get_text(err) << ")"
                        << " pending_writes=" << hdlr->pending_write()
                        << " pending_read=" << hdlr->pending_read()
                        << " queue=" << hdlr->queue()
                        << " avg=" << latency->avg()
                        << " min=" << latency->min()
                        << " max=" << latency->max()
                        << " count=" << latency->count()
                        << "\n";
          }
        );
      }
    }));
  }

  for(auto& t : threads)t->join();
  
  while(hdlr->due()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

    
  std::cout << " threads_num="<< threads_num
            << " checks="<< checks
            << " func="<< func 
            << " pending_writes=" << hdlr->pending_write()
            << " pending_read=" << hdlr->pending_read()
            << " queue=" << hdlr->queue()
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << "\n";
  hdlr->close();
}


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);
  
  Env::Clients::init(std::make_shared<client::Clients>(
    nullptr,
    std::make_shared<client::AppContext>()
  ));
  

  Protocol::Req::MngColumnPtr hdlr 
    = std::make_shared<Protocol::Req::MngColumn>();
  /* 
  for(int i=1;i<=10;){
    chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, 1);
    i+=1;
  }
  std::cout << "####\n";
  for(int i=1;i<=10;){
    chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, 10);
    i+=1;
  }
  std::cout << "####\n";
  for(int i=1;i<=10;){
    chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 4, 100);
    i+=1;
  }
  std::cout << "####\n";

  for(int i=1;i<=10;){
    chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 8, 1000);
    i+=1;
  }
  std::cout << "####\n\n";
  */
  int num_of_cols = 2000;
  std::cout << "## already exists response expected ##\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";

  //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  
  std::cout << "########### delete request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";

  //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  
  std::cout << "#### no exists response expected #####\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";

  //std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  std::cout << "########### create request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";


  //std::this_thread::sleep_for(std::chrono::milliseconds(60000));

  std::cout << "########### delete request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";

  /* 
  std::cout << "#### delete all (cid: 3+) #####\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, 1000, true);
  std::cout << "######################################\n\n";
  */


  Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###, pending_writes=" << hdlr->pending_write()
            << " pending_read=" << hdlr->pending_read()
            << " queue=" << hdlr->queue()
            << "\n";
  return 0;
}
