/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_ConnQueues_h
#define swc_client_ConnQueues_h

#include <memory>
#include "swcdb/core/comm/SerializedClient.h"
#include "swcdb/db/Protocol/Common/req/ConnQueue.h"

namespace SWC { namespace client {

class ConnQueues;
typedef std::shared_ptr<ConnQueues> ConnQueuesPtr;

class Host : public Protocol::Common::Req::ConnQueue  {
  public:
  typedef std::shared_ptr<Host> Ptr;

  const EndPoints   endpoints;

  Host(const ConnQueuesPtr queues, const EndPoints& endpoints, 
       const Property::V_GINT32::Ptr keepalive_ms, 
       const Property::V_GINT32::Ptr again_delay_ms);

  virtual ~Host();
  
  bool connect() override;

  void close_issued() override;

  protected:
  const ConnQueuesPtr queues;
};


class ConnQueues : public std::enable_shared_from_this<ConnQueues> {

  public:

  const Serialized::Ptr            service;
  const Property::V_GINT32::Ptr    cfg_conn_timeout;
  const Property::V_GINT32::Ptr    cfg_conn_probes;
  const Property::V_GINT32::Ptr    cfg_keepalive_ms;
  const Property::V_GINT32::Ptr    cfg_again_delay_ms;
  
  
  ConnQueues(const Serialized::Ptr service, 
             const Property::V_GINT32::Ptr timeout, 
             const Property::V_GINT32::Ptr probes, 
             const Property::V_GINT32::Ptr keepalive_ms,
             const Property::V_GINT32::Ptr again_delay_ms);

  virtual ~ConnQueues();

  std::string to_string();

  Host::Ptr get(const EndPoints& endpoints);

  void remove(const EndPoints& endpoints);

  private:

  std::mutex              m_mutex;
  std::vector<Host::Ptr>  m_hosts;
  
};



}}
// 
#endif // swc_client_ConnQueues_h