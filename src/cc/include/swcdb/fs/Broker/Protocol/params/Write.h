/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Write_h
#define swc_fs_Broker_Protocol_params_Write_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class WriteReq : public Serializable {
  public:

  WriteReq() {}

  WriteReq(const std::string& fname, uint32_t flags,
	          uint8_t replication, int64_t blksz)
            : fname(fname), flags(flags),
	            replication(replication), blksz(blksz) {
  }
  
  std::string fname;
  uint32_t    flags;
  uint8_t     replication;
  int64_t     blksz;

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return 13 + Serialization::encoded_length_vstr(fname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, flags);
    Serialization::encode_i8(bufp, replication);
    Serialization::encode_i64(bufp, blksz);
    Serialization::encode_vstr(bufp, fname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    flags = Serialization::decode_i32(bufp, remainp);
    replication = (uint8_t)Serialization::decode_i8(bufp, remainp);
    blksz = (int64_t)Serialization::decode_i64(bufp, remainp);
    fname.clear();
    fname.append(Serialization::decode_vstr(bufp, remainp));
  }
};

}}}}

#endif // swc_fs_Broker_Protocol_params_Write_h
