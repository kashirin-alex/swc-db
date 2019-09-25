
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrRsGet_h
#define swc_db_protocol_params_MngrRsGet_h

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

//req-mngr-master, n(cid)+Cells::Interval  => cid(1) + rid + rs(endpoints) + ?next
//            ->  req-rs. 1(cid)+rid+n(cid):Cells::Interval => 2(cid) + rid + ?next
//req-mngr-meta,   2(cid)+rid              => cid(2) + rid + rs(endpoints)
//            ->  req-rs. 2(cid)+rid+n(cid):Cells::Interval => n(cid) + rid + ?next
//req-mngr,        n(cid)+rid              => cid(n) + rid + rs(endpoints)
//            ->  req-rs. n(cid)+rid+Specs::Interval => results

class MngrRsGetReq : public Serializable {
  public:

  enum By {
    INTERVAL,
    RID
  };

  MngrRsGetReq(int64_t cid = 0): cid(cid), rid(-1) {}

  MngrRsGetReq(int64_t cid, DB::Specs::Interval::Ptr interval)
                   : cid(cid), rid(0), interval(interval), by(By::INTERVAL) {}

  MngrRsGetReq(int64_t cid, int64_t rid)
                   : cid(cid), rid(rid), by(By::RID) {}

  virtual ~MngrRsGetReq(){ }

  void free(){
    by = By::RID;
    cid = 0;
    rid = 0;
    interval->free();
  }
  
  By                        by;
  int64_t                   cid;
  int64_t                   rid;
  DB::Specs::Interval::Ptr  interval;
  
  const std::string to_string() {
    std::string s("Range-RS(");
    s.append("by=");
    s.append(std::to_string((uint8_t)by));
    s.append(" cid=");
    s.append(std::to_string(cid));
    switch(by) {
      case By::RID:
        s.append(" rid=");
        s.append(std::to_string(rid));
        break;
      case By::INTERVAL:
        s.append(" interval=");
        s.append(interval->to_string());
        break;
      default:
        break;
    }
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    size_t len = 1+Serialization::encoded_length_vi64(cid);
    switch(by) {
      case By::RID:
        return len + Serialization::encoded_length_vi64(rid);
      case By::INTERVAL:
        return len + interval->encoded_length();
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
      case By::INTERVAL:
        interval->encode(bufp);
        return;
      default:
        return;
    }
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    by = (By)Serialization::decode_i8(bufp, remainp);
    cid = Serialization::decode_vi64(bufp, remainp);

    switch(by) {
      case By::RID:
        rid = Serialization::decode_vi64(bufp, remainp);
        return;
      case By::INTERVAL:
        interval = DB::Specs::Interval::make_ptr(bufp, remainp);
        return;
      default:
        return;
    }
  }

};



class MngrRsGetRsp  : public HostEndPoints {
  public:

  MngrRsGetRsp(): err(0), cid(0), rid(0) {}

  MngrRsGetRsp(int64_t cid, int64_t rid, const EndPoints& endpoints) 
                    :  HostEndPoints(endpoints), err(0), cid(cid), rid(rid) {
  }

  MngrRsGetRsp(int64_t cid, int64_t rid, const EndPoints& endpoints, 
                    const DB::Specs::Key& next_key)  
                    : HostEndPoints(endpoints), err(0), cid(cid), rid(rid), 
                      next_key(next_key) {
  }

  int             err;         
  int64_t         cid; 
  int64_t         rid; 
  DB::Specs::Key  next_key;

  const std::string to_string() {
    std::string s("Range-RS(");
    s.append("err=");
    s.append(std::to_string(err));
    if(err == Error::OK) {
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));
      s.append(" ");
      s.append(HostEndPoints::to_string());
      if(cid == 1) {
        s.append(" Next");
        s.append(next_key.to_string());
      }
    } else {
      s.append("(");
      s.append(Error::get_text(err));
      s.append(")");
    }
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    return  Serialization::encoded_length_vi32(err) 
          + (err != Error::OK ? 0 :
              (Serialization::encoded_length_vi64(cid)
              + Serialization::encoded_length_vi64(rid)
              + HostEndPoints::encoded_length_internal()
              + (cid==1?next_key.encoded_length():0))
            );
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    if(err == Error::OK) {
      Serialization::encode_vi64(bufp, cid);
      Serialization::encode_vi64(bufp, rid);
      HostEndPoints::encode_internal(bufp);
      if(cid == 1)
        next_key.encode(bufp);
    }
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    err = Serialization::decode_vi32(bufp, remainp);
    if(err == Error::OK) {
      cid = Serialization::decode_vi64(bufp, remainp);
      rid = Serialization::decode_vi64(bufp, remainp);
      HostEndPoints::decode_internal(version, bufp, remainp);
      if(cid == 1)
        next_key.decode(bufp, remainp, true);
    }
  }

};
  

}}}

#endif // swc_db_protocol_params_MngrRsGet_h
