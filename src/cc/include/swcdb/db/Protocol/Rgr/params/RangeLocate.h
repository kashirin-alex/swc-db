
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_RangeLocate_h
#define swc_db_protocol_rgr_params_RangeLocate_h

#include "swcdb/core/Serializable.h"

#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeLocateReq : public Serializable {
  public:

  RangeLocateReq() {}

  RangeLocateReq(int64_t cid, int64_t rid)
                : cid(cid), rid(rid) {}

  RangeLocateReq(int64_t cid, int64_t rid,
                 const DB::Cell::Key& range_begin, 
                 const DB::Cell::Key& range_end)
                : cid(cid), rid(rid), 
                  range_begin(range_begin), range_end(range_end) {
  }

  virtual ~RangeLocateReq(){ }

  int64_t        cid;
  int64_t        rid;
  DB::Cell::Key  range_begin, range_end;
  
  const std::string to_string() {
    std::string s("RangeLocateReq(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));

    s.append(" RangeBegin");
    s.append(range_begin.to_string());
    s.append(" RangeEnd");
    s.append(range_end.to_string());
    s.append(")");
    return s;
  }

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return  Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(rid)
          + range_begin.encoded_length() + range_end.encoded_length();
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    range_begin.encode(bufp);
    range_end.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    range_begin.decode(bufp, remainp);
    range_end.decode(bufp, remainp);
  }

};



class RangeLocateRsp  : public Serializable {
  public:

  RangeLocateRsp() : err(Error::OK), cid(0), rid(0) { }

  virtual ~RangeLocateRsp() { }

  int             err;         
  int64_t         cid; 
  int64_t         rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   next_range_begin;

  const std::string to_string() const {
    std::string s("Range(");
    s.append("err=");
    s.append(std::to_string(err));
    if(!err) {
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));

      s.append(" RangeEnd");
      s.append(range_end.to_string());
      s.append(" next_range_begin=");
      s.append(next_range_begin.to_string());
      
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
    return Serialization::encoded_length_vi32(err) 
      + (err ? 0 : (
          Serialization::encoded_length_vi64(cid)
        + Serialization::encoded_length_vi64(rid)
        + range_end.encoded_length()
        + next_range_begin.encoded_length()
        ) );
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    if(!err) {
      Serialization::encode_vi64(bufp, cid);
      Serialization::encode_vi64(bufp, rid);
      range_end.encode(bufp);
      next_range_begin.encode(bufp);
    }
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    err = Serialization::decode_vi32(bufp, remainp);
    if(!err) {
      cid = Serialization::decode_vi64(bufp, remainp);
      rid = Serialization::decode_vi64(bufp, remainp);
      range_end.decode(bufp, remainp, true);
      next_range_begin.decode(bufp, remainp, true);
    }
  }

};
  

}}}}

#endif // swc_db_protocol_rgr_params_RangeLocate_h
