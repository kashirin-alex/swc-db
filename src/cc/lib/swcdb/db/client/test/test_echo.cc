/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/Echo.h"

#include "swcdb/db/client/Stats/Stat.h"


namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  init_client_options();
  file_desc.add_options()
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

std::atomic<size_t> failures=0;
std::atomic<ssize_t> expected=0;

class Checker {
  public:
  Checker(int num_req, int batch_sz, int threads_conn)
          : num_req(num_req), batch_sz(batch_sz), threads_conn(threads_conn) {
    expected = num_req * batch_sz * threads_conn;
  }

  void run(ConnHandlerPtr conn, int req_n = 0){
    if(++req_n > num_req)
      return;

    for(int i=1;i<=batch_sz;i++) {
      std::make_shared<Protocol::Mngr::Req::Echo>(
        conn, 
        [this, req_n, conn, last=i==batch_sz, start_ts=std::chrono::system_clock::now()]
        (bool state){
          if(!state)
            failures++;
          expected--;

          latency->add(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now() - start_ts).count());   
      
          if(expected % 100000 == 0)
            print_stats();

          if(expected <= 0) {
            std::cout << "notify_all expected=" << expected << "\n";
            cv.notify_all();
            return;
          } 

          if(!last)
            return;
            
          run(conn, req_n);
        },
        buffer_sz
      )->run();
    }
  }

  void get_conn(){
    Env::Clients::get()->mngr_service->get_connection(
      Env::Clients::get()->mngrs_groups->get_endpoints(1, 1), 
      [this](ConnHandlerPtr conn){
        if(conn == nullptr || !conn->is_open()){
          get_conn();
          return;
        }
        
        std::vector<std::thread*> threads;
        for(int t=1;t<=threads_conn;t++)
          std::thread([this, conn](){run(conn);}).detach();
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
              << " failures=" << failures
              << "\n";;
  }

  void run(int num_threads){
    expected = expected * num_threads;

    auto start = std::chrono::system_clock::now();

    for(int t=1;t<=num_threads;t++)
      std::thread([this](){get_conn();}).detach();

    std::unique_lock<std::mutex> lock_wait(lock);
    cv.wait(lock_wait, []{return !expected.load();});


    int64_t took = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::system_clock::now() - start).count();

    std::cout << "# FINISHED,"
              << " threads=" << num_threads
              << " threads_conn=" << threads_conn
              << " requests=" << num_req
              << " batch=" << batch_sz
              << " took=" << took
              << " median=" << took/latency->count()
              << "\n";
    print_stats();
  }

  int num_req;
  int batch_sz;
  int threads_conn;
  std::mutex lock;
  std::condition_variable cv;
  
  std::shared_ptr<Stats::Stat> latency = std::make_shared<Stats::Stat>();
};


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);
  
  Env::Clients::init(
    std::make_shared<client::Clients>(
      nullptr,
      std::make_shared<client::AppContext>()
    )
  );

  buffer_sz = Env::Config::settings()->get_i64("buffer");

  Checker(
    Env::Config::settings()->get_i32("requests"), 
    Env::Config::settings()->get_i32("batch"),
    Env::Config::settings()->get_i32("threads_conn")
  ).run(
    Env::Config::settings()->get_i32("threads")
  );

  Env::IoCtx::io()->stop();

  return 0;
}
