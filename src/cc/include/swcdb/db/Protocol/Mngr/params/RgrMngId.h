
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_RgrMngId_h
#define swc_db_protocol_mngr_params_RgrMngId_h

#include "swcdb/core/Serializable.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RgrMngId  : public Common::Params::HostEndPoints {
  public:

    enum Flag {
      MNGR_ASSIGNED = 1,
      MNGR_NOT_ACTIVE = 2,
      MNGR_REASSIGN = 3,
      MNGR_REREQ = 4,
      RS_REQ = 5, // >= params with host endpoints
      RS_ACK = 6,
      RS_DISAGREE = 7,
      RS_SHUTTINGDOWN = 8
    };

    RgrMngId() {}

    RgrMngId(uint64_t id, Flag flag) 
            : id(id), flag(flag){
    }
    RgrMngId(uint64_t id, Flag flag, const EndPoints& endpoints) 
            : id(id), flag(flag),
              Common::Params::HostEndPoints(endpoints){     
    }

    uint64_t        id; 
    Flag            flag;
    Types::Fs       fs;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      size_t len = 1;
      if(flag != Flag::MNGR_NOT_ACTIVE)
        len += Serialization::encoded_length_vi64(id);

      if(flag >= Flag::RS_REQ) 
        len +=  Common::Params::HostEndPoints::encoded_length_internal();
      
      if(flag == Flag::MNGR_ASSIGNED)
        len++; // fs-type
      return len;
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_i8(bufp, (uint8_t)flag);
      if(flag != Flag::MNGR_NOT_ACTIVE)
        Serialization::encode_vi64(bufp, id);
      
      if(flag >= Flag::RS_REQ)
        Common::Params::HostEndPoints::encode_internal(bufp);

      if(flag == Flag::MNGR_ASSIGNED)
        Serialization::encode_i8(
          bufp, (int8_t)Env::FsInterface::interface()->get_type());
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      flag = (Flag)Serialization::decode_i8(bufp, remainp);
      if(flag != Flag::MNGR_NOT_ACTIVE)
        id = Serialization::decode_vi64(bufp, remainp);
      
      if(flag >= Flag::RS_REQ)
        Common::Params::HostEndPoints::decode_internal(version, bufp, remainp);
      
      if(flag == Flag::MNGR_ASSIGNED)
        fs = (Types::Fs)Serialization::decode_i8(bufp, remainp);
    }

  };
  

}}}}

#endif // swc_db_protocol_params_RgrMngId_h
