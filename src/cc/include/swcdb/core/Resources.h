/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_Resources_h
#define swc_core_Resources_h

#include <sys/sysinfo.h>
#include <fstream>


namespace SWC { 

class Resources final {
  public:

  Resources() {}

  ~Resources() {
    if(m_timer != nullptr)
      delete m_timer;
  }

  void init(asio::io_context* io, gInt32tPtr percent_ram, 
            std::function<void(size_t)> release_call=0) {
    if(m_timer == nullptr)
      m_timer = new asio::high_resolution_timer(*io);
    
    cfg_percent_ram = percent_ram;
    release = release_call;

    checker();
  }

  const size_t need_ram() {
   return ram.used > ram.allowed ? ram.used - ram.allowed : 0;
  }

  const size_t avail_ram() {
   return ram.allowed - ram.used  ? ram.allowed - ram.used : 0;
  }

  const bool need_ram(const uint32_t& sz) {
    return ram.free - sz*2 < 0 || ram.used + sz > ram.allowed;
  }

  void stop() {
    m_timer->cancel();
  }

  const std::string to_string() {
    std::string s("Resources(");
    s.append("Mem-MB-");
    s.append(ram.to_string(1048576));
    s.append(")");
    return s;
  }

  private:

  void checker() {
    refresh_stats();

    std::cout << to_string() << "\n";

    if(release) {
      size_t bytes = need_ram();
      if(bytes)
        release(bytes);
    }

    schedule();
  }

  void refresh_stats() {
    if(next_major_chk++ == 0) {
      page_size = sysconf(_SC_PAGE_SIZE);
    
      ram.total   = page_size * sysconf(_SC_PHYS_PAGES); 
      ram.free    = page_size * sysconf(_SC_AVPHYS_PAGES);
      ram.allowed = (ram.total/100) * cfg_percent_ram->get();
    }

    size_t sz = 0, rss = 0;
    std::ifstream buffer("/proc/self/statm");
    buffer >> sz >> rss;
    buffer.close();
    rss *= page_size;
    ram.used    = ram.used ? (ram.used+rss)/2 : rss;
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


  
  struct Component final {
    std::atomic<size_t> total    = 0;
    std::atomic<size_t> free     = 0;
    std::atomic<size_t> used     = 0;
    std::atomic<size_t> allowed  = 0;

    const std::string to_string(uint32_t base = 1) {
      std::string s("Res(");
      s.append("total=");
      s.append(std::to_string(total/base));
      s.append(" free=");
      s.append(std::to_string(free/base));
      s.append(" used=");
      s.append(std::to_string(used/base));
      s.append(" allowed=");
      s.append(std::to_string(allowed/base));
      s.append(")");
      return s;
    }
  };


  asio::high_resolution_timer*  m_timer; 
  gInt32tPtr                    cfg_percent_ram;
  std::function<void(size_t)>   release;

  int8_t                        next_major_chk = 0;
  Component                     ram;
  uint32_t                      page_size;
  // Component                     storage;

  
};



namespace Env {

inline static SWC::Resources Resources;

}


}

#endif // swc_core_Resources_h
