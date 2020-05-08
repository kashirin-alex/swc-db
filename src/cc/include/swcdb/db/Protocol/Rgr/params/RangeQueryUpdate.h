
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_rgr_params_RangeQueryUpdate_h
#define swc_db_protocol_rgr_params_RangeQueryUpdate_h

#include "swcdb/core/Error.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeQueryUpdateReq : public Serializable {
  public:

  RangeQueryUpdateReq();

  RangeQueryUpdateReq(int64_t cid, int64_t rid);

  virtual ~RangeQueryUpdateReq();

  int64_t           cid;
  int64_t           rid;
  
  std::string to_string() const;

  private:

  uint8_t encoding_version() const;

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};



class RangeQueryUpdateRsp  : public Serializable {
  public:

  RangeQueryUpdateRsp(int err = Error::OK);

  RangeQueryUpdateRsp(int err, const DB::Cell::Key& range_prev_end,
                               const DB::Cell::Key& range_end);

  virtual ~RangeQueryUpdateRsp();

  int32_t       err;
  DB::Cell::Key range_prev_end;
  DB::Cell::Key range_end;

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
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.cc"
#endif 

#endif // swc_db_protocol_rgr_params_RangeQueryUpdate_h
