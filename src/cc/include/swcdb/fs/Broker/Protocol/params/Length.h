/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Length_h
#define swc_fs_Broker_Protocol_params_Length_h


#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class LengthReq : public Serializable {
  public:

  LengthReq();

  LengthReq(const std::string& fname);

  std::string fname;

  private:
  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);

};




class LengthRsp : public Serializable {
  public:
  
  LengthRsp();

  LengthRsp(size_t length);

  size_t length;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
};

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/params/Length.cc"
#endif 


#endif // swc_fs_Broker_Protocol_params_Length_h
