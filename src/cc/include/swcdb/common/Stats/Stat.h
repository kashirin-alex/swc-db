
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_Common_Stats_Stat_h
#define swcdb_Common_Stats_Stat_h



namespace SWC { namespace Common { namespace Stats {

class Stat {
  public:

  Stat() noexcept : m_count(0), m_avg(0), m_min(-1), m_max(0) { }

  virtual ~Stat() { }

  void add(uint64_t v) {
    Core::MutexAtomic::scope lock(m_mutex);
    m_avg *= m_count;
    m_avg += v;
    m_avg /= ++m_count;
    if(v > m_max)
      m_max = v;
    if(v < m_min)
      m_min = v;
  }

  uint64_t avg() {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_avg;
  }
  uint64_t max() {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_max;
  }
  uint64_t min() {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_min;
  }

  uint64_t count() {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_count;
  }

  private:
  Core::MutexAtomic   m_mutex;
  uint64_t            m_count;
  uint64_t            m_avg;
  uint64_t            m_min;
  uint64_t            m_max;
};

}}}

#endif // swcdb_Common_Stats_Stat_h
