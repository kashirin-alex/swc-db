
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_Stats_Stat_h
#define swc_db_Stats_Stat_h



namespace SWC{ namespace Stats {

class Stat {
  public:
  Stat(): m_count(0), m_avg(0), m_max(0), m_min(-1) {}
  virtual ~Stat(){}

  void add(uint64_t v){
    std::lock_guard lock(m_mutex);
    m_avg *= m_count;
    m_avg += v;
    m_avg /= ++m_count;
    if(v > m_max)
      m_max = v;
    if(v < m_min)
      m_min = v;
  }

  uint64_t avg(){
    std::lock_guard lock(m_mutex);
    return m_avg;
  }
  uint64_t max(){
    std::lock_guard lock(m_mutex);
    return m_max;
  }
  uint64_t min(){
    std::lock_guard lock(m_mutex);
    return m_min;
  }

  uint64_t count(){
    std::lock_guard lock(m_mutex);
    return m_count;
  }

  private:
  std::mutex m_mutex;
  uint64_t m_count;
  uint64_t m_avg;
  uint64_t m_min;
  uint64_t m_max;
};

}}

#endif // swc_db_Stats_Stat_h