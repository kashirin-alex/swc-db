
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

  static const uint8_t NEXT_RANGE  = 0x01;
  static const uint8_t COMMIT      = 0x02;

  RangeLocateReq(int64_t cid=0, int64_t rid=0)
                : cid(cid), rid(rid), flags(0) {}

  virtual ~RangeLocateReq(){ }

  int64_t        cid;
  int64_t        rid;
  DB::Cell::Key  range_begin, range_end, range_offset;
  uint8_t        flags;
  
  const std::string to_string() {
    std::string s("RangeLocateReq(");
    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" rid=");
    s.append(std::to_string(rid));

    s.append(" flags=");
    s.append(std::to_string((int)flags));
    s.append(" RangeBegin");
    s.append(range_begin.to_string());
    s.append(" RangeEnd");
    s.append(range_end.to_string());
    if(flags & NEXT_RANGE) {
      s.append(" RangeOffset");
      s.append(range_offset.to_string());
    }
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
          + range_begin.encoded_length() 
          + range_end.encoded_length()
          + 1
          + (flags & NEXT_RANGE ? range_offset.encoded_length() : 0);
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, rid);
    range_begin.encode(bufp);
    range_end.encode(bufp);
    Serialization::encode_i8(bufp, flags);
    if(flags & NEXT_RANGE)
      range_offset.encode(bufp);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    rid = Serialization::decode_vi64(bufp, remainp);
    range_begin.decode(bufp, remainp);
    range_end.decode(bufp, remainp);
    flags = Serialization::decode_i8(bufp, remainp);
    if(flags & NEXT_RANGE)
      range_offset.decode(bufp, remainp);
  }

};



class RangeLocateRsp  : public Serializable {
  public:

  RangeLocateRsp(int err=Error::OK) : err(err), cid(0), rid(0) { }

  virtual ~RangeLocateRsp() { }

  int             err;         
  int64_t         cid; 
  int64_t         rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

  const std::string to_string() const {
    std::string s("Range(");
    s.append("err=");
    s.append(std::to_string(err));
    if(err) {
      s.append("(");
      s.append(Error::get_text(err));
      s.append(")");
    } else {
      s.append(" cid=");
      s.append(std::to_string(cid));
      s.append(" rid=");
      s.append(std::to_string(rid));

      s.append(" RangeBegin");
      s.append(range_begin.to_string());
      s.append(" RangeEnd");
      s.append(range_end.to_string());
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
        + range_begin.encoded_length()
        ) );
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    if(!err) {
      Serialization::encode_vi64(bufp, cid);
      Serialization::encode_vi64(bufp, rid);
      range_end.encode(bufp);
      range_begin.encode(bufp);
    }
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    if(!(err = Serialization::decode_vi32(bufp, remainp))) {
      cid = Serialization::decode_vi64(bufp, remainp);
      rid = Serialization::decode_vi64(bufp, remainp);
      range_end.decode(bufp, remainp, true);
      range_begin.decode(bufp, remainp, true);
    }
  }

};
  

}}}}

#endif // swc_db_protocol_rgr_params_RangeLocate_h
