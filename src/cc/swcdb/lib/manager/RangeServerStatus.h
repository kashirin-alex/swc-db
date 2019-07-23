
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServerStatus_h
#define swc_lib_manager_RangeServerStatus_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"


namespace SWC { namespace server { namespace Mngr {

class RangeServerStatus : public Protocol::Params::HostEndPoints {

  public:
  RangeServerStatus(uint64_t rs_id, EndPoints endpoints)
                    : ack(false), role(Types::RsRole::DATA), rs_id(rs_id), 
                    Protocol::Params::HostEndPoints(endpoints) {}

  virtual ~RangeServerStatus(){}

  std::string to_string(){
    std::string s(" RS-status(");

    s.append("ID=");
    s.append(std::to_string(rs_id));
    s.append(" ACK=");
    s.append(ack?"true":"false");
    s.append(" ROLE=");
    s.append(
      role==Types::RsRole::MASTER?
      "master":(role==Types::RsRole::META?"meta":"data"));
    s.append(" ");
    s.append(Protocol::Params::HostEndPoints::to_string());
    
    s.append(")\n");
    return s;
  }

  bool      ack;
  uint64_t  rs_id;
  Types::RsRole  role;

};
typedef std::shared_ptr<RangeServerStatus> RangeServerStatusPtr;

}}}

#endif // swc_lib_manager_RangeServerStatus_h