
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Ranger_h
#define swcdb_manager_Ranger_h

#include "swcdb/db/Types/MngrRangerState.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"


namespace SWC { namespace Manager {


class Ranger : public Comm::Protocol::Common::Params::HostEndPoints {

  public:

  using State = DB::Types::MngrRanger::State;

  typedef std::shared_ptr<Ranger> Ptr;

  Ranger(): rgrid(0), state(State::NONE), 
            failures(0), interm_ranges(0), load_scale(0) {
  }
                       
  Ranger(rgrid_t rgrid, const Comm::EndPoints& endpoints)
        : Comm::Protocol::Common::Params::HostEndPoints(endpoints),
          rgrid(rgrid), state(State::NONE), 
          failures(0), interm_ranges(0), load_scale(0) {
  }

  virtual ~Ranger() { }

  void print(std::ostream& out) const {
    out << "[rgrid="          << rgrid
        << " state="          << DB::Types::to_string(state)
        << " failures="       << failures
        << " load_scale="     << load_scale
        << " interm_ranges="  << interm_ranges;
    Comm::Protocol::Common::Params::HostEndPoints::print(out << ' ');
    if(m_queue)
      m_queue->print(out << ' ');
    out << ']';
  }

  size_t internal_encoded_length() const {
    size_t len = 3
      + Serialization::encoded_length_vi64(rgrid.load())
      + Comm::Protocol::Common::Params::HostEndPoints::internal_encoded_length();
    return len;
  }

  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, (uint8_t)state.load());
    Serialization::encode_vi64(bufp, rgrid.load());
    Serialization::encode_i16(bufp, load_scale.load());
    Comm::Protocol::Common::Params::HostEndPoints::internal_encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    state = (State)Serialization::decode_i8(bufp, remainp);
    rgrid = Serialization::decode_vi64(bufp, remainp);
    load_scale = Serialization::decode_i16(bufp, remainp);
    Comm::Protocol::Common::Params::HostEndPoints::internal_decode(bufp, remainp);
  }

  void init_queue() {
    m_queue = Env::Clients::get()->rgr->get(endpoints);
  }

  void put(const Comm::client::ConnQueue::ReqBase::Ptr& req) {
    m_queue->put(req);
  }

  void stop() {
    m_queue->stop();
  }

  std::atomic<rgrid_t>  rgrid;
  std::atomic<State>    state;

  std::atomic<int32_t>  failures;
  std::atomic<size_t>   interm_ranges;
  std::atomic<uint16_t> load_scale;
  // int32_t resource;
  
  private:
  Comm::client::Host::Ptr m_queue = nullptr;

};
typedef std::vector<Ranger::Ptr>  RangerList;

}}

#endif // swcdb_manager_Ranger_h
