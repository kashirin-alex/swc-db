/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Ranger_h
#define swcdb_manager_Ranger_h

#include "swcdb/db/Types/MngrRangerState.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"


namespace SWC { namespace Manager {

namespace RangerState = DB::Types::MngrRangerState;

class Ranger : public Comm::Protocol::Common::Params::HostEndPoints {

  public:

  typedef std::shared_ptr<Ranger> Ptr;

  Ranger() noexcept
          : rgrid(0), state(RangerState::NONE),
            failures(0), interm_ranges(0), load_scale(0), m_rebalance(0) {
  }

  Ranger(rgrid_t rgrid, const Comm::EndPoints& endpoints)
        : Comm::Protocol::Common::Params::HostEndPoints(endpoints),
          rgrid(rgrid), state(RangerState::NONE),
          failures(0), interm_ranges(0), load_scale(0), m_rebalance(0) {
  }

  virtual ~Ranger() { }

  void print(std::ostream& out) const {
    out << "[rgrid="          << rgrid.load()
        << " state="          << RangerState::to_string(state.load())
        << " failures="       << failures.load()
        << " load_scale="     << load_scale.load()
        << " rebalance="      << int(rebalance())
        << " interm_ranges="  << interm_ranges.load();
    Comm::Protocol::Common::Params::HostEndPoints::print(out << ' ');
    if(m_queue)
      m_queue->print(out << ' ');
    out << ']';
  }

  size_t internal_encoded_length() const override {
    size_t len = 4
      + Serialization::encoded_length_vi64(rgrid.load())
      + Comm::Protocol::Common::Params::HostEndPoints::internal_encoded_length();
    return len;
  }

  void internal_encode(uint8_t** bufp) const override {
    Serialization::encode_i8(bufp, state.load());
    Serialization::encode_vi64(bufp, rgrid.load());
    Serialization::encode_i16(bufp, load_scale.load());
    Serialization::encode_i8(bufp, rebalance());
    Comm::Protocol::Common::Params::HostEndPoints::internal_encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    state.store(Serialization::decode_i8(bufp, remainp));
    rgrid.store(Serialization::decode_vi64(bufp, remainp));
    load_scale.store(Serialization::decode_i16(bufp, remainp));
    rebalance(Serialization::decode_i8(bufp, remainp));
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

  void rebalance(uint8_t num) {
    Core::MutexAtomic::scope lock(m_mutex);
    m_rebalance = num;
  }

  uint8_t rebalance() const {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_rebalance;
  }

  bool can_rebalance() {
    Core::MutexAtomic::scope lock(m_mutex);
    if(m_rebalance) {
      --m_rebalance;
      return true;
    }
    return false;
  }


  Core::Atomic<rgrid_t>     rgrid;
  Core::Atomic<uint8_t>     state;

  Core::Atomic<int32_t>     failures;
  Core::Atomic<size_t>      interm_ranges;
  Core::Atomic<uint16_t>    load_scale;

  private:

  Comm::client::Host::Ptr   m_queue = nullptr;
  mutable Core::MutexAtomic m_mutex;
  uint8_t                   m_rebalance;

};

typedef std::vector<Ranger::Ptr>  RangerList;

}}

#endif // swcdb_manager_Ranger_h
