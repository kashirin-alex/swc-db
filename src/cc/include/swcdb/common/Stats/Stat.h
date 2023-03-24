/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_Common_Stats_Stat_h
#define swcdb_Common_Stats_Stat_h


#include "swcdb/core/MutexAtomic.h"


namespace SWC { namespace Common { namespace Stats {



template<typename ValueT>
struct MinMaxAvgCount {
  ValueT  count;
  ValueT  total;
  ValueT  min;
  ValueT  max;

  SWC_CAN_INLINE
  MinMaxAvgCount() noexcept : count(0), total(0), min(0), max(0) { }

  SWC_CAN_INLINE
  void add(ValueT v) noexcept {
    if(std::numeric_limits<ValueT>::max() - total > v) {
      ++count;
      total += v;
    }
    if(v > max)
      max = v;
    if(!min || v < min)
      min = v;
  }

  SWC_CAN_INLINE
  ValueT avg() const noexcept {
    return count ? (total/count) : 0;
  }

  SWC_CAN_INLINE
  void gather(MinMaxAvgCount<ValueT>& to) noexcept {
    to.count = count;
    to.min = min;
    to.max = max;
    to.total = total;
    reset();
  }

  SWC_CAN_INLINE
  void reset() noexcept {
    count = total = min = max = 0;
  }
};



template<typename ValueT>
class MinMaxAvgCount_Safe {
  public:

  SWC_CAN_INLINE
  MinMaxAvgCount_Safe() noexcept : m_mutex(), m_value() { }

  SWC_CAN_INLINE
  void add(ValueT v) noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    m_value.add(v);
  }

  SWC_CAN_INLINE
  ValueT count() const noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_value.count;
  }

  SWC_CAN_INLINE
  ValueT total() const noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_value.total;
  }

  SWC_CAN_INLINE
  ValueT avg() const noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_value.avg();
  }

  SWC_CAN_INLINE
  ValueT max() const noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_value.max;
  }

  SWC_CAN_INLINE
  ValueT min() const noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_value.min;
  }

  SWC_CAN_INLINE
  void gather(MinMaxAvgCount<ValueT>& to) noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    m_value.gather(to);
  }

  SWC_CAN_INLINE
  void reset() noexcept {
    Core::MutexAtomic::scope lock(m_mutex);
    m_value.reset();
  }

  void print(std::ostream& out) const {
    ValueT min, max, avg;
    {
      Core::MutexAtomic::scope lock(m_mutex);
      min = m_value.min;
      max =  m_value.max;
      avg = m_value.avg();
    }
    out << "(min=" << min << " max=" << max << " avg=" << avg << ')';
  }

  private:
  mutable Core::MutexAtomic m_mutex;
  MinMaxAvgCount<ValueT>    m_value;
};


typedef MinMaxAvgCount_Safe<uint64_t> Stat;

}}}

#endif // swcdb_Common_Stats_Stat_h
