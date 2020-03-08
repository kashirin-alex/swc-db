
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_mngr_params_RgrUpdate_h
#define swc_db_protocol_mngr_params_RgrUpdate_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RgrUpdate : public Serializable {
  public:

    RgrUpdate() {}

    RgrUpdate(Manager::RangerList hosts, bool sync_all) 
              : hosts(hosts), sync_all(sync_all) {}

    std::string to_string() const {
      std::string s("Rangers-params:");
      for(auto& h : hosts) {
        s.append("\n ");
        s.append(h->to_string());
      }
      return s;
    }

    Manager::RangerList hosts;
    bool sync_all;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      size_t len = 1 + Serialization::encoded_length_vi32(hosts.size());
      for(auto& h : hosts)
        len += h->encoded_length();
      return len;
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_bool(bufp, sync_all);
      Serialization::encode_vi32(bufp, hosts.size());
      for(auto& h : hosts)
        h->encode(bufp);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {   
      sync_all = Serialization::decode_bool(bufp, remainp);
      size_t len = Serialization::decode_vi32(bufp, remainp);
      hosts.clear();
      hosts.resize(len);
      for(size_t i =0; i<len; i++)
        (hosts[i] = std::make_shared<Manager::Ranger>())
          ->decode(bufp, remainp);
    }

  };
  

}}}}

#endif // swc_db_protocol_rgr_params_rgrUpdate_h
