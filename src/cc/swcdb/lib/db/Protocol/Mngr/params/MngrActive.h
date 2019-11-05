
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_MngrActive_h
#define swc_db_protocol_mngr_params_MngrActive_h

#include "../../Common/params/HostEndPoints.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class MngrActiveReq : public Serializable {
  public:

    MngrActiveReq(size_t begin=0, size_t end=0) 
                  : begin(begin), end(end){
    }

    size_t begin; 
    size_t end;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      return Serialization::encoded_length_vi64(begin)
           + Serialization::encoded_length_vi64(end);
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_vi64(bufp, begin);
      Serialization::encode_vi64(bufp, end);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                         size_t *remainp) {
      begin = (size_t)Serialization::decode_vi64(bufp, remainp);
      end = (size_t)Serialization::decode_vi64(bufp, remainp);
    }

};
  


class MngrActiveRsp : public Common::Params::HostEndPoints {
  public:

    MngrActiveRsp() {}

    MngrActiveRsp(const EndPoints& endpoints) 
                 : Common::Params::HostEndPoints(endpoints), 
                  available(endpoints.size()>0) { }
    bool available;
  
  private:

    size_t encoded_length_internal() const {
      size_t len = 1;
      if(available)
        len += Common::Params::HostEndPoints::encoded_length_internal();
      return len;
    }

    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_bool(bufp, available);
      if(available) {
        Common::Params::HostEndPoints::encode_internal(bufp);
      }
    }

    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      available = Serialization::decode_bool(bufp, remainp);
      if(available) {
        Common::Params::HostEndPoints::decode_internal(version, bufp, remainp);
      }
    }

};
  


}}}}

#endif // swc_db_protocol_params_MngrActive_h
