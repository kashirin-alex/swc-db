/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_fs_Broker_Protocol_params_Rename_h
#define swc_fs_Broker_Protocol_params_Rename_h



namespace SWC { namespace FS { namespace Protocol { namespace Params {


class RenameReq : public Serializable {
  public:

  RenameReq() {}

  RenameReq(const std::string& from, const std::string& to) 
            : from(from), to(to) {}

  std::string from;
  std::string to;

  private:

  size_t encoded_length_internal() const {
  return Serialization::encoded_length_vstr(from)
       + Serialization::encoded_length_vstr(to);
  }

  void encode_internal(uint8_t** bufp) const {
    Serialization::encode_vstr(bufp, from);
    Serialization::encode_vstr(bufp, to);
  }

  void decode_internal(const uint8_t** bufp, size_t* remainp) {
    from.clear();
    from.append(Serialization::decode_vstr(bufp, remainp));
    to.clear();
    to.append(Serialization::decode_vstr(bufp, remainp));
  }

};

}}}}

#endif // swc_fs_Broker_Protocol_params_Rename_h
