/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Ranger_h
#define swcdb_manager_Ranger_h


#include "swcdb/db/Types/MngrRangerState.h"


namespace SWC { namespace Manager {

namespace RangerState = DB::Types::MngrRangerState;

class Ranger final {

  public:

  typedef std::shared_ptr<Ranger> Ptr;

  /*
  SWC_CAN_INLINE
  Ranger() noexcept
          : rgrid(0),
            interm_ranges(0), failures(0), load_scale(0),
            state(RangerState::NONE), m_rebalance(0) {
  }
  */

  SWC_CAN_INLINE
  Ranger(rgrid_t a_rgrid, const Comm::EndPoints& a_endpoints)
        : rgrid(a_rgrid), endpoints(a_endpoints),
          interm_ranges(0), failures(0), load_scale(0),
          state(RangerState::NONE), m_mutex(), m_rebalance(0),
          m_queue(nullptr) {
  }

  SWC_CAN_INLINE
  Ranger(const uint8_t** bufp, size_t* remainp)
        : rgrid(Serialization::decode_vi64(bufp, remainp)),
          endpoints(Serialization::decode_endpoints(bufp, remainp)),
          interm_ranges(0), failures(0),
          load_scale(Serialization::decode_i16(bufp, remainp)),
          state(Serialization::decode_i8(bufp, remainp)),
          m_mutex(), m_rebalance(Serialization::decode_i8(bufp, remainp)),
          m_queue(nullptr) {
  }

  SWC_CAN_INLINE
  Ranger(const Ranger& other, const Comm::EndPoints& a_endpoints)
        : rgrid(other.rgrid.load()),
          endpoints(a_endpoints),
          interm_ranges(other.interm_ranges.load()),
          failures(other.failures.load()),
          load_scale(other.load_scale.load()),
          state(other.state.load()),
          m_mutex(), m_rebalance(other.rebalance()),
          m_queue(nullptr) {
  }

  ~Ranger() noexcept { }

  void print(std::ostream& out) const {
    out << "[rgrid="          << rgrid.load()
        << " state="          << RangerState::to_string(state.load())
        << " failures="       << failures.load()
        << " load_scale="     << load_scale.load()
        << " rebalance="      << int(rebalance())
        << " interm_ranges="  << interm_ranges.load();
    Comm::print(out << ' ', endpoints);
    if(m_queue)
      m_queue->print(out << ' ');
    out << ']';
  }

  size_t encoded_length() const noexcept {
    size_t len = 4
      + Serialization::encoded_length_vi64(rgrid.load())
      + Serialization::encoded_length(endpoints);
    return len;
  }

  void encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, rgrid.load());
    Serialization::encode(bufp, endpoints);
    Serialization::encode_i16(bufp, load_scale.load());
    Serialization::encode_i8(bufp, state.load());
    Serialization::encode_i8(bufp, rebalance());
  }

  SWC_CAN_INLINE
  void init_queue() {
    m_queue = Env::Clients::get()->get_rgr_queue(endpoints);
  }

  SWC_CAN_INLINE
  void put(const Comm::client::ConnQueue::ReqBase::Ptr& req) {
    m_queue->put(req);
  }

  void stop() {
    m_queue->stop();
  }

  SWC_CAN_INLINE
  void rebalance(uint8_t num) {
    Core::MutexAtomic::scope lock(m_mutex);
    m_rebalance = num;
  }

  SWC_CAN_INLINE
  uint8_t rebalance() const {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_rebalance;
  }

  SWC_CAN_INLINE
  bool can_rebalance() {
    Core::MutexAtomic::scope lock(m_mutex);
    if(m_rebalance) {
      --m_rebalance;
      return true;
    }
    return false;
  }


  Core::Atomic<rgrid_t>     rgrid;
  Comm::EndPoints           endpoints;
  Core::Atomic<size_t>      interm_ranges;
  Core::Atomic<int16_t>     failures;
  Core::Atomic<uint16_t>    load_scale;
  Core::Atomic<uint8_t>     state;


  private:

  mutable Core::MutexAtomic m_mutex;
  uint8_t                   m_rebalance;
  Comm::client::Host::Ptr   m_queue;

};

typedef Core::Vector<Ranger::Ptr>  RangerList;

}}

#endif // swcdb_manager_Ranger_h
