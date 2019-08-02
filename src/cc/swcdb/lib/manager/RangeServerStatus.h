
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServerStatus_h
#define swc_lib_manager_RangeServerStatus_h

#include "swcdb/lib/db/Protocol/params/HostEndPoints.h"


namespace SWC { namespace server { namespace Mngr {

struct IdsColumnRanges {
  public:
  int64_t              cid;
  std::vector<int64_t> ranges;

  IdsColumnRanges(int64_t cid, std::vector<int64_t> ranges) 
                  : cid(cid), ranges(ranges){}
  virtual ~IdsColumnRanges(){}
  

  std::string to_string(){
    std::string s("[cid=");
    s.append(std::to_string(cid));
    s.append(", ranges=(");
    
    for(auto rid : ranges){
      s.append(std::to_string(rid));
      s.append(",");
    }
    s.append(")]");
    return s;
  }
  
  bool has_range(int64_t chk_rid){
    for(auto rid : ranges) {
      if(rid == chk_rid)
        return true;
    }
    return false;
  }

};
typedef std::shared_ptr<IdsColumnRanges> IdsColumnRangesPtr;
typedef std::vector<IdsColumnRangesPtr> IdsColumnsRanges;

class RangeServerStatus : public Protocol::Params::HostEndPoints {

  public:
  RangeServerStatus(uint64_t rs_id, EndPoints endpoints)
                    : ack(false), rs_id(rs_id), failures(0),
                    Protocol::Params::HostEndPoints(endpoints) {}

  virtual ~RangeServerStatus(){}

  std::string to_string(){
    std::string s(" RS-status(");

    s.append("ID=");
    s.append(std::to_string(rs_id));
    s.append(", ACK=");
    s.append(ack?"true":"false");
    s.append(", columns=(");
    for(auto col : columns){
      s.append(col->to_string());
      s.append(", ");
    }
    s.append(")");
    
    s.append(", queued=(");
    for(auto col : queued){
      s.append(col->to_string());
      s.append(", ");
    }
    s.append("), ");

    s.append(Protocol::Params::HostEndPoints::to_string());
    
    s.append(")\n");
    return s;
  }

  size_t total_ranges(){
    size_t num = 0;
    for(auto col : columns)
      num += col->ranges.size();
    for(auto col : queued)
      num += col->ranges.size();
    return num;
  }

  bool has_range(int64_t cid, int64_t rid){
    for(auto col : columns) {
      if(col->cid != cid)
        continue;
      if(col->has_range(rid))
        return true;
    }
    for(auto col : queued) {
      if(col->cid != cid)
        continue;
      if(col->has_range(rid))
        return true;
    }
    return false;
  }

  bool           ack;
  uint64_t       rs_id;
  int            failures;

  IdsColumnsRanges columns;
  IdsColumnsRanges queued;
  // num-cols, cols{cid,num-ranges{ranges}} ~= 1024x1024=> 8MB
};
typedef std::shared_ptr<RangeServerStatus> RangeServerStatusPtr;

}}}

#endif // swc_lib_manager_RangeServerStatus_h