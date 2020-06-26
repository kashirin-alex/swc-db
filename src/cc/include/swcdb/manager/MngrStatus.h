/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_manager_MngrStatus_h
#define swc_manager_MngrStatus_h

#include "swcdb/db/Types/MngrState.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"

namespace SWC { namespace Manager {


class MngrStatus : public Protocol::Common::Params::HostEndPoints {
  public:

  typedef std::shared_ptr<MngrStatus> Ptr;

  MngrStatus() {}

  MngrStatus(uint8_t role, cid_t begin, cid_t end,
             const EndPoints& points, 
             ConnHandlerPtr c, uint32_t pr)
             : role(role), cid_begin(begin), cid_end(end), 
               Protocol::Common::Params::HostEndPoints(points), 
               conn(c), priority(pr), state(Types::MngrState::NOTSET),
               failures(0) { }
  
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
    state.store((Types::MngrState)Serialization::decode_i8(bufp, remainp));
    role = Serialization::decode_i8(bufp, remainp);
    cid_begin = Serialization::decode_vi64(bufp, remainp);
    cid_end = Serialization::decode_vi64(bufp, remainp);

    Protocol::Common::Params::HostEndPoints::internal_decode(bufp, remainp);
  }

  std::string to_string(){
    std::string s("MngrStatus:");
    
    s.append(" priority=");
    s.append(std::to_string(priority));

    s.append(" state=");
    s.append(std::to_string((int)state.load()));

    s.append(" role=");
    s.append(Types::MngrRole::to_string(role));

    s.append(" cid=");
    s.append(std::to_string(cid_begin));
    s.append("-");
    s.append(std::to_string(cid_end));

    s.append(" ");
    s.append(Protocol::Common::Params::HostEndPoints::to_string());
    return s;
  }

  std::atomic<uint32_t>           priority;
  std::atomic<Types::MngrState>   state;
  uint8_t                         role;
  cid_t                           cid_begin;
  cid_t                           cid_end;

  ConnHandlerPtr                  conn; // mngr-inchain
  int                             failures;
};

typedef std::vector<MngrStatus::Ptr> MngrsStatus;




}}

#endif // swc_manager_MngrStatus_h
