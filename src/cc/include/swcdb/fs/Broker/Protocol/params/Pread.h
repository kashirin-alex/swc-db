/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Pread_h
#define swc_fs_Broker_Protocol_params_Pread_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class PreadReq : public Serializable {
  public:

  PreadReq() {}

  PreadReq(int32_t fd, uint64_t offset, uint32_t amount)
          : fd(fd), offset(offset), amount(amount) {}

  int32_t   fd;
  uint64_t  offset;
  uint32_t  amount;

  private:

  size_t internal_encoded_length() const {
      return 16;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, fd);
    Serialization::encode_i64(bufp, offset);
    Serialization::encode_i32(bufp, amount);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    offset = Serialization::decode_i64(bufp, remainp);
    amount = Serialization::decode_i32(bufp, remainp);
  }
  
};


}}}}

#endif // swc_fs_Broker_Protocol_params_Pread_h
