/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/Error.h"
#include "swcdb/lib/manager/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/mngr/AppContext.h"

#include <string>
#include <vector>

#include <chrono>
#include <thread>


class ReqHandler : public SWC::DispatchHandler{
  public:
  ReqHandler(size_t th, size_t num, std::atomic<size_t>& t):total(t){
    m_num = num;
    m_tnum = th;
    m_start = std::chrono::system_clock::now();
  }

  void handle(SWC::ConnHandlerPtr conn, SWC::EventPtr &ev) { 
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

  SWC::Config::settings->init(argc, argv);
  
  SWC::client::ClientsPtr clients = std::make_shared<SWC::client::Clients>(
    nullptr,
    std::make_shared<SWC::client::Mngr::AppContext>()
  );

  SWC::EndPoints endpoints = clients->mngrs_groups->get_endpoints(101);
  for(auto endpoint : endpoints){
    std::cout << "[" << endpoint.address().to_string() << "]:" << endpoint.port() << "\n";
  }
  
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  
  /* 
  SWC::client::ClientConPtr con_h = client->get_connection(endpoints);
  std::cout << client->to_str(con_h) << " (NEW)\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  */
  std::cout << "--######################--\n";
  std::vector<std::thread*> threads;

  for(size_t t=1;t<=64;t++){
    SWC::EndPoints endpoints_copy;
    endpoints_copy.assign(endpoints.begin(),endpoints.end());
    std::cout << "starting thread=" << t << "\n";
    threads.push_back(new std::thread([clients, endpoints_copy, t](){
      
      SWC::client::ClientConPtr con_h = clients->mngr_service->get_connection(endpoints_copy);
      std::cout << "thread=" << t << " " << clients->mngr_service->to_str(con_h) << " (NEW)\n";
      //std::this_thread::sleep_for(std::chrono::microseconds(1000));
      size_t num_req = 1000;
      std::atomic<size_t> total = num_req;

      for(size_t n=1;n<=num_req;n++){
        SWC::DispatchHandlerPtr req = std::make_shared<ReqHandler>(t, n, total);
        SWC::CommHeader header;
        header.timeout_ms = 10000;
        SWC::CommBufPtr cbp = SWC::Protocol::create_error_message(
          header, SWC::Error::OK, SWC::format("req.BLOCK_COMPRESSOR_UNSUPPORTED_TYPE t=(%d) n=(%d)", t, n).c_str());
        con_h->send_request(cbp, req);
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

  for(auto t : threads)t->join();

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  std::cout << "--######################--\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  exit(0);

  /* 

  for(int n=0; n<1; n++){
    SWC::client::ClientConPtr con = client->get_connection(ips);
    std::cout << client->to_str(con) << " (" << n << " ) (NEW)\n";

    std::cout << "----\n";
    con->response_ok();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    con->read_header();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "----\n";
    con->response_ok();
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    con->read_header(req);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "----\n";
    con->response_ok();
    con->read_header();
    std::cout << "----\n";
    con->response_ok();
    con->read_header(req);
    std::cout << "----\n";
    con->response_ok();
    con->read_header(req);
    std::cout << "----\n";
    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    client->release(con);

    SWC::client::ClientConPtr con1 = client->get_connection(ips);
    std::cout << client->to_str(con1) << " (" << n << " ) (REUSE)\n";
    client->close(con1);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(5000));

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  con1 = client->get_connection(ips);
  std::cout << client->to_str(con1) << " (REUSED)\n";
  client->close_connection(con1);

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  con1 = client->get_connection(ips);
  std::cout << client->to_str(con1) << " (NEW)\n";
  client->close_connection(con1);

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  con1 = client->get_connection({"136.243.13.216"});
  std::cout << client->to_str(con1) << " (IPv4)\n";
  client->release_connection(con1);

  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  con1 = client->get_connection({"2a01:4f8:211:2757::2"});
  std::cout << client->to_str(con1) << "(IPv6)\n";
  client->release_connection(con1);

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));

  // con1 = client->get_connection({"2a01:4f8:211:2757::2"});
  // con1->getContext();
  client->close_all();
  
  /* 
  SWC::client::ClientConPtr con2 = client->get_connection();
  SWC::client::ClientConPtr con3 = client->get_connection();
  SWC::client::ClientConPtr con4 = client->get_connection();
  */

  /* 
  std::make_shared<SWC::SerializedEventHandlerCtx<SWC::client::BytesPipeline>>(con1, SWC::Event::Type::MESSAGE)
    ->error(SWC::Error::PROTOCOL_ERROR, "Client Con-1 send event");

  std::cout << "Closing Con-2 \n";
  try{
    con2->close();
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
  */

  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  return 0;
}
