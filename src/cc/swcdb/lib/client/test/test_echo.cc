/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/db/Protocol/Common/req/Echo.h"

#include "swcdb/lib/db/Stats/Stat.h"


namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  file_desc().add_options()
    ("requests", i32(1), "number of requests") 
    ("batch", i32(1), "batch size of each request")
    ("threads", i32(1), "number of threads x (requests x batch)")
    ("threads_conn", i32(1), "threads a connection") 
    ("buffer", i64(0), "buffer size to send and receive")
  ;
}
void Settings::init_post_cmd_args(){ }
}}

using namespace SWC;

size_t buffer_sz = 0;

class Checker: public std::enable_shared_from_this<Checker>{
  public:
  Checker(int num_req, int batch_sz, int threads_conn)
          : num_req(num_req), batch_sz(batch_sz), threads_conn(threads_conn),
            total(num_req*batch_sz*threads_conn) {}

  void run(client::ClientConPtr conn, int req_n = 1){
    
    for(int i=1;i<=batch_sz;i++) {
      std::make_shared<Protocol::Common::Req::Echo>(
        conn, 
        [req_n, conn, last=i==batch_sz, ptr=shared_from_this(), start_ts=std::chrono::system_clock::now()]
        (bool state){

          if(!state)
            ptr->failures++;

          ptr->latency->add(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now() - start_ts).count());   
      
          if(ptr->latency->count() % 100000 == 0) {
            std::cout << conn->endpoint_local_str() << ",";
            ptr->print_stats();
            std::cout << "\n";
          }
          if(!last)
            return;

          if(req_n < ptr->num_req){
            ptr->run(conn, req_n+1);

          } else if(ptr->latency->count() == ptr->total) {
            ptr->processed.store(true);
            ptr->cv.notify_all();
          }  

        },
        buffer_sz
      )->run();
    }
  }

  void get_conn(){
    Env::Clients::get()->mngr_service->get_connection(
      Env::Clients::get()->mngrs_groups->get_endpoints(1, 1), 
      [ptr=shared_from_this()](client::ClientConPtr conn){
        if(conn == nullptr || !conn->is_open()){
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
          ptr->get_conn();
          return;
        }
        
        std::vector<std::thread*> threads;
        for(int t=1;t<=ptr->threads_conn;t++)
          threads.push_back(
            new std::thread(
              [conn, ptr](){ptr->run(conn);}
          ));
        for(auto& t : threads)t->join();
      },
      std::chrono::milliseconds(1000), 
      1
    );
  }

  void print_stats(){
    std::cout << " avg=" << latency->avg()
              << " min=" << latency->min()
              << " max=" << latency->max()
              << " count=" << latency->count()
              << " failures=" << failures;
  }

  void run(int num_threads){
    total*=num_threads;

    auto start_ts = std::chrono::system_clock::now();

    std::vector<std::thread*> threads;
    for(int t=1;t<=num_threads;t++)
      threads.push_back(
        new std::thread([ptr=shared_from_this()](){ptr->get_conn();}));

    for(auto& t : threads)t->join();
    
    std::unique_lock<std::mutex> lock_wait(lock);
    cv.wait(lock_wait, [stop=&processed]{return stop->load();});

    auto took = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::system_clock::now() - start_ts).count();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "# FINISHED,"
              << " threads=" << num_threads
              << " threads_conn=" << threads_conn
              << " requests=" << num_req
              << " batch=" << batch_sz
              << " took=" << took
              << " median=" << took/latency->count();
    print_stats();
    std::cout << "\n";
  }

  int total;
  std::atomic<bool> processed=false;
  std::atomic<int> failures=0;
  int num_req;
  int batch_sz;
  int threads_conn;
  std::mutex lock;
  std::condition_variable cv;
  
  std::shared_ptr<Stats::Stat> latency = std::make_shared<Stats::Stat>();
};


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);
  
  Env::Clients::init(std::make_shared<client::Clients>(
    nullptr,
    std::make_shared<client::AppContext>()
  ));

  buffer_sz = Env::Config::settings()->get<int64_t>("buffer");

  std::make_shared<Checker>(
    Env::Config::settings()->get<int32_t>("requests"), 
    Env::Config::settings()->get<int32_t>("batch"),
    Env::Config::settings()->get<int32_t>("threads_conn")
  )->run(
    Env::Config::settings()->get<int32_t>("threads")
  );

  Env::IoCtx::io()->stop();

  return 0;
}
