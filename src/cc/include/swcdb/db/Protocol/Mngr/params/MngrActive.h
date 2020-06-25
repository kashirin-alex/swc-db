
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_protocol_mngr_params_MngrActive_h
#define swc_protocol_mngr_params_MngrActive_h


#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"
#include "swcdb/db/client/mngr/Groups.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class MngrActiveReq : public Serializable {
  public:

  MngrActiveReq(uint8_t role=Types::MngrRole::COLUMNS, cid_t cid=0);

  virtual ~MngrActiveReq();
  
  uint8_t   role; 
  cid_t     cid;

  private:

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t** bufp) const;
    
  void decode_internal(const uint8_t** bufp, size_t* remainp);

};
  


class MngrActiveRsp : public Common::Params::HostEndPoints {
  public:

  MngrActiveRsp();

  MngrActiveRsp(const EndPoints& endpoints);
  
  virtual ~MngrActiveRsp();

  bool available;
  
  private:

  size_t encoded_length_internal() const;

  void encode_internal(uint8_t** bufp) const;

  void decode_internal(const uint8_t** bufp, size_t* remainp);

};
  


}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/MngrActive.cc"
#endif 

#endif // swc_protocol_mngr_params_MngrActive_h
