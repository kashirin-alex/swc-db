
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_Ranger_h
#define swc_lib_manager_Ranger_h

#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"


namespace SWC { namespace server { namespace Mngr {


class Ranger : public Protocol::Common::Params::HostEndPoints {

  public:

  enum State {
    NONE,
    AWAIT,
    ACK,
    REMOVED
  };
  typedef std::shared_ptr<Ranger> Ptr;

  Ranger(): id(0), state(State::NONE), 
            failures(0), total_ranges(0) {}
                       
  Ranger(uint64_t id, const EndPoints& endpoints)
        : id(id), state(State::NONE), 
          failures(0), total_ranges(0),
          Protocol::Common::Params::HostEndPoints(endpoints) {
  }

  virtual ~Ranger(){}

  std::string to_string(){
    std::string s("[id=");
    s.append(std::to_string(id));
    s.append(", state=");
    s.append(std::to_string(state));
    s.append(", failures=");
    s.append(std::to_string(failures));
    s.append(", total_ranges=");
    s.append(std::to_string(total_ranges));
    s.append(", ");
    s.append(Protocol::Common::Params::HostEndPoints::to_string());
    if(m_queue != nullptr) {
      s.append(", ");
      s.append(m_queue->to_string());
    }
    s.append("]");
    return s;
  }

  size_t encoded_length_internal() const {
    size_t len = 1
      + Serialization::encoded_length_vi64(id)
      + Protocol::Common::Params::HostEndPoints::encoded_length_internal();
    return len;
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)state);
    Serialization::encode_vi64(bufp, id);
    Protocol::Common::Params::HostEndPoints::encode_internal(bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp){
    state = (State)Serialization::decode_i8(bufp, remainp);
    id = Serialization::decode_vi64(bufp, remainp);
    Protocol::Common::Params::HostEndPoints::decode_internal(
      version, bufp, remainp);
  }

  void init_queue(){
    m_queue = Env::Clients::get()->rgr->get(endpoints);
  }

  void put(Protocol::Common::Req::ConnQueue::ReqBase::Ptr req){
    m_queue->put(req);
  }

  void stop(){
    m_queue->stop();
  }

  void pending_id(Protocol::Common::Req::ConnQueue::ReqBase::Ptr req){
    m_pending_id.push(req);
  }

  bool pending_id_pop(Protocol::Common::Req::ConnQueue::ReqBase::Ptr& req){
    if(m_pending_id.empty())
      return false;
    req = m_pending_id.front();
    m_pending_id.pop();
    return true;
  }

  uint64_t   id;
  State      state;

  std::atomic<int32_t>  failures;
  std::atomic<size_t>   total_ranges;
  // int32_t resource;
  
  private:
  client::Host::Ptr m_queue = nullptr;
  std::queue<Protocol::Common::Req::ConnQueue::ReqBase::Ptr> m_pending_id;

};
typedef std::vector<Ranger::Ptr>  RangerList;

}}}

#endif // swc_lib_manager_Ranger_h