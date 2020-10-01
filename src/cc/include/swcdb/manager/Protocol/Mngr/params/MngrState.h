
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_params_MngrState_h
#define swcdb_manager_Protocol_mngr_params_MngrState_h

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class MngrState : public Comm::Serializable {
  public:

  MngrState();

  MngrState(Manager::MngrsStatus states, 
            uint64_t token, const Comm::EndPoint& mngr_host);

  Manager::MngrsStatus  states;
  uint64_t              token;
  Comm::EndPoint        mngr_host;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}

#endif // swcdb_manager_Protocol_mngr_params_MngrState_h
