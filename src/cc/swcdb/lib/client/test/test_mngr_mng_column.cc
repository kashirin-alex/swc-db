/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/db/Protocol/req/MngColumn.h"
#include "swcdb/lib/db/Protocol/req/GetColumn.h"

#include "swcdb/lib/db/Stats/Stat.h"


namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  // file_desc().add_options();
}
void Settings::init_post_cmd_args(){ }
}}

using namespace SWC;

void check_get(int num_of_cols, int num_of_cols_to_remain, bool modified = false){


  std::cout << "########### get_scheme_by_name ###########\n";

  std::shared_ptr<Stats::Stat> latency = std::make_shared<Stats::Stat>();
  
  struct ExpctedRsp{
    public:
    ExpctedRsp(std::string name, Types::Encoding blk_encoding, bool exists)
              : name(name), blk_encoding(blk_encoding),
                exists(exists), chks(0) { }
    //int64_t     cid;

    std::string name;
    Types::Encoding blk_encoding;
    bool exists;
    std::atomic<int> chks;
  };
  std::vector<std::shared_ptr<ExpctedRsp>> expected;

  Protocol::Req::GetColumnPtr hdlr_get
    = std::make_shared<Protocol::Req::GetColumn>(1000);
  for(int n=1; n<=num_of_cols+num_of_cols_to_remain;n++){

    std::string name("column-");
    name.append(std::to_string(n));
    
    
    expected.push_back(std::make_shared<ExpctedRsp>(
      name, 
      modified ?  Types::Encoding::SNAPPY : Types::Encoding::PLAIN,
      n>num_of_cols)
    );
  }

  for(auto& req : expected){

    hdlr_get->get_scheme_by_name(
      req->name, 
      [req, latency, start_ts=std::chrono::system_clock::now()]
      (Protocol::Req::GetColumn::Req::Ptr ptr, int err, Protocol::Params::GetColumnRsp rsp)
      {
        uint64_t took  = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::system_clock::now() - start_ts).count();
        latency->add(took);
        std::cout << "GetColumnRsp: exists="<< req->exists << " took=" << took  
                  << " err=" << err << "(" << Error::get_text(err) << ") " 
                  << " " << (err==Error::OK?rsp.schema->to_string().c_str():"NULL") << "\n";
        
        if(!req->exists && err==Error::OK)
          exit(1);  
        if(req->exists && err!=Error::OK)
          exit(1); 
        if(err==Error::OK && req->blk_encoding != rsp.schema->blk_encoding)
          exit(1); 
        if(err==Error::OK && req->name.compare(rsp.schema->col_name) != 0)
          exit(1); 
        req->chks++;
      }
    );
  }
  while(hdlr_get->due()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "get_scheme_by_name"
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count() 
            << "\n";

  std::cout << "########### get_id_by_name ###########\n";
  latency = std::make_shared<Stats::Stat>();

  for(auto& req : expected){
    hdlr_get->get_id_by_name(
      req->name, 
      [req, latency, start_ts=std::chrono::system_clock::now()]
      (Protocol::Req::GetColumn::Req::Ptr ptr, int err, Protocol::Params::GetColumnRsp rsp)
      {
        uint64_t took  = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::system_clock::now() - start_ts).count();
        latency->add(took);
        std::cout << "GetColumnRsp: exists="<< req->exists << " took=" << took  
                  << " err=" << err << "(" << Error::get_text(err) << ") " 
                  << " " << (err==Error::OK?rsp.cid:-1) << "\n";

        if(!req->exists && err==Error::OK)
          exit(1);  
        if(req->exists && err!=Error::OK)
          exit(1);
        req->chks++;

      }
    );
  }
  while(hdlr_get->due()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "get get_id_by_name"
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count() 
            << "\n";


  for(auto& req : expected) {
    if(req->chks != 2){
      
      std::cerr << " chks="<< req->chks.load()
                << " name=" << req->name 
                << " exists=" << req->exists 
                << " \n";
      exit(1);
    }
  }
}

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

        Types::Encoding blk_encoding = Types::Encoding::PLAIN;
        if(func == Protocol::Req::MngColumn::Function::MODIFY)
          blk_encoding = Types::Encoding::SNAPPY;

        hdlr->request(
          func,
          DB::Schema::make(0, name, Types::Column::COUNTER_I64, 10, 1234, 3, blk_encoding, 9876543),

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
                  && err !=  Error::COLUMN_SCHEMA_NAME_EXISTS ) 
              || 
                (ptr->function == Protocol::Req::MngColumn::Function::DELETE 
                  && err !=  Error::COLUMN_SCHEMA_NAME_NOT_EXISTS )
              || 
                (ptr->function == Protocol::Req::MngColumn::Function::MODIFY 
                  && err !=  Error::COLUMN_SCHEMA_NAME_NOT_EXISTS )
              ))
              {
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
  

  int num_of_cols = 1000;
  int num_of_cols_to_remain = 1000;
  Protocol::Req::MngColumnPtr hdlr 
    = std::make_shared<Protocol::Req::MngColumn>(60000);
    
  //chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, 100000);
  //exit(0);

  std::cout << "## already exists response expected ##\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";

  std::cout << "########### delete request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";
  
  std::cout << "#### no exists response expected #####\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, num_of_cols+num_of_cols_to_remain, true);
  std::cout << "######################################\n\n";

  std::cout << "########### create request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, num_of_cols+num_of_cols_to_remain, true);
  std::cout << "######################################\n\n";

  std::cout << "########### delete request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, num_of_cols, true);
  std::cout << "######################################\n\n";

  check_get(num_of_cols, num_of_cols_to_remain);

  
  std::cout << "########### modify request ###########\n";
  chk(hdlr, Protocol::Req::MngColumn::Function::MODIFY, 1, num_of_cols+num_of_cols_to_remain, true);
  std::cout << "######################################\n\n";

  check_get(num_of_cols, num_of_cols_to_remain, true);

  //chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 1, 100000);
  //chk(hdlr, Protocol::Req::MngColumn::Function::DELETE, 1, 100000);


  Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###\n";
  return 0;
}
