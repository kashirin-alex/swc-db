
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_IoContext_h
#define swc_core_comm_IoContext_h


namespace SWC {
  
typedef asio::executor_work_guard<asio::io_context::executor_type> IO_DoWork;
typedef std::shared_ptr<IO_DoWork> IO_DoWorkPtr;
typedef std::shared_ptr<asio::signal_set> IO_SignalsPtr;

class IoContext {
  public:
  IoContext(int32_t size) 
    : m_run(true),
      m_ioctx(std::make_shared<asio::io_context>(size)),
      m_wrk(std::make_shared<IO_DoWork>(asio::make_work_guard(*m_ioctx.get())))
  {
    (new std::thread(
      [io_ptr=m_ioctx, run=&m_run]{ 
        do{
          io_ptr->run();
          HT_DEBUG("IO stopped, restarting");
          io_ptr->restart();
        }while(run->load());
        HT_DEBUG("IO exited");
      })
    )->detach();
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
    std::cout << "Holding-on for IO-ctx to finish \n";

    m_run.store(false);
    m_wrk->reset();
    // m_wrk->get_executor().context().stop();
  }

  virtual ~IoContext(){
    std::cout << " ~IoContext\n";
  }

  private:
  std::atomic<bool>   m_run;
  IOCtxPtr            m_ioctx;
  IO_DoWorkPtr        m_wrk = nullptr;
  IO_SignalsPtr       m_signals;

};
typedef std::shared_ptr<IoContext> IoContextPtr;


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
  

  EnvIoCtx(int32_t size) : m_io(std::make_shared<IoContext>(size)) { }

  virtual ~EnvIoCtx(){
    std::cout << " ~EnvIoCtx\n";
  }

  private:
  IoContextPtr  m_io;
  inline static std::shared_ptr<EnvIoCtx> m_env = nullptr;
};
}

#endif // swc_core_comm_IoContext_h