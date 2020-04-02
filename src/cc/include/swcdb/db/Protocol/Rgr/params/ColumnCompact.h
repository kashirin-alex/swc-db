
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_rgr_params_ColumnCompact_h
#define swc_db_protocol_rgr_params_ColumnCompact_h

#include "swcdb/core/Serializable.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


class ColumnCompactReq : public Serializable {
  public:

  ColumnCompactReq(int64_t cid=0);

  virtual ~ColumnCompactReq();
  
  int64_t cid;
  
  std::string to_string() const;

  private:

  uint8_t encoding_version() const;

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};



class ColumnCompactRsp : public Serializable {
  public:

  ColumnCompactRsp();

  virtual ~ColumnCompactRsp();

  int err;        

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
#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.cc"
#endif 

#endif // swc_db_protocol_rgr_params_ColumnCompact_h
