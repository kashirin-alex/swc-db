
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_params_RgrUpdate_h
#define swcdb_manager_Protocol_mngr_params_RgrUpdate_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RgrUpdate : public Serializable {
  public:

    RgrUpdate() {}

    RgrUpdate(const Manager::RangerList& hosts, bool sync_all) 
              : hosts(hosts), sync_all(sync_all) {}

    void print(std::ostream& out) const {
      out << "Rangers-params:";
      for(auto& h : hosts)
        h->print(out << "\n ");
    }

    Manager::RangerList hosts;
    bool sync_all;

  private:

    size_t internal_encoded_length() const {
      size_t len = 1 + Serialization::encoded_length_vi32(hosts.size());
      for(auto& h : hosts)
        len += h->encoded_length();
      return len;
    }
    
    void internal_encode(uint8_t** bufp) const {
      Serialization::encode_bool(bufp, sync_all);
      Serialization::encode_vi32(bufp, hosts.size());
      for(auto& h : hosts)
        h->encode(bufp);
    }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
      sync_all = Serialization::decode_bool(bufp, remainp);
      size_t len = Serialization::decode_vi32(bufp, remainp);
      hosts.clear();
      hosts.resize(len);
      for(size_t i =0; i<len; ++i)
        (hosts[i] = std::make_shared<Manager::Ranger>())
          ->decode(bufp, remainp);
    }

  };
  

}}}}

#endif // swcdb_manager_Protocol_mngr_params_RgrUpdate_h
