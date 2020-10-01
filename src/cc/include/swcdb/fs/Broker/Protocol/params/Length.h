/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Length_h
#define swcdb_fs_Broker_Protocol_params_Length_h


#include "swcdb/core/Serializable.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Params {


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


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Length.cc"
#endif 


#endif // swcdb_fs_Broker_Protocol_params_Length_h
