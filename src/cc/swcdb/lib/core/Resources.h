/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_Resources_h
#define swc_core_Resources_h

//Env::Resources::init(Env::Config::settings()->get_ptr<gInt32t>("swc."+app+".ram.percent"))

namespace SWC { 

class Resources {
  public:

  Resources() {}

  virtual ~Resources() {
    if(m_timer != nullptr)
      delete m_timer;
  }

  void init(asio::io_context* io, gInt32tPtr percent_ram) {
    cfg_percent_ram = percent_ram;

    if(m_timer == nullptr)
      m_timer = new asio::high_resolution_timer(*io);

    checker();
  }

  const bool need_ram(const uint32_t& sz) {
    return ram_total < ram_free + sz || ram_used + sz >  ram_allowed;
  }

  void checker() {
    // ram_total, ram_free   /proc/meminfo
    // ram_used              /proc/self/statm

    ram_allowed = (ram_total/100) * cfg_percent_ram->get();
    schedule();
  }

  void schedule(uint32_t ms = 10000) {
    m_timer->expires_from_now(std::chrono::milliseconds(ms));
    m_timer->async_wait(
      [ptr=this](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->checker();
        }
    }); 
  }

  void stop() {
    m_timer->cancel();
  }

  gInt32tPtr                    cfg_percent_ram;

  asio::high_resolution_timer*  m_timer; 
  
  std::atomic<size_t> ram_total    = 0;
  std::atomic<size_t> ram_free     = 0;
  std::atomic<size_t> ram_used     = 0;
  std::atomic<size_t> ram_allowed  = 0;
  
};



namespace Env {

inline static SWC::Resources Resources;

}


}

#endif // swc_core_Resources_h
