
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_protocol_mngr_params_MngrActive_h
#define swc_protocol_mngr_params_MngrActive_h


#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class MngrActiveReq : public Serializable {
  public:

  MngrActiveReq(size_t begin=0, size_t end=0);

  virtual ~MngrActiveReq();
  
  size_t begin; 
  size_t end;

  private:

  uint8_t encoding_version() const;
    
  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};
  


class MngrActiveRsp : public Common::Params::HostEndPoints {
  public:

  MngrActiveRsp();

  MngrActiveRsp(const EndPoints& endpoints);
  
  virtual ~MngrActiveRsp();

  bool available;
  
  private:

  size_t encoded_length_internal() const;

  void encode_internal(uint8_t **bufp) const;

  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};
  


}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/MngrActive.cc"
#endif 

#endif // swc_protocol_mngr_params_MngrActive_h
