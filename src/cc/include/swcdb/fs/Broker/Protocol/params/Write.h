/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Write_h
#define swc_fs_Broker_Protocol_params_Write_h


#include "swcdb/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class WriteReq : public Serializable {
  public:

  WriteReq();

  WriteReq(const std::string& fname, uint32_t flags,
            uint8_t replication, int64_t blksz);
  
  std::string fname;
  uint32_t    flags;
  uint8_t     replication;
  int64_t     blksz;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);

};

}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Write.cc"
#endif 


#endif // swc_fs_Broker_Protocol_params_Write_h
