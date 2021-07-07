/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_MngrStatus_h
#define swcdb_manager_MngrStatus_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/db/Types/MngrState.h"


namespace SWC { namespace Manager {


class MngrStatus final {
  public:

  typedef std::shared_ptr<MngrStatus> Ptr;

  SWC_CAN_INLINE
  MngrStatus(uint8_t role, cid_t begin, cid_t end,
             const Comm::EndPoints& endpoints,
             Comm::ConnHandlerPtr c, uint32_t pr)
             : priority(pr), state(DB::Types::MngrState::NOTSET), role(role),
               cid_begin(begin), cid_end(end),
               endpoints(endpoints),
               conn(c), failures(0) {
  }

  SWC_CAN_INLINE
  MngrStatus(const uint8_t** bufp, size_t* remainp)
      : priority(Serialization::decode_i32(bufp, remainp)),
        state(DB::Types::MngrState(Serialization::decode_i8(bufp, remainp))),
        role(Serialization::decode_i8(bufp, remainp)),
        cid_begin(Serialization::decode_vi64(bufp, remainp)),
        cid_end(Serialization::decode_vi64(bufp, remainp)),
        endpoints(Serialization::decode_endpoints(bufp, remainp)),
        conn(nullptr), failures(0) {
  }

  //~MngrStatus() { }

  SWC_CAN_INLINE
  bool eq_grouping(const MngrStatus& other) const noexcept {
    return role == other.role &&
           cid_begin == other.cid_begin &&
           cid_end == other.cid_end;
  }

  size_t encoded_length() const noexcept {
    size_t len = 6
      + Serialization::encoded_length_vi64(cid_begin)
      + Serialization::encoded_length_vi64(cid_end)
      + Serialization::encoded_length(endpoints);
    return len;
  }

  void encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, priority.load());
    Serialization::encode_i8(bufp, uint8_t(state.load()));
    Serialization::encode_i8(bufp, role);
    Serialization::encode_vi64(bufp, cid_begin);
    Serialization::encode_vi64(bufp, cid_end);
    Serialization::encode(bufp, endpoints);
  }

  void print(std::ostream& out) const {
    out << "MngrStatus(priority=" << priority
        << " state=" << DB::Types::to_string(state)
        << " role=" << DB::Types::MngrRole::to_string(role)
        << " cid=" << cid_begin << '-' << cid_end;
    Comm::print(out << ' ', endpoints);
    out << ')';
  }


  Core::Atomic<uint32_t>              priority;
  Core::Atomic<DB::Types::MngrState>  state;
  const uint8_t                       role;
  const cid_t                         cid_begin;
  const cid_t                         cid_end;
  const Comm::EndPoints               endpoints;

  Comm::ConnHandlerPtr                conn; // mngr-inchain
  int                                 failures;
};

typedef Core::Vector<MngrStatus::Ptr> MngrsStatus;




}}

#endif // swcdb_manager_MngrStatus_h
