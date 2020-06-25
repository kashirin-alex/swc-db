
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_manager_Protocol_mngr_params_MngrState_h
#define swc_manager_Protocol_mngr_params_MngrState_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class MngrState : public Serializable {
  public:

  MngrState();

  MngrState(Manager::MngrsStatus states, 
            uint64_t token, const EndPoint& mngr_host);

  Manager::MngrsStatus states;
  uint64_t token;
  EndPoint mngr_host;

  private:

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t** bufp) const;
    
  void decode_internal(const uint8_t** bufp, size_t* remainp);

};
  

}}}}

#endif // swc_manager_Protocol_mngr_params_MngrState_h
