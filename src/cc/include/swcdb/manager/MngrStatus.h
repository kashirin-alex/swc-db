/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_MngrStatus_h
#define swcdb_manager_MngrStatus_h

#include "swcdb/db/Types/MngrState.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"

namespace SWC { namespace Manager {


class MngrStatus : public Protocol::Common::Params::HostEndPoints {
  public:

  typedef std::shared_ptr<MngrStatus> Ptr;

  MngrStatus() {}

  MngrStatus(uint8_t role, cid_t begin, cid_t end,
             const Comm::EndPoints& points, Comm::ConnHandlerPtr c, uint32_t pr)
             : Protocol::Common::Params::HostEndPoints(points), 
               priority(pr), state(DB::Types::MngrState::NOTSET), role(role), 
               cid_begin(begin), cid_end(end), 
               conn(c), failures(0) { 
  }
  
  virtual ~MngrStatus(){ }

  bool eq_grouping(const MngrStatus& other) const {
    return role == other.role && 
           cid_begin == other.cid_begin &&
           cid_end == other.cid_end;
  }

  size_t internal_encoded_length() const {
    size_t len = 6 
               + Serialization::encoded_length_vi64(cid_begin)
               + Serialization::encoded_length_vi64(cid_end)
               + Protocol::Common::Params::HostEndPoints::internal_encoded_length();
    return len;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i32(bufp, priority.load());
    Serialization::encode_i8(bufp, (uint8_t)state.load());
    Serialization::encode_i8(bufp, role);
    Serialization::encode_vi64(bufp, cid_begin);
    Serialization::encode_vi64(bufp, cid_end);
    Protocol::Common::Params::HostEndPoints::internal_encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    priority.store(Serialization::decode_i32(bufp, remainp));
    state.store((DB::Types::MngrState)Serialization::decode_i8(bufp, remainp));
    role = Serialization::decode_i8(bufp, remainp);
    cid_begin = Serialization::decode_vi64(bufp, remainp);
    cid_end = Serialization::decode_vi64(bufp, remainp);

    Protocol::Common::Params::HostEndPoints::internal_decode(bufp, remainp);
  }

  void print(std::ostream& out) const {
    out << "MngrStatus(priority=" << priority
        << " state=" << (int)state
        << " role=" << DB::Types::MngrRole::to_string(role)
        << " cid=" << cid_begin << '-' << cid_end;
    Protocol::Common::Params::HostEndPoints::print(out << ' ');
    out << ')';
  }

  std::atomic<uint32_t>               priority;
  std::atomic<DB::Types::MngrState>   state;
  uint8_t                             role;
  cid_t                               cid_begin;
  cid_t                               cid_end;

  Comm::ConnHandlerPtr                conn; // mngr-inchain
  int                                 failures;
};

typedef std::vector<MngrStatus::Ptr> MngrsStatus;




}}

#endif // swcdb_manager_MngrStatus_h
