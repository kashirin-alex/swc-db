
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_IoContext_h
#define swc_core_comm_IoContext_h

#include <asio.hpp>
#include "swcdb/core/Logger.h"

namespace SWC {
  
typedef std::shared_ptr<asio::signal_set>   IO_SignalsPtr;
typedef std::shared_ptr<asio::io_context>   IOCtxPtr;

class IoContext final {
  public:

  typedef std::shared_ptr<IoContext>  Ptr;

  static Ptr make(const std::string name, int32_t size);

  std::atomic<bool>                   running;

  IoContext(const std::string name, int32_t size);

  void run(Ptr ptr);

  void do_run();
  
  IOCtxPtr shared();

  asio::io_context* ptr();

  void set_signals();

  IO_SignalsPtr signals();

  void stop();

  const int32_t get_size() const;
  
  ~IoContext();

  private:
  const std::string   m_name;
  IOCtxPtr            m_ioctx;
  IO_SignalsPtr       m_signals;
  asio::thread_pool   m_pool;
  asio::executor_work_guard<asio::io_context::executor_type> m_wrk;
  int32_t             m_size;
};


namespace Env {
class IoCtx final {
  public:

  static void init(int32_t size);

  static const bool ok();
  
  static IoContext::Ptr io();
  
  static const bool stopping();

  IoCtx(int32_t size);

  ~IoCtx();

  private:
  IoContext::Ptr                       m_io;
  inline static std::shared_ptr<IoCtx> m_env = nullptr;
};
} // namespace Env

} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/IoContext.cc"
#endif 

#endif // swc_core_comm_IoContext_h