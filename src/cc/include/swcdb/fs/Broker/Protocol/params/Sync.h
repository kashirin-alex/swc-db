/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */
 
#ifndef swc_fs_Broker_Protocol_params_Sync_h
#define swc_fs_Broker_Protocol_params_Sync_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {

class SyncReq : public Serializable {
  public:
  
  SyncReq() {}

  SyncReq(int32_t fd) : fd(fd) {}

  int32_t fd;

  private:

  size_t internal_encoded_length() const {
    return 4;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, fd);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    fd = Serialization::decode_i32(bufp, remainp);
  }
  
};

}}}}

#endif // swc_fs_Broker_Protocol_params_Sync_h
