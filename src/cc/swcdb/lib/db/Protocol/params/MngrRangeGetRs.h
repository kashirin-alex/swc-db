
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrRangeGetRs_h
#define swc_db_protocol_params_MngrRangeGetRs_h

#include "swcdb/lib/core/Serializable.h"
#include "HostEndPoints.h"
#include "swcdb/lib/db/Cells/Cell.h"
#include "swcdb/lib/db/Cells/SpecsInterval.h"

namespace SWC {
namespace Protocol {
namespace Params {

// flag(if cid==1) 
//      in(n(cid)+Specs::Interval)  out(cid + rid + rs-endpoints + ?next) 
// else 
//      in(cid+rid)                         out(cid + rid + rs-endpoints)

//req-mngr-master, n(cid)+Cells::Interval     => cid(1) + rid + rs(endpoints) + ?next
//            ->  req-rs. 1(cid)+rid+n(cid):Cells::Interval => 2(cid) + rid + ?next
//req-mngr-meta,   2(cid)+rid              => cid(2) + rid + rs(endpoints)
//            ->  req-rs. 2(cid)+rid+n(cid):Cells::Interval => n(cid) + rid + ?next
//req-mngr,        n(cid)+rid              => cid(n) + rid + rs(endpoints)
//            ->  req-rs. n(cid)+rid+Specs::Interval => results

class MngrRangeGetRsReq : public Serializable {
  public:

  enum By {
    KEYS,
    INTERVALS,
    RID
  };

  MngrRangeGetRsReq(): cid(0), rid(-1) {}

  MngrRangeGetRsReq(int64_t cid, const DB::Cell::Key& key)
                   : cid(cid), rid(0), key(key), by(By::KEYS) {}

  MngrRangeGetRsReq(int64_t cid, const DB::Specs::Interval& interval)
                   : cid(cid), rid(0), interval(interval), by(By::INTERVALS) {}

  MngrRangeGetRsReq(int64_t cid, int64_t rid)
                   : cid(cid), rid(rid), by(By::RID) {}

  virtual ~MngrRangeGetRsReq(){}
  
  By                  by;
  int64_t             cid;
  int64_t             rid;
  DB::Specs::Interval interval;
  DB::Cell::Key       key;
  
  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    size_t len = 1+Serialization::encoded_length_vi64(cid);
    switch(by) {
      case By::RID:
        return len + Serialization::encoded_length_vi64(rid);
      case By::INTERVALS:
        return len + interval.encoded_length();
      case By::KEYS:
        return len + key.encoded_length();
      default:
        return len;
    }
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)by);
    Serialization::encode_vi64(bufp, cid);
    
    switch(by) {
      case By::RID:
        Serialization::encode_vi64(bufp, rid);
        return;
      case By::INTERVALS:
        interval.encode(bufp);
        return;
      case By::KEYS:
        key.encode(bufp);
        return;
      default:
        return;
    }
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    by = (By)Serialization::decode_i8(bufp, remainp);

    switch(by) {
      case By::RID:
        rid = Serialization::decode_vi64(bufp, remainp);
        return;
      case By::INTERVALS:{
        interval.decode(bufp, remainp);
        return;
      }
      case By::KEYS:
        key.decode(bufp, remainp);
        return;
      default:
        return;
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
                    const DB::Specs::Key& next_key)  
                    : HostEndPoints(endpoints), cid(cid), rid(rid), 
                      next_key(next_key) {
  }

  int64_t         cid; 
  int64_t         rid; 
  DB::Specs::Key  next_key;

  const std::string to_string() {
    std::string s("Range-RS(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" ");
    s.append(HostEndPoints::to_string());
    if(cid == 1) {
      s.append(" Next");
      s.append(next_key.to_string());
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
         + (cid==1?next_key.encoded_length():0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    HostEndPoints::encode_internal(bufp);
    if(cid == 1)
      next_key.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    HostEndPoints::decode_internal(version, bufp, remainp);
    if(cid == 1)
      next_key.decode(bufp, remainp);
  }

};
  

}}}

#endif // swc_db_protocol_params_MngrRangeGetRs_h
