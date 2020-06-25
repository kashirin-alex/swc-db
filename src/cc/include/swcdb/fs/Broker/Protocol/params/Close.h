/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Close_h
#define swc_fs_Broker_Protocol_params_Close_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class CloseReq : public Serializable {
  public:
  
  CloseReq(): fd(-1) {}

  CloseReq(int32_t fd) : fd(fd) {}

  int32_t fd;

  private:

  size_t encoded_length_internal() const {
    return 4;
  }

  void encode_internal(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, fd);
  }

  void decode_internal(const uint8_t** bufp, size_t* remainp) {
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  }
  
};

}}}}

#endif // swc_fs_Broker_Protocol_params_Close_h
