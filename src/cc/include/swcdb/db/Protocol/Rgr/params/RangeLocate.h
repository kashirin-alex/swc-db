
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_RangeLocate_h
#define swc_db_protocol_rgr_params_RangeLocate_h


#include "swcdb/core/Error.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeLocateReq : public Serializable {
  public:

  static const uint8_t NEXT_RANGE  = 0x01;
  static const uint8_t COMMIT      = 0x02;

  RangeLocateReq(int64_t cid=0, int64_t rid=0);
  
  virtual ~RangeLocateReq();

  int64_t        cid;
  int64_t        rid;
  DB::Cell::Key  range_begin, range_end, range_offset;
  uint8_t        flags;
  
  const std::string to_string() const;

  private:

  uint8_t encoding_version() const;

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};



class RangeLocateRsp  : public Serializable {
  public:

  RangeLocateRsp(int err=Error::OK);

  virtual ~RangeLocateRsp();

  int             err;         
  int64_t         cid; 
  int64_t         rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

  const std::string to_string() const;

  private:

  uint8_t encoding_version() const;
    
  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};
  

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.cc"
#endif 

#endif // swc_db_protocol_rgr_params_RangeLocate_h
