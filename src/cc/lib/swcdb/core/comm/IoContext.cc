
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/Error.h"

namespace SWC {

IoContext::Ptr IoContext::make(const std::string name, int32_t size) {
  auto ptr = std::make_shared<IoContext>(name, size);
  ptr->run(ptr);
  return ptr;
}

IoContext::IoContext(const std::string name, int32_t size) 
                    : m_name(name), running(true), m_size(size),
                      m_pool(asio::thread_pool(size)),
                      m_ioctx(std::make_shared<asio::io_context>(size)),
                      m_wrk(asio::make_work_guard(*m_ioctx.get())) { 
    SWC_ASSERT(size>0);
}
  
IoContext::~IoContext() { }

void IoContext::run(IoContext::Ptr ptr){
  SWC_LOGF(LOG_DEBUG, "Starting IO-ctx(%s)", m_name.c_str());
  for(int n=0;n<m_size;++n)
    asio::post(m_pool, [ptr](){ptr->do_run();});
}

void IoContext::do_run() {
  do{
    m_ioctx->run();
    m_ioctx->restart();
  }while(running);
}
  
IOCtxPtr IoContext::shared() {
  return m_ioctx;
}

asio::io_context* IoContext::ptr() {
  return m_ioctx.get();
}

void IoContext::set_signals() {
  m_signals = std::make_shared<asio::signal_set>(
    *m_ioctx.get(), SIGINT, SIGTERM);
}

IO_SignalsPtr IoContext::signals() {
  return m_signals;
}

void IoContext::set_periodic_timer(const gInt32tPtr ms, 
                                   PeriodicTimer::Call_t call) {
  m_periodic_timers.set(ms, call, ptr());
}

void IoContext::stop() {
  SWC_LOGF(LOG_DEBUG, "Stopping IO-ctx(%s)", m_name.c_str());
  
  m_periodic_timers.stop();

  running.store(false);

  m_wrk.reset();
    
  // hold on for IO to finish
  for(int i=0;i<10;++i){
    if(m_ioctx->stopped())
      break;
    SWC_LOGF(LOG_DEBUG, "Waiting for IO-ctx(%s)", m_name.c_str());
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  SWC_LOGF(LOG_DEBUG, "Wait for IO-ctx(%s) finished %sgracefully", 
           m_name.c_str(), m_ioctx->stopped()?"":"not ");

  if(!m_ioctx->stopped())
    m_ioctx->stop();
}

const int32_t IoContext::get_size() const {
  return m_size;
}


namespace Env {

void IoCtx::init(int32_t size) {
  m_env = std::make_shared<IoCtx>(size);
}

const bool IoCtx::ok() {
  return m_env != nullptr;
}
  
IoContext::Ptr IoCtx::io() {
  SWC_ASSERT(ok());
  return m_env->m_io;
}
  
const bool IoCtx::stopping(){
  return !m_env->m_io->running;
}

IoCtx::IoCtx(int32_t size) 
            : m_io(std::make_shared<IoContext>("Env", size)) { 
  m_io->run(m_io);
}

IoCtx::~IoCtx() { }

} // namespace Env

}