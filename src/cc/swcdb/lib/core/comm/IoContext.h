
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_IoContext_h
#define swc_core_comm_IoContext_h


namespace SWC {
  
typedef asio::executor_work_guard<asio::io_context::executor_type> IO_DoWork;
typedef std::shared_ptr<IO_DoWork> IO_DoWorkPtr;
typedef std::shared_ptr<asio::signal_set> IO_SignalsPtr;

class IoContext;
typedef std::shared_ptr<IoContext> IoContextPtr;

class IoContext {
  public:
  IoContext(const std::string name, int32_t size) 
    : m_name(name), m_run(true), m_size(size),
      m_pool(std::make_shared<asio::thread_pool>(size)),
      m_ioctx(std::make_shared<asio::io_context>(size)),
      m_wrk(std::make_shared<IO_DoWork>(asio::make_work_guard(*m_ioctx.get())))
  { }

  void run(IoContextPtr p){
    HT_DEBUGF("Starting IO-ctx(%s)", m_name.c_str());
    for(int n=0;n<m_size;n++)
      asio::post(*m_pool.get(), std::bind(&IoContext::do_run, p));
  }

  void do_run(){
    do{
      m_ioctx->run();
      m_ioctx->restart();
    }while(m_run);
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
    m_run.store(false);
    m_wrk->reset();
    
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

  virtual ~IoContext(){}

  private:
  const std::string   m_name;
  std::atomic<bool>   m_run;
  IOCtxPtr            m_ioctx;
  IO_DoWorkPtr        m_wrk = nullptr;
  IO_SignalsPtr       m_signals;
  std::shared_ptr<asio::thread_pool>   m_pool;
  int32_t             m_size;
};


class EnvIoCtx {
  public:

  static void init(int32_t size) {
    m_env = std::make_shared<EnvIoCtx>(size);
  }

  static bool ok(){
    return m_env != nullptr;
  }

  static IoContextPtr io(){
    HT_ASSERT(ok());

    return m_env->m_io;
  }
  

  EnvIoCtx(int32_t size) : m_io(std::make_shared<IoContext>("Env", size)) { 
    m_io->run(m_io);
  }

  virtual ~EnvIoCtx(){ }

  private:
  IoContextPtr  m_io;
  inline static std::shared_ptr<EnvIoCtx> m_env = nullptr;
};
}

#endif // swc_core_comm_IoContext_h