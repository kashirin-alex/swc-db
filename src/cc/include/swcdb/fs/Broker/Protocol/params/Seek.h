/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Seek_h
#define swc_fs_Broker_Protocol_params_Seek_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class SeekReq : public Serializable {
  public:

  SeekReq() {}

  SeekReq(int32_t fd, size_t offset)
          : fd(fd), offset(offset) {}

  int32_t   fd;
  uint64_t  offset;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 12;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, fd);
    Serialization::encode_i64(bufp, offset);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
    offset = Serialization::decode_i64(bufp, remainp);
  }
  
};




class SeekRsp : public Serializable {
  public:

  SeekRsp() {}

  SeekRsp(size_t offset) : offset(offset) {}
  
  uint64_t offset;
  
  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
      return 8;
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i64(bufp, offset);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    offset = Serialization::decode_i64(bufp, remainp);
  }
  
};

}}}}

#endif // swc_fs_Broker_Protocol_params_Seek_h
