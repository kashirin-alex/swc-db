/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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

  MngrStatus(uint64_t  begin, uint64_t  end,
             const EndPoints& points, 
             ConnHandlerPtr c, uint32_t pr)
             : col_begin(begin), col_end(end), 
               Protocol::Common::Params::HostEndPoints(points), 
               conn(c), priority(pr), state(Types::MngrState::NOTSET),
               failures(0) { }
  
  virtual ~MngrStatus(){ }

  size_t encoded_length_internal() const {
    size_t len = 5 
               + Serialization::encoded_length_vi64(col_begin)
               + Serialization::encoded_length_vi64(col_end)
               + Protocol::Common::Params::HostEndPoints::encoded_length_internal();
    return len;
  }

  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i32(bufp, priority);
    Serialization::encode_i8(bufp, (uint8_t)state.load());
    Serialization::encode_vi64(bufp, col_begin);
    Serialization::encode_vi64(bufp, col_end);
    Protocol::Common::Params::HostEndPoints::encode_internal(bufp);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp)  {
    priority = Serialization::decode_i32(bufp, remainp);
    state.store((Types::MngrState)Serialization::decode_i8(bufp, remainp));
    col_begin = Serialization::decode_vi64(bufp, remainp);
    col_end = Serialization::decode_vi64(bufp, remainp);

    Protocol::Common::Params::HostEndPoints::decode_internal(version, bufp, remainp);
  }

  std::string to_string(){
    std::string s("MngrStatus:");
    
    s.append(" priority=");
    s.append(std::to_string(priority));

    s.append(" state=");
    s.append(std::to_string((uint8_t)state.load()));

    s.append(" col(begin=");
    s.append(std::to_string(col_begin));
    s.append(" end=");
    s.append(std::to_string(col_end));

    s.append(" ");
    s.append(Protocol::Common::Params::HostEndPoints::to_string());
    return s;
  }

  uint32_t          priority;
  std::atomic<Types::MngrState>  state;
  uint64_t          col_begin;
  uint64_t          col_end;

  ConnHandlerPtr    conn; // mngr-inchain
  int               failures;
};

typedef std::vector<MngrStatus::Ptr> MngrsStatus;




}}

#endif // swc_manager_MngrStatus_h
