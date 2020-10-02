/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Read_h
#define swcdb_fs_Broker_Protocol_params_Read_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class ReadReq : public Serializable {
  public:

  ReadReq();

  ReadReq(int32_t fd, uint32_t amount);

  int32_t   fd;
  uint32_t  amount;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
};



class ReadRsp : public Serializable {
  public:

  ReadRsp();

  ReadRsp(uint64_t offset);

  uint64_t offset;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  

};

}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Read.cc"
#endif 


#endif // swcdb_fs_Broker_Protocol_params_Read_h
