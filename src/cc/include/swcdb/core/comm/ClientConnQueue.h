/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_ClientConnQueue_h
#define swcdb_core_comm_ClientConnQueue_h


#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"
#include <queue>
#include <unordered_set>


namespace SWC { namespace Comm { namespace client {


class ConnQueue;
typedef std::shared_ptr<ConnQueue> ConnQueuePtr;

class ConnQueueReqBase : public DispatchHandler {
  public:

  typedef std::shared_ptr<ConnQueueReqBase> Ptr;
  Buffers::Ptr                              cbp;
  ConnQueuePtr                              queue;

  SWC_CAN_INLINE
  ConnQueueReqBase(Buffers::Ptr&& a_cbp) noexcept
                  : cbp(std::move(a_cbp)), queue(nullptr) { }

  SWC_CAN_INLINE
  ConnQueueReqBase(Buffers::Ptr&& a_cbp,
                   const ConnQueuePtr& a_queue) noexcept
                  : cbp(std::move(a_cbp)), queue(a_queue) { }

  SWC_CAN_INLINE
  Ptr req() noexcept {
    return std::dynamic_pointer_cast<ConnQueueReqBase>(shared_from_this());
  }

  virtual bool insistent() noexcept { return false; };

  void request_again();

  void print(std::ostream& out);

  protected:

  virtual ~ConnQueueReqBase() noexcept { }

};



class ConnQueue :
    private std::queue<ConnQueueReqBase::Ptr>,
    public std::enable_shared_from_this<ConnQueue> {
  public:

  typedef ConnQueueReqBase ReqBase;

  SWC_CAN_INLINE
  ConnQueue(const IoContextPtr& ioctx,
            const Config::Property::Value_int32_g::Ptr keepalive_ms=nullptr,
            const Config::Property::Value_int32_g::Ptr again_delay_ms=nullptr)
            : cfg_keepalive_ms(keepalive_ms),
              cfg_again_delay_ms(again_delay_ms),
              m_ioctx(ioctx), m_conn(nullptr),
              m_connecting(false),
              m_timer(cfg_keepalive_ms
                        ? new asio::high_resolution_timer(m_ioctx->executor())
                        : nullptr) {
  }

  virtual ~ConnQueue() noexcept {
    delete m_timer;
  }

  virtual bool connect()  {
    return false; // not implemented by default
  }

  virtual void close_issued() { }

  void stop();

  EndPoint get_endpoint_remote() noexcept;

  EndPoint get_endpoint_local() noexcept;

  void put(const ReqBase::Ptr& req);

  void set(const ConnHandlerPtr& conn);

  void delay(ReqBase::Ptr&& req);

  void print(std::ostream& out);

  protected:

  const Config::Property::Value_int32_g::Ptr  cfg_keepalive_ms;
  const Config::Property::Value_int32_g::Ptr  cfg_again_delay_ms;

  private:

  void exec_queue();

  void run_queue();

  void schedule_close(bool closing);

  Core::MutexSptd                                   m_mutex;
  IoContextPtr                                      m_ioctx;
  ConnHandlerPtr                                    m_conn;
  bool                                              m_connecting;
  Core::StateRunning                                m_q_state;
  asio::high_resolution_timer*                      m_timer;
  std::unordered_set<asio::high_resolution_timer*>  m_delayed;


};


}}} //namespace SWC::Comm::client



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ClientConnQueue.cc"
#endif


#endif // swcdb_core_comm_ClientConnQueue_h
