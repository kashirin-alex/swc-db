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

void check_get(int num_of_cols, int num_of_cols_to_remain, 
              bool modified, bool verbose=false){


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
    Protocol::Req::Column::Get::scheme(
      req->name, 
      [req, latency, verbose, start_ts=std::chrono::system_clock::now()]
      (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr, 
        int err, Protocol::Params::GetColumnRsp rsp) {

        if(err == Error::REQUEST_TIMEOUT) {
          req_ptr->request_again();
          return;
        }
        
        uint64_t took  = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::system_clock::now() - start_ts).count();
        latency->add(took);
        if(verbose)
          std::cout << "GetColumnRsp: exists="<< req->exists << " took=" << took  
                    << " count=" << latency->count()
                    << " err=" << err << "(" << Error::get_text(err) << ") " 
                    << " " << (err==Error::OK?rsp.schema->to_string().c_str():"NULL") << "\n";

        if(err==Error::OK){
          if(!req->exists) {
            std::cerr << " SHOULDN'T exist \n";
            exit(1); 
          }
          if(req->blk_encoding != rsp.schema->blk_encoding) {
            std::cerr << " blk_encoding don't match \n";
            exit(1); 
          }
          if(req->name.compare(rsp.schema->col_name) != 0) {
            std::cerr << " name don't match \n";
            exit(1); 
          }
        } else if(req->exists){
          std::cerr << " SHOULD exist \n";
          exit(1);  
        }
        req->chks++;
      }
    );
  }
  while(latency->count() < expected.size()) {
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
    Protocol::Req::Column::Get::cid(
      req->name, 
      [req, latency, verbose, start_ts=std::chrono::system_clock::now()]
      (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr,
       int err, Protocol::Params::GetColumnRsp rsp) {
        
        if(err == Error::REQUEST_TIMEOUT) {
          req_ptr->request_again();
          return;
        }

        uint64_t took  = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::system_clock::now() - start_ts).count();
        latency->add(took);
        if(verbose)
          std::cout << "GetColumnRsp: exists="<< req->exists << " took=" << took  
                    << " count=" << latency->count()
                    << " err=" << err << "(" << Error::get_text(err) << ") " 
                    << " " << (err==Error::OK?rsp.cid:-1) << "\n";

        if(err==Error::OK){
          if(!req->exists) {
            std::cerr << " SHOULDN'T exist \n";
            exit(1); 
          }
        } else if(req->exists){
          std::cerr << " SHOULD exist \n";
          exit(1);  
        }
        req->chks++;

      },
      60000
    );
  }
  while(latency->count() < expected.size()) {
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



void chk(Protocol::Req::Column::Mng::Func func, 
         int threads_num=1, int checks=1, bool verbose=false) {
  std::atomic<int> chks = checks+1;
  std::shared_ptr<Stats::Stat> latency = std::make_shared<Stats::Stat>();

  std::vector<std::thread*> threads;
  for(int t=1;t<=threads_num;t++) {
    threads.push_back(new std::thread([func, t, latency, verbose, &chks](){
      int n;
      for(;;) {
        n = --chks;
        if(n <= 0)
          break;
        std::string name("column-");
        name.append(std::to_string(n));

        Types::Encoding blk_encoding = Types::Encoding::PLAIN;
        if(func == Protocol::Req::Column::Mng::Func::MODIFY)
          blk_encoding = Types::Encoding::SNAPPY;

        Protocol::Req::Column::Mng::request(
          func,
          DB::Schema::make(0, name, Types::Column::COUNTER_I64, 10, 1234, 3, blk_encoding, 9876543),

          [func, latency, verbose, start_ts=std::chrono::system_clock::now()]
          (Protocol::Req::ConnQueue::ReqBase::Ptr req_ptr, int err){

            uint64_t took 
              = std::chrono::duration_cast<std::chrono::nanoseconds>(
                  std::chrono::system_clock::now() - start_ts).count();

            if(err != Error::OK 
              && (
                (func == Protocol::Req::Column::Mng::Func::CREATE 
                  && err !=  Error::COLUMN_SCHEMA_NAME_EXISTS ) 
              || 
                (func == Protocol::Req::Column::Mng::Func::DELETE 
                  && err !=  Error::COLUMN_SCHEMA_NAME_NOT_EXISTS )
              || 
                (func == Protocol::Req::Column::Mng::Func::MODIFY 
                  && err != Error::COLUMN_SCHEMA_NAME_NOT_EXISTS
                  && err != Error::COLUMN_SCHEMA_NOT_DIFFERENT )
              ))
              {
              req_ptr->request_again();

            } else {
              latency->add(took);
            }
            
            if(verbose)
              std::cout << " func="<< func
                        << " err="<<err<< "(" << Error::get_text(err) << ")"
                        << " took=" << took
                        << " avg=" << latency->avg()
                        << " min=" << latency->min()
                        << " max=" << latency->max()
                        << " count=" << latency->count()
                        << "\n";
          },
          60000
        );
      }
    }));
  }

  for(auto& t : threads)t->join();
  
  while(latency->count() < checks*threads_num ) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

    
  std::cout << " threads_num="<< threads_num
            << " checks="<< checks
            << " func="<< func 
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << "\n";
}


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);
  
  Env::Clients::init(std::make_shared<client::Clients>(
    nullptr,
    std::make_shared<client::AppContext>()
  ));
  

  int num_of_cols = 2000;
  int num_of_cols_to_remain = 100;

  /*  
  chk(Protocol::Req::Column::Mng::Func::DELETE, 1, 31000, true);
  std::cout << " OK! \n";
  exit(0);
  */
  chk(Protocol::Req::Column::Mng::Func::DELETE, 1, num_of_cols, false);
    
  chk(Protocol::Req::Column::Mng::Func::CREATE, 1, num_of_cols, false);
  check_get(0, num_of_cols, false, false);

  chk(Protocol::Req::Column::Mng::Func::MODIFY, 1, num_of_cols, false);
  check_get(0, num_of_cols, true, false);

  chk(Protocol::Req::Column::Mng::Func::DELETE, 1, num_of_cols, false);
  check_get(num_of_cols, 0, false, false);

  std::cout << " OK! \n";
  exit(0);


  Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###\n";
  return 0;
}
