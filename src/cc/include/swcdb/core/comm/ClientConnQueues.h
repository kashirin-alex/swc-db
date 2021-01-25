/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_ClientConnQueues_h
#define swcdb_core_comm_ClientConnQueues_h

#include "swcdb/core/comm/SerializedClient.h"
#include "swcdb/core/comm/ClientConnQueue.h"

namespace SWC { namespace Comm { namespace client {

class ConnQueues;
typedef std::shared_ptr<ConnQueues> ConnQueuesPtr;

class Host final : public ConnQueue  {
  public:
  typedef std::shared_ptr<Host> Ptr;

  const EndPoints   endpoints;

  Host(const ConnQueuesPtr queues, const EndPoints& endpoints,
       const Config::Property::V_GINT32::Ptr keepalive_ms,
       const Config::Property::V_GINT32::Ptr again_delay_ms);

  virtual ~Host();

  bool connect() override;

  void close_issued() override;

  protected:
  const ConnQueuesPtr queues;
};


class ConnQueues final :
    private std::vector<Host::Ptr>,
    public std::enable_shared_from_this<ConnQueues> {

  public:

  const Serialized::Ptr                    service;
  const Config::Property::V_GINT32::Ptr    cfg_conn_timeout;
  const Config::Property::V_GINT32::Ptr    cfg_conn_probes;
  const Config::Property::V_GINT32::Ptr    cfg_keepalive_ms;
  const Config::Property::V_GINT32::Ptr    cfg_again_delay_ms;


  ConnQueues(const Serialized::Ptr service,
             const Config::Property::V_GINT32::Ptr timeout,
             const Config::Property::V_GINT32::Ptr probes,
             const Config::Property::V_GINT32::Ptr keepalive_ms,
             const Config::Property::V_GINT32::Ptr again_delay_ms) noexcept;

  ~ConnQueues();

  void print(std::ostream& out);

  Host::Ptr get(const EndPoints& endpoints);

  void remove(const EndPoints& endpoints);

  void stop();

  private:

  Core::MutexSptd m_mutex;

};



}}} //namespace SWC::Comm::client


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/ClientConnQueues.cc"
#endif


//
#endif // swcdb_core_comm_ClientConnQueues_h
