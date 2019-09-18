
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrRangeGetRs_h
#define swc_db_protocol_params_MngrRangeGetRs_h

#include "swcdb/lib/core/Serializable.h"
#include "HostEndPoints.h"
#include "swcdb/lib/db/ScanSpecs/ScanSpecs.h"

namespace SWC {
namespace Protocol {
namespace Params {

// flag(if cid==1) 
//      in(n(cid)+ScanSpecs::CellsInterval)  out(cid + rid + rs-endpoints + ?next) 
// else 
//      in(cid+rid)                         out(cid + rid + rs-endpoints)

//req-mngr-master, n(cid)+CellsInterval     => cid(1) + rid + rs(endpoints) + ?next
//            ->  req-rs. 1(cid)+rid+n(cid):CellsInterval => 2(cid) + rid + ?next
//req-mngr-meta,   2(cid)+rid              => cid(2) + rid + rs(endpoints)
//            ->  req-rs. 2(cid)+rid+n(cid):CellsInterval => n(cid) + rid + ?next
//req-mngr,        n(cid)+rid              => cid(n) + rid + rs(endpoints)
//            ->  req-rs. n(cid)+rid+ScanSpecs::CellsInterval => results

class MngrRangeGetRsReq : public Serializable {
  public:
  MngrRangeGetRsReq(): cid(0), rid(-1) {}

  MngrRangeGetRsReq(int64_t cid, ScanSpecs::CellsInterval& intervals)
                   : cid(cid), rid(0), intervals(intervals) {}

  MngrRangeGetRsReq(int64_t cid, int64_t rid)
                   : cid(cid), rid(rid) {}

  virtual ~MngrRangeGetRsReq(){}
  
  int64_t                  cid;
  int64_t                  rid;
  ScanSpecs::CellsInterval intervals;
  
  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi64(cid)
         + Serialization::encoded_length_vi64(rid)
         + (rid == 0 ? intervals.encoded_length() : 0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    if(rid == 0)
      intervals.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    if(rid == 0) {
      intervals.keys_start.keys.push_back(ScanSpecs::Key());
      intervals.keys_finish.keys.push_back(ScanSpecs::Key());
      intervals.decode(bufp, remainp);
    }
  }

};



class MngrRangeGetRsRsp  : public HostEndPoints {
  public:

  MngrRangeGetRsRsp(): cid(0), rid(0) {}

  MngrRangeGetRsRsp(int64_t cid, int64_t rid, const EndPoints& endpoints) 
                    :  HostEndPoints(endpoints), cid(cid), rid(rid) {
  }

  MngrRangeGetRsRsp(int64_t cid, int64_t rid, const EndPoints& endpoints, 
                    ScanSpecs::Keys next_keys)  
                    : HostEndPoints(endpoints), cid(cid), rid(rid), 
                      next_keys(next_keys) {
  }

  int64_t  cid; 
  int64_t  rid; 
  ScanSpecs::Keys next_keys;

  const std::string to_string() {
    std::string s("Range-RS(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" ");
    s.append(HostEndPoints::to_string());
    if(cid == 1) {
      s.append(" next_");
      s.append(next_keys.to_string());
    }
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi64(cid)
         + Serialization::encoded_length_vi64(rid)
         + HostEndPoints::encoded_length_internal()
         + (cid==1?next_keys.encoded_length(1):0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    HostEndPoints::encode_internal(bufp);
    if(cid == 1)
      next_keys.encode(bufp, 1);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    HostEndPoints::decode_internal(version, bufp, remainp);
    if(cid == 1)
      next_keys.decode(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_MngrRangeGetRs_h
