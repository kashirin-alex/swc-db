
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrRangeGetRs_h
#define swc_db_protocol_params_MngrRangeGetRs_h

#include "swcdb/lib/core/Serializable.h"
#include "HostEndPoints.h"
#include "swcdb/lib/db/Cells/Intervals.h"

namespace SWC {
namespace Protocol {
namespace Params {

// flag(if cid==1) 
//      in(cid+intervals)  out(cid + rid + rs-endpoints + ?next) 
// else 
//      in(cid+rid)        out(cid + rid + rs-endpoints)

//req-mngr-master, 1(cid)+n(cid):intervals => cid + rid + rs(endpoints) + ?next
//              ->  req-rs. 1(cid)+rid+n(cid):intervals => 2(cid) + rid + ?next
//req-mngr-meta,   2(cid)+rid              => cid + rid + rs(endpoints)
//              ->  req-rs. 2(cid)+rid+n(cid):intervals => n(cid) + rid + ?next
//req-mngr,        n(cid)+rid              => cid + rid + rs(endpoints)
//              ->  req-rs. n(cid)+scanspecs => results

class MngrRangeGetRsReq : public Serializable {
  public:
  MngrRangeGetRsReq(): cid(0), rid(0) {}
  MngrRangeGetRsReq(int64_t cid, Cells::Intervals::Ptr intervals)
                   : cid(cid), rid(0), intervals(intervals) {}
  MngrRangeGetRsReq(int64_t cid, int64_t rid)
                   : cid(cid), rid(rid), intervals(nullptr) {}

  virtual ~MngrRangeGetRsReq(){}
  
  int64_t               cid;
  int64_t               rid;
  Cells::Intervals::Ptr intervals;
  
  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi64(cid)
         + (cid==1 ? 
            intervals->encoded_length() : 
            Serialization::encoded_length_vi64(rid));
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    if(cid==1)
      intervals->encode(bufp);
    else
      Serialization::encode_vi64(bufp, rid);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    if(cid == 1) {
      intervals = std::make_shared<Cells::Intervals>();
      intervals->decode(bufp, remainp);
    } else 
      rid = Serialization::decode_vi64(bufp, remainp);
  }

};



class MngrRangeGetRsRsp  : public HostEndPoints {
  public:

  MngrRangeGetRsRsp(): cid(0), rid(0) {}

  MngrRangeGetRsRsp(int64_t cid, int64_t rid, const EndPoints& endpoints, bool next) 
                    : cid(cid), rid(rid), HostEndPoints(endpoints), next(next) {     
  }

  int64_t  cid; 
  int64_t  rid; 
  bool     next;

  const std::string to_string() {
    std::string s("Range-RS(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" ");
    s.append(HostEndPoints::to_string());
    if(cid == 1) {
      s.append(" next=");
      s.append(std::to_string(next));
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
         + (cid==1?1:0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    HostEndPoints::encode_internal(bufp);
    if(cid == 1)
      Serialization::encode_bool(bufp, next);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    HostEndPoints::decode_internal(version, bufp, remainp);
    if(cid == 1)
      next = Serialization::decode_bool(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_MngrRangeGetRs_h
