
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_rgr_params_ColumnCompact_h
#define swc_db_protocol_rgr_params_ColumnCompact_h

#include "swcdb/core/Serializable.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {


class ColumnCompactReq : public Serializable {
  public:

  ColumnCompactReq(cid_t cid=0);

  virtual ~ColumnCompactReq();
  
  cid_t cid;
  
  std::string to_string() const;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class ColumnCompactRsp : public Serializable {
  public:

  ColumnCompactRsp(int err = Error::OK);

  virtual ~ColumnCompactRsp();

  int err;        

  std::string to_string() const;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/ColumnCompact.cc"
#endif 

#endif // swc_db_protocol_rgr_params_ColumnCompact_h
