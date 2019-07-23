/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_manager_HostStatus_h
#define swc_app_manager_HostStatus_h

#include "swcdb/lib/db/Types/MngrState.h"
#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"

namespace SWC { namespace server { namespace Mngr {


struct HostStatus : public Protocol::Params::HostEndPoints {
  public:

  HostStatus() {}

  HostStatus(uint64_t  begin, uint64_t  end,
             EndPoints points, client::ClientConPtr c, uint32_t pr)
             : col_begin(begin), col_end(end), 
               Protocol::Params::HostEndPoints(points), conn(c), priority(pr),
               state(Types::MngrState::NOTSET) { }
  
  virtual ~HostStatus(){ }

  size_t encoded_length(){
    size_t len = 5 
               + Serialization::encoded_length_vi64(col_begin)
               + Serialization::encoded_length_vi64(col_end)
               + Protocol::Params::HostEndPoints::encoded_length();
    return len;
  }

  void encode(uint8_t **bufp){
    Serialization::encode_i32(bufp, priority);
    Serialization::encode_i8(bufp, (uint8_t)state);
    Serialization::encode_vi64(bufp, col_begin);
    Serialization::encode_vi64(bufp, col_end);
    Protocol::Params::HostEndPoints::encode(bufp);
  }

  void decode(const uint8_t **bufp, size_t *remainp) {
    priority = Serialization::decode_i32(bufp, remainp);
    state = (Types::MngrState)Serialization::decode_i8(bufp, remainp);
    col_begin = Serialization::decode_vi64(bufp, remainp);
    col_end = Serialization::decode_vi64(bufp, remainp);

    Protocol::Params::HostEndPoints::decode(bufp, remainp);
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

    s.append(" ");
    s.append(Protocol::Params::HostEndPoints::to_string());
    return s;
  }

  uint32_t     priority;
  Types::MngrState  state;
  uint64_t     col_begin;
  uint64_t     col_end;

  client::ClientConPtr  conn; // mngr-inchain
};

typedef std::shared_ptr<HostStatus> HostStatusPtr;
typedef std::vector<HostStatusPtr> HostStatuses;




}}}

#endif // swc_app_manager_HostStatus_h
