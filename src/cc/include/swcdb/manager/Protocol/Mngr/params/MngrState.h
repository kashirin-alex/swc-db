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
  MngrState() noexcept : states(), token(), mngr_host() { }

  SWC_CAN_INLINE
  MngrState(const Manager::MngrsStatus& a_states,
            uint64_t a_token, const EndPoint& a_mngr_host)
            : states(a_states), token(a_token), mngr_host(a_mngr_host) {
  }

  SWC_CAN_INLINE
  MngrState(MngrState&& other) noexcept
            : states(std::move(other.states)),
              token(other.token),
              mngr_host(std::move(other.mngr_host)) {
  }

  MngrState(const MngrState&) = delete;
  MngrState& operator=(MngrState&&) = delete;
  MngrState& operator=(const MngrState&) = delete;

  ~MngrState() noexcept { }

  Manager::MngrsStatus  states;
  uint64_t              token;
  EndPoint              mngr_host;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}

#endif // swcdb_manager_Protocol_mngr_params_MngrState_h
