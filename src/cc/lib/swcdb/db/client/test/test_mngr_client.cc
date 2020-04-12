/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */
 
#include "swcdb/db/client/Settings.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/AppContext.h"

#include <string>
#include <vector>

#include <chrono>
#include <thread>
namespace SWC { namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}}


class ReqHandler : public SWC::DispatchHandler{
  public:
  ReqHandler(size_t th, size_t num, std::atomic<size_t>& t):total(t){
    m_num = num;
    m_tnum = th;
    m_start = std::chrono::system_clock::now();
  }

  void handle(SWC::ConnHandlerPtr conn, SWC::Event::Ptr &ev) { 
    if(ev->type == SWC::Event::Type::DISCONNECT) {
      std::cout << "ReqHandler: t=" << m_tnum << " num=" << m_num  << " " << ev->to_str() << "\n"; 
      return;
    }

    std::cout << "ReqHandler:  t=" << m_tnum << " num=" << m_num 
              << ", took=" << std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now() - m_start).count()
      << ", " << ev->to_str() << "\n"; 
    
    if(--total == 0){
      conn->do_close();
      return;
    } 
  }
  std::chrono::system_clock::time_point m_start;
  size_t m_num;
  size_t m_tnum;
  std::atomic<size_t>& total;
};

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  
  auto clients = std::make_shared<SWC::client::Clients>(
    nullptr,
    std::make_shared<SWC::client::AppContext>()
  );

  SWC::EndPoints endpoints = clients->mngrs_groups->get_endpoints(101);
  for(auto& endpoint : endpoints){
    std::cout << "[" << endpoint.address().to_string() << "]:" << endpoint.port() << "\n";
  }
  
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  
  /* 
  SWC::ConnHandlerPtr con_h = client->get_connection(endpoints);
  std::cout << client->to_str(con_h) << " (NEW)\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  */
  std::cout << "--######################--\n";
  std::vector<std::thread*> threads;

  for(size_t t=1;t<=64;++t){
    SWC::EndPoints endpoints_copy;
    endpoints_copy.assign(endpoints.begin(),endpoints.end());
    std::cout << "starting thread=" << t << "\n";
    threads.push_back(new std::thread([clients, endpoints_copy, t](){
      
      auto con_h = clients->mngr_service->get_connection(endpoints_copy);
      std::cout << "thread=" << t << " " << clients->mngr_service->to_str(con_h) << " (NEW)\n";
      //std::this_thread::sleep_for(std::chrono::microseconds(1000));
      size_t num_req = 1000;
      std::atomic<size_t> total = num_req;

      for(size_t n=1;n<=num_req;++n){
        SWC::DispatchHandler::Ptr req = std::make_shared<ReqHandler>(t, n, total);
        auto cbp = SWC::CommBuf::create_error_message(
          SWC::Error::OK, 
          SWC::format("req.BLOCK_COMPRESSOR_UNSUPPORTED_TYPE t=(%d) n=(%d)", t, n).c_str()
        );  
        cbp->header.timeout_ms = 10000;
        con_h->send_request(cbp, req); // sequential readings
        //std::this_thread::sleep_for(std::chrono::microseconds(500));
      }

      do{
        std::this_thread::sleep_for(std::chrono::microseconds(100000));
        //std::cout << " thread=" << t << " total=" << total << "\n";
      } while (total>0);

      //client->release(con_h);
      //std::this_thread::sleep_for(std::chrono::microseconds(1000));
      std::cout << "exiting thread=" << t << " total=" << total << " " << clients->mngr_service->to_str(con_h) << "\n";
    }));
    //std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }

  for(auto& t : threads)t->join();

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  std::cout << "--######################--\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  exit(0);

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  return 0;
}
