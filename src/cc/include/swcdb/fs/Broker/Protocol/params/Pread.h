/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Pread_h
#define swc_fs_Broker_Protocol_params_Pread_h


#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class PreadReq : public Serializable {
  public:

  PreadReq();

  PreadReq(int32_t fd, uint64_t offset, uint32_t amount);

  int32_t   fd;
  uint64_t  offset;
  uint32_t  amount;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/params/Pread.cc"
#endif 


#endif // swc_fs_Broker_Protocol_params_Pread_h
