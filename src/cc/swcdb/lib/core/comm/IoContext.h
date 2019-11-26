
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_IoContext_h
#define swc_core_comm_IoContext_h

#include "../Mutex.h"

namespace SWC {
  
typedef std::shared_ptr<asio::signal_set>   IO_SignalsPtr;
typedef std::shared_ptr<asio::io_context>   IOCtxPtr;

class IoContext {
  public:

  typedef std::shared_ptr<IoContext>  Ptr;
  std::atomic<bool>                   running;

  IoContext(const std::string name, int32_t size) 
    : m_name(name), running(true), m_size(size),
      m_pool(asio::thread_pool(size)),
      m_ioctx(std::make_shared<asio::io_context>(size)),
      m_wrk(asio::make_work_guard(*m_ioctx.get()))
  { 
    HT_ASSERT(size>0);
  }

  void run(Ptr p){
    HT_DEBUGF("Starting IO-ctx(%s)", m_name.c_str());
    for(int n=0;n<m_size;n++)
      asio::post(m_pool, std::bind(&IoContext::do_run, p));
  }

  void do_run(){
    do{
      m_ioctx->run();
      m_ioctx->restart();
    }while(running);
  }
  
  IOCtxPtr shared(){
    return m_ioctx;
  }

  asio::io_context* ptr(){
    return m_ioctx.get();
  }

  void set_signals(){
    m_signals = std::make_shared<asio::signal_set>(
      *m_ioctx.get(), SIGINT, SIGTERM);
  }

  IO_SignalsPtr signals(){
    return m_signals;
  }

  void stop(){
    HT_DEBUGF("Stopping IO-ctx(%s)", m_name.c_str());
    running.store(false);
    m_wrk.reset();
    
    // hold on for IO to finish
    for(int i=0;i<10;i++){
      if(m_ioctx->stopped())
        break;
      HT_DEBUGF("Waiting for IO-ctx(%s)", m_name.c_str());
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    HT_DEBUGF("Wait for IO-ctx(%s) finished %sgracefully", 
              m_name.c_str(), m_ioctx->stopped()?"":"not ");

    if(!m_ioctx->stopped())
      m_ioctx->stop();
  }

  int32_t get_size() {
    return m_size;
  }
  
  virtual ~IoContext(){}

  private:
  const std::string   m_name;
  IOCtxPtr            m_ioctx;
  IO_SignalsPtr       m_signals;
  asio::thread_pool   m_pool;
  asio::executor_work_guard<asio::io_context::executor_type> m_wrk;
  int32_t             m_size;
};


namespace Env {
class IoCtx {
  public:

  static void init(int32_t size) {
    m_env = std::make_shared<IoCtx>(size);
  }

  static bool ok(){
    return m_env != nullptr;
  }
  
  static IoContext::Ptr io(){
    HT_ASSERT(ok());

    return m_env->m_io;
  }
  
  static bool stopping(){
    return !m_env->m_io->running;
  }

  IoCtx(int32_t size) : m_io(std::make_shared<IoContext>("Env", size)) { 
    m_io->run(m_io);
  }

  virtual ~IoCtx(){ }

  private:
  IoContext::Ptr                         m_io;
  inline static std::shared_ptr<IoCtx> m_env = nullptr;
};
}

}

#endif // swc_core_comm_IoContext_h