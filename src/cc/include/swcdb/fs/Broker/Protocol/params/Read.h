/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Read_h
#define swc_fs_Broker_Protocol_params_Read_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class ReadReq : public Serializable {
  public:

  ReadReq() {}

  ReadReq(int32_t fd, uint32_t amount)
          : fd(fd), amount(amount) {}

  int32_t   fd {};
  uint32_t  amount;

  private:

  size_t internal_encoded_length() const {
      return 8;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, fd);
    Serialization::encode_i32(bufp, amount);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    amount = Serialization::decode_i32(bufp, remainp);
  }
  
};



class ReadRsp : public Serializable {
  public:

  ReadRsp() {}

  ReadRsp(uint64_t offset) 
          : offset(offset) {}

  uint64_t offset;

  private:

  size_t internal_encoded_length() const {
      return 8;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i64(bufp, offset);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    offset = Serialization::decode_i64(bufp, remainp);
  }
  

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Read_h
