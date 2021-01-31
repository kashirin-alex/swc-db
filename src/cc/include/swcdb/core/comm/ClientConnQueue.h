/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_ClientConnQueue_h
#define swcdb_core_comm_ClientConnQueue_h

#include <queue>
#include <unordered_set>
#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/ConnHandler.h"

namespace SWC { namespace Comm { namespace client {


class ConnQueue;
typedef std::shared_ptr<ConnQueue> ConnQueuePtr;

class ConnQueueReqBase : public DispatchHandler {
  public:

  typedef std::shared_ptr<ConnQueueReqBase> Ptr;

  ConnQueueReqBase(bool insistent, const Buffers::Ptr& cbp=nullptr) noexcept;

  Ptr req() noexcept;

  virtual ~ConnQueueReqBase();

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  bool is_timeout(const Event::Ptr& ev);

  bool is_rsp(const Event::Ptr& ev);

  void request_again();

  virtual bool valid();

  virtual void handle_no_conn();

  void print(std::ostream& out);

  const bool            insistent;
  Buffers::Ptr          cbp;
  ConnQueuePtr          queue;
};



class ConnQueue :
    private std::queue<ConnQueueReqBase::Ptr>,
    public std::enable_shared_from_this<ConnQueue> {
  public:

  typedef ConnQueueReqBase ReqBase;

  ConnQueue(const IoContextPtr& ioctx,
            const Config::Property::V_GINT32::Ptr keepalive_ms=nullptr,
            const Config::Property::V_GINT32::Ptr again_delay_ms=nullptr);

  virtual ~ConnQueue();

  virtual bool connect();

  virtual void close_issued();

  void stop();

  EndPoint get_endpoint_remote() noexcept;

  EndPoint get_endpoint_local() noexcept;

  void put(const ReqBase::Ptr& req);

  void set(const ConnHandlerPtr& conn);

  void delay(const ReqBase::Ptr& req);

  void print(std::ostream& out);

  protected:

  const Config::Property::V_GINT32::Ptr  cfg_keepalive_ms;
  const Config::Property::V_GINT32::Ptr  cfg_again_delay_ms;

  private:

  void exec_queue();

  void run_queue();

  void schedule_close();

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
