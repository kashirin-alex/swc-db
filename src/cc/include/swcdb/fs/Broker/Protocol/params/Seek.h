/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Seek_h
#define swc_fs_Broker_Protocol_params_Seek_h


#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class SeekReq : public Serializable {
  public:

  SeekReq();

  SeekReq(int32_t fd, size_t offset);

  int32_t   fd;
  uint64_t  offset;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
};




class SeekRsp : public Serializable {
  public:

  SeekRsp();

  SeekRsp(size_t offset);
  
  uint64_t offset;
  
  private:
  
  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
};

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/params/Seek.cc"
#endif 


#endif // swc_fs_Broker_Protocol_params_Seek_h
