
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_RgrGet_h
#define swc_db_protocol_mngr_params_RgrGet_h

#include "swcdb/lib/core/Serializable.h"
#include "../../Common/params/HostEndPoints.h"

#include "swcdb/lib/db/Cells/Cell.h"
#include "swcdb/lib/db/Cells/SpecsInterval.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RgrGetReq : public Serializable {
  public:

  RgrGetReq(int64_t cid, DB::Specs::Interval::Ptr interval)
            : cid(cid), rid(0), interval(interval) {}

  RgrGetReq(int64_t cid=0, int64_t rid=0) : cid(cid), rid(rid) {}

  virtual ~RgrGetReq(){ }

  void free(){
    cid = 0;
    rid = 0;
    interval->free();
  }
  
  int64_t                   cid;
  int64_t                   rid;
  DB::Specs::Interval::Ptr  interval;
  
  const std::string to_string() {
    std::string s("Ranger(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));
    if(rid == 0) {
      s.append(" ");
      s.append(interval->to_string());
    }
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return Serialization::encoded_length_vi64(cid)
      + Serialization::encoded_length_vi64(rid)
      + (rid == 0 ? interval->encoded_length() : 0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    if(rid == 0) 
      interval->encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    if(rid == 0)
      interval = DB::Specs::Interval::make_ptr(bufp, remainp);
  }

};



class RgrGetRsp : public Common::Params::HostEndPoints {
  public:

  RgrGetRsp(): err(0), cid(0), rid(0) {}

  RgrGetRsp(int64_t cid, int64_t rid, const EndPoints& endpoints) 
            : Common::Params::HostEndPoints(endpoints), 
              err(0), cid(cid), rid(rid) { }

  RgrGetRsp(int64_t cid, int64_t rid, const EndPoints& endpoints, 
                const DB::Specs::Key& next_key)  
              : Common::Params::HostEndPoints(endpoints), 
                err(0), cid(cid), rid(rid), next_key(next_key) { }

  int             err;         
  int64_t         cid; 
  int64_t         rid; 
  DB::Specs::Key  next_key;

  const std::string to_string() {
    std::string s("Ranger(");
    s.append("err=");
    s.append(std::to_string(err));
    if(err == Error::OK) {
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));
      s.append(" ");
      s.append(Common::Params::HostEndPoints::to_string());
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
              + Common::Params::HostEndPoints::encoded_length_internal()
              + (cid==1?next_key.encoded_length():0))
            );
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    if(err == Error::OK) {
      Serialization::encode_vi64(bufp, cid);
      Serialization::encode_vi64(bufp, rid);
      Common::Params::HostEndPoints::encode_internal(bufp);
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
      Common::Params::HostEndPoints::decode_internal(version, bufp, remainp);
      if(cid == 1)
        next_key.decode(bufp, remainp, true);
    }
  }

};
  

}}}}

#endif // swc_db_protocol_mngr_params_RgrGet_h
