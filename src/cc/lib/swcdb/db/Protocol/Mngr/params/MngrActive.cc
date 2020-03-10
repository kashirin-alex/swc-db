
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/db/Protocol/Mngr/params/MngrActive.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


MngrActiveReq::MngrActiveReq(size_t begin, size_t end) 
                            : begin(begin), end(end){
}

MngrActiveReq::~MngrActiveReq() { }

uint8_t MngrActiveReq::encoding_version() const {
  return 1;
}

size_t MngrActiveReq::encoded_length_internal() const {
  return Serialization::encoded_length_vi64(begin)
       + Serialization::encoded_length_vi64(end);
}

void MngrActiveReq::encode_internal(uint8_t **bufp) const {
  Serialization::encode_vi64(bufp, begin);
  Serialization::encode_vi64(bufp, end);
}

void MngrActiveReq::decode_internal(uint8_t version, const uint8_t **bufp, 
                                    size_t *remainp) {
  begin = (size_t)Serialization::decode_vi64(bufp, remainp);
  end = (size_t)Serialization::decode_vi64(bufp, remainp);
}



MngrActiveRsp::MngrActiveRsp() {}

MngrActiveRsp::MngrActiveRsp(const EndPoints& endpoints) 
                            : Common::Params::HostEndPoints(endpoints), 
                              available(endpoints.size()>0) { 
}

MngrActiveRsp::~MngrActiveRsp() { }

size_t MngrActiveRsp::encoded_length_internal() const {
  size_t len = 1;
  if(available)
    len += Common::Params::HostEndPoints::encoded_length_internal();
  return len;
}

void MngrActiveRsp::encode_internal(uint8_t **bufp) const {
  Serialization::encode_bool(bufp, available);
  if(available) {
    Common::Params::HostEndPoints::encode_internal(bufp);
  }
}

void MngrActiveRsp::decode_internal(uint8_t version, const uint8_t **bufp,
                                    size_t *remainp) {
  available = Serialization::decode_bool(bufp, remainp);
  if(available) {
    Common::Params::HostEndPoints::decode_internal(version, bufp, remainp);
  }
}



}}}}
