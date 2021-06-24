/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_mngr_params_MngrState_h
#define swcdb_manager_Protocol_mngr_params_MngrState_h

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class MngrState final : public Serializable {
  public:

  SWC_CAN_INLINE
  MngrState() noexcept { }

  SWC_CAN_INLINE
  MngrState(const Manager::MngrsStatus& states,
            uint64_t token, const EndPoint& mngr_host)
            : states(states), token(token), mngr_host(mngr_host) {
  }

  Manager::MngrsStatus  states;
  uint64_t              token;
  EndPoint              mngr_host;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}

#endif // swcdb_manager_Protocol_mngr_params_MngrState_h
