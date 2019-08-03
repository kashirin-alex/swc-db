
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
                    : rs_id(rs_id), ack(false), failures(0), total_ranges(0),
                    Protocol::Params::HostEndPoints(endpoints) {}

  virtual ~RangeServerStatus(){}

  std::string to_string(){
    std::string s("[rs_id=");
    s.append(std::to_string(rs_id));
    s.append(", ACK=");
    s.append(ack?"true":"false");
    s.append(", failures=");
    s.append(std::to_string(failures));
    s.append(", total_ranges=");
    s.append(std::to_string(total_ranges));
    s.append(", ");
    s.append(Protocol::Params::HostEndPoints::to_string());
    s.append("]");
    return s;
  }

  uint64_t       rs_id;
  bool           ack;

  int            failures;
  size_t         total_ranges;
};
typedef std::shared_ptr<RangeServerStatus> RangeServerStatusPtr;
typedef  std::vector<RangeServerStatusPtr> RangeServerStatusList;

}}}

#endif // swc_lib_manager_RangeServerStatus_h