/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_HostStatus_h
#define swc_app_manager_HostStatus_h

namespace SWC { namespace server { namespace Mngr {
  
enum State {
  NOTSET = 0,
  OFF = 1,
  STANDBY = 2,
  WANT = 3,
  NOMINATED = 4,
  ACTIVE = 5
};


struct HostStatus {
  public:

  HostStatus() {}

  HostStatus(uint64_t  begin, uint64_t  end,
             EndPoints points, client::ClientConPtr c, uint32_t pr)
             : col_begin(begin), col_end(end), 
               endpoints(points), conn(c), priority(pr),
               state(State::NOTSET) { }
  
  virtual ~HostStatus(){ }

  size_t encoded_length(){
    size_t len = 5 
               + Serialization::encoded_length_vi64(col_begin)
               + Serialization::encoded_length_vi64(col_end)
               + 4;
    for(auto endpoint : endpoints)
      len += Serialization::encoded_length(endpoint);
    return len;
  }

  void encode(uint8_t **bufp){
    Serialization::encode_i32(bufp, priority);
    Serialization::encode_i8(bufp, (uint8_t)state);
    Serialization::encode_vi64(bufp, col_begin);
    Serialization::encode_vi64(bufp, col_end);
    Serialization::encode_i32(bufp, endpoints.size());
    for(auto endpoint : endpoints)
      Serialization::encode(endpoint, bufp);
  }

  void decode(const uint8_t **bufp, size_t *remainp) {
    priority = Serialization::decode_i32(bufp, remainp);
    state = (State)Serialization::decode_i8(bufp, remainp);
    col_begin = Serialization::decode_vi64(bufp, remainp);
    col_end = Serialization::decode_vi64(bufp, remainp);
    size_t len = Serialization::decode_i32(bufp, remainp);
    for(size_t i=0;i<len;i++)
      endpoints.push_back(Serialization::decode(bufp, remainp));
  }

  std::string to_string(){
    std::string s("HostStatus:");
    
    s.append(" priority=");
    s.append(std::to_string(priority));

    s.append(" state=");
    s.append(std::to_string((uint8_t)state));

    s.append(" col(begin=");
    s.append(std::to_string(col_begin));
    s.append(" end=");
    s.append(std::to_string(col_end));
    s.append(") endpoints=(");
    for(auto e : endpoints){
      s.append("[");
      s.append(e.address().to_string());
      s.append("]:");
      s.append(std::to_string(e.port()));
      s.append(",");
    }
    s.append(")");
    return s;
  }

  uint32_t     priority;
  State        state;
  uint64_t     col_begin;
  uint64_t     col_end;
  EndPoints    endpoints;

  client::ClientConPtr  conn; // mngr-inchain
};

typedef std::shared_ptr<HostStatus> HostStatusPtr;
typedef std::vector<HostStatusPtr> HostStatuses;




}}}

#endif // swc_app_manager_HostStatus_h
