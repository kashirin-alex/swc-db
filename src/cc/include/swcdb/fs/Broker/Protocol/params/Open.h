/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Open_h
#define swc_fs_Broker_Protocol_params_Open_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class OpenReq : public Serializable {
  public:

  OpenReq() {}

  OpenReq(const std::string& fname, uint32_t flags, int32_t bufsz)
          : fname(fname), flags(flags), bufsz(bufsz) {}

  std::string fname;
  uint32_t    flags;
  int32_t     bufsz;

  private:

  size_t internal_encoded_length() const {
    return 8 + Serialization::encoded_length_vstr(fname);
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, flags);
    Serialization::encode_i32(bufp, bufsz);
    Serialization::encode_vstr(bufp, fname);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    flags = Serialization::decode_i32(bufp, remainp);
    bufsz = (int32_t)Serialization::decode_i32(bufp, remainp);
    fname.clear();
    fname.append(Serialization::decode_vstr(bufp, remainp));
  }

};




class OpenRsp : public Serializable {
  public:
  
  OpenRsp() {}

  OpenRsp(int32_t fd) : fd(fd) {}

  int32_t fd;

  private:

  size_t internal_encoded_length() const {
    return 4;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, fd);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    fd = (int32_t)Serialization::decode_i32(bufp, remainp);
  }
  
};

}}}}

#endif // swc_fs_Broker_Protocol_params_Open_h
