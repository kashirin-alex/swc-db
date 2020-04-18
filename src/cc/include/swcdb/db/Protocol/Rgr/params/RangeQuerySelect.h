
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_rgr_params_RangeQuerySelect_h
#define swc_db_protocol_rgr_params_RangeQuerySelect_h

#include "swcdb/core/StaticBuffer.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


class RangeQuerySelectReq : public Serializable {
  public:

  RangeQuerySelectReq();

  RangeQuerySelectReq(int64_t cid, int64_t rid, 
                      const DB::Specs::Interval& interval);

  virtual ~RangeQuerySelectReq();

  int64_t              cid;
  int64_t              rid;
  DB::Specs::Interval  interval;
  
  std::string to_string() const;

  private:

  uint8_t encoding_version() const;

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};



class RangeQuerySelectRsp  : public Serializable {
  public:

  RangeQuerySelectRsp(int err=Error::OK, bool reached_limit=false, 
                      uint64_t offset=0);
  
  RangeQuerySelectRsp(StaticBuffer& data);

  virtual ~RangeQuerySelectRsp();

  int32_t         err;
  bool            reached_limit;
  uint64_t        offset;
  StaticBuffer    data;
  
  std::string to_string() const;

  private:

  uint8_t encoding_version() const;
    
  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};
  

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.cc"
#endif 

#endif // swc_db_protocol_rgr_params_RangeQuerySelect_h
