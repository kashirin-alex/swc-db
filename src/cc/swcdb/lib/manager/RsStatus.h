
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RsStatus_h
#define swc_lib_manager_RsStatus_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"


namespace SWC { namespace server { namespace Mngr {


class RsStatus : public Protocol::Params::HostEndPoints {

  public:

  enum State {
    NONE,
    ACK,
    REMOVED
  };
  
  RsStatus(): rs_id(0), state(State::NONE), 
              failures(0), total_ranges(0) {}
                       
  RsStatus(uint64_t rs_id, const EndPoints& endpoints)
          : rs_id(rs_id), state(State::NONE), 
            failures(0), total_ranges(0),
            Protocol::Params::HostEndPoints(endpoints) {
    init_queue();
  }

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
    if(m_queue != nullptr) {
      s.append(", ");
      s.append(m_queue->to_string());
    }
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
    m_queue = std::make_shared<client::Rs::Host>(
      endpoints,
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.RS.connection.timeout"),
      Env::Config::settings()->get_ptr<gInt32t>(
        "swc.mngr.ranges.assign.RS.connection.probes")
    );
  }

  void put(Protocol::Req::ConnQueue::ReqBase::Ptr req){
    m_queue->put(req);
  }

  void stop(){
    m_queue->stop();
  }

  uint64_t   rs_id;
  State      state;

  int32_t    failures;
  size_t     total_ranges;
  // int32_t resource;
  
  private:
  client::Rs::Host::Ptr m_queue = nullptr;

};
typedef std::shared_ptr<RsStatus> RsStatusPtr;
typedef std::vector<RsStatusPtr>  RsStatusList;

}}}

#endif // swc_lib_manager_RsStatus_h