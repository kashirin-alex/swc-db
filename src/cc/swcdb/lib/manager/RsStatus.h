
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RsStatus_h
#define swc_lib_manager_RsStatus_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"


namespace SWC { namespace server { namespace Mngr {

class RsQueue : public std::enable_shared_from_this<RsQueue> {

  public:

  typedef std::function<void(client::ClientConPtr conn)> ConnCb_t;

  RsQueue(EndPoints endpoints)
          : m_endpoints(endpoints), m_conn(nullptr),
            cfg_rs_conn_timeout(EnvConfig::settings()->get_ptr<gInt32t>(
              "swc.mngr.ranges.assign.RS.connection.timeout")),
            cfg_rs_conn_probes(EnvConfig::settings()->get_ptr<gInt32t>(
              "swc.mngr.ranges.assign.RS.connection.probes")) {}

  virtual ~RsQueue(){
    ConnCb_t req;
    std::lock_guard<std::mutex> lock(m_mutex);

    while(m_requests.size() > 0){
      req = m_requests.front();
      m_requests.pop();
      req(nullptr);
    }
  }
  
  void connect() {
    EnvClients::get()->rs_service->get_connection(
      m_endpoints, 
      [ptr=shared_from_this()] (client::ClientConPtr conn){ptr->set(conn);},
      std::chrono::milliseconds(cfg_rs_conn_timeout->get()), 
      cfg_rs_conn_probes->get()
    );
  }

  void set(client::ClientConPtr  conn) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_conn = conn;
    }
    run();
  }

  void put(ConnCb_t req) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_requests.push(req);

      if(m_running)
        return;
      m_running = true;
      if(m_conn == nullptr || !m_conn->is_open()) {
        connect();
        return;
      }
    }

    run();
  }

  void run() {
    ConnCb_t req;
    for(;;){
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_running = m_requests.size() > 0;
        if(!m_running)
          break;
        req = m_requests.front();
        m_requests.pop();
      }
      req(m_conn);
    }
  }

  private:
  
  std::mutex            m_mutex;
  const EndPoints       m_endpoints;
  client::ClientConPtr  m_conn;
  std::queue<ConnCb_t>  m_requests;
  bool                  m_running;

  gInt32tPtr          cfg_rs_conn_timeout;
  gInt32tPtr          cfg_rs_conn_probes;
};


class RsStatus : public Protocol::Params::HostEndPoints {

  public:

  enum State {
    NONE,
    ACK,
    REMOVED
  };
  
  RsStatus(): rs_id(0), state(State::NONE), 
              failures(0), total_ranges(0) {}
                       
  RsStatus(uint64_t rs_id, EndPoints endpoints)
          : rs_id(rs_id), state(State::NONE), 
            failures(0), total_ranges(0),
            Protocol::Params::HostEndPoints(endpoints),
            m_queue(std::make_shared<RsQueue>(endpoints)) {}

  virtual ~RsStatus(){}

  std::string to_string(){
    std::string s("[rs_id=");
    s.append(std::to_string(rs_id));
    s.append(", state=");
    s.append(std::to_string(state));
    s.append(", failures=");
    s.append(std::to_string(failures));
    s.append(", total_ranges=");
    s.append(std::to_string(total_ranges));
    s.append(", ");
    s.append(Protocol::Params::HostEndPoints::to_string());
    s.append("]");
    return s;
  }

  size_t encoded_length_internal() const {
    size_t len = 1 +  
               + Serialization::encoded_length_vi64(rs_id)
               + Protocol::Params::HostEndPoints::encoded_length_internal();
    return len;
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)state);
    Serialization::encode_vi64(bufp, rs_id);
    Protocol::Params::HostEndPoints::encode_internal(bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp){
    state = (State)Serialization::decode_i8(bufp, remainp);
    rs_id = Serialization::decode_vi64(bufp, remainp);
    Protocol::Params::HostEndPoints::decode_internal(version, bufp, remainp);
  }

  void init_queue(){
    m_queue = std::make_shared<RsQueue>(endpoints);
  }

  void put(RsQueue::ConnCb_t req){
    m_queue->put(req);
  }

  uint64_t   rs_id;
  State      state;

  int32_t    failures;
  size_t     total_ranges;
  // int32_t resource;
  
  private:
  std::shared_ptr<RsQueue> m_queue = nullptr;

};
typedef std::shared_ptr<RsStatus> RsStatusPtr;
typedef std::vector<RsStatusPtr>  RsStatusList;

}}}

#endif // swc_lib_manager_RsStatus_h