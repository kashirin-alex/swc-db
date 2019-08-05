
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_params_MngrUpdateRangeServers_h
#define swc_db_protocol_params_MngrUpdateRangeServers_h

#include "swcdb/lib/core/Serializable.h"

namespace SWC {
namespace Protocol {
namespace Params {

  class MngrUpdateRangeServers : public Serializable {
  public:

    MngrUpdateRangeServers() {}

    MngrUpdateRangeServers(server::Mngr::RsStatusList hosts, 
                           bool sync_all) : hosts(hosts), sync_all(sync_all) {}

    const std::string to_string() {
      std::string s("RangeServers-params:");
      for(auto h : hosts) {
        s.append("\n ");
        s.append(h->to_string());
      }
      return s;
    }

    server::Mngr::RsStatusList hosts;
    bool sync_all;

  private:

    uint8_t encoding_version() const {
      return 1;
    }
    
    size_t encoded_length_internal() const {
      size_t len = 5; // (sync_all+num_hosts)
      for(auto h : hosts)
        len += h->encoded_length();
      return len;
    }
    
    void encode_internal(uint8_t **bufp) const {
      Serialization::encode_bool(bufp, sync_all);
      Serialization::encode_i32(bufp, hosts.size());
      for(auto h : hosts)
        h->encode(bufp);
    }
    
    void decode_internal(uint8_t version, const uint8_t **bufp, 
                        size_t *remainp) {
      sync_all = Serialization::decode_bool(bufp, remainp);
      size_t len = Serialization::decode_i32(bufp, remainp);
      server::Mngr::RsStatusPtr h;
      for(size_t i =0; i<len; i++){
        h = std::make_shared<server::Mngr::RsStatus>();
        h->decode(bufp, remainp);
        hosts.push_back(h);
      }
    }

  };
  

}}}

#endif // swc_db_protocol_params_ReqIsMngrActive_h
