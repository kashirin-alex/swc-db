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

class Stat {
  public:
  Stat(): m_count(0), m_avg(0), m_max(0),m_min(-1) {}
  virtual ~Stat(){}

  void add(uint64_t v){
    std::lock_guard<std::mutex> lock(m_mutex);
    m_avg *= m_count;
    m_avg += v;
    m_avg /= ++m_count;
    if(v > m_max)
      m_max = v;
    if(v < m_min)
      m_min = v;
  }

  uint64_t avg(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_avg;
  }
  uint64_t max(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_max;
  }
  uint64_t min(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_min;
  }

  uint64_t count(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_count;
  }

  private:
  std::mutex m_mutex;
  uint64_t m_count;
  uint64_t m_avg;
  uint64_t m_min;
  uint64_t m_max;
};

void chk(Protocol::Req::MngColumnPtr hdlr, 
         Protocol::Req::MngColumn::Function func, int threads_num=1, int checks=1){
  std::atomic<int> chks = checks+1;
  std::shared_ptr<Stat> stat = std::make_shared<Stat>();

  std::vector<std::thread*> threads;
  for(int t=1;t<=threads_num;t++) {
    threads.push_back(new std::thread([hdlr, func, t, stat, &chks](){
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
          [hdlr, stat, start_ts=std::chrono::system_clock::now()]
          (Protocol::Req::MngColumn::Req::BasePtr req, int err){
            Protocol::Req::MngColumn::Req::Ptr ptr = std::dynamic_pointer_cast<Protocol::Req::MngColumn::Req>(req);

            uint64_t took = std::chrono::duration_cast<std::chrono::nanoseconds>(
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
              stat->add(took);
            /* 
            std::cout << " func="<< ptr->function 
                      << " pending_writes=" << hdlr->pending_write()
                      << " pending_read=" << hdlr->pending_read()
                      << " queue=" << hdlr->queue()
                      << " name="<< ptr->schema->col_name  
                      << " " << err << "(" << Error::get_text(err) 
                      << " took=" << took << ")\n"; 
            */
          }
        );
      }
    }));
  }

  for(auto& t : threads)t->join();
  
  while(hdlr->due()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

    
    std::cout << " threads_num="<< threads_num
              << " checks="<< checks
              << " func="<< func 
              << " pending_writes=" << hdlr->pending_write()
              << " pending_read=" << hdlr->pending_read()
              << " queue=" << hdlr->queue()
              << " avg=" << stat->avg()
              << " min=" << stat->min()
              << " max=" << stat->max()
              << " count=" << stat->count()
              << "\n";
  hdlr->close();
}


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);
  
  Env::Clients::init(std::make_shared<client::Clients>(
    nullptr,
    std::make_shared<client::AppContext>()
  ));
  

  Protocol::Req::MngColumnPtr hdlr = std::make_shared<Protocol::Req::MngColumn>();
  
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
    chk(hdlr, Protocol::Req::MngColumn::Function::CREATE, 128, 10000);
    i+=1;
  }
  std::cout << "####\n";
  //chk(hdlr, Protocol::Req::MngColumn::Function::DELETE);

  //chk(hdlr, Protocol::Req::MngColumn::Function::CREATE);


  Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###, pending_writes=" << hdlr->pending_write()
            << " pending_read=" << hdlr->pending_read()
            << " queue=" << hdlr->queue()
            << "\n";
  return 0;
}
