/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Statistics.h"


namespace SWC { namespace FS {


Statistics::Metric::Tracker::Tracker(Metric* m) noexcept : m(m) { }

void Statistics::Metric::Tracker::stop() noexcept {
  if(auto ptr = m.exchange(nullptr))
    ptr->add(elapsed());
}



void Statistics::Metric::add(uint48_t ns) noexcept {
  lock();
  if(UINT64_MAX - m_total > ns) {
    ++m_count;
    m_total += ns;
  }
  if(!m_min || m_min > ns)
    m_min = ns;
  if(m_max < ns)
    m_max = ns;
  unlock();
}

void Statistics::Metric::gather(Metric& m) noexcept {
  lock();
  m.m_count = m_count;
  m.m_total = m_total;
  m.m_min = m_min;
  m.m_max = m_max;
  m_count = 0;
  m_max = m_min = 0;
  m_total = 0;
  unlock();
}

void Statistics::Metric::reset() noexcept {
  lock();
  m_count = 0;
  m_max = m_min = 0;
  m_total = 0;
  unlock();
}



void Statistics::gather(Statistics& stats) noexcept {
  for(uint8_t i = 0; i < Command::MAX; ++i) {
    metrics[i].gather(stats.metrics[i]);
  }
}

void Statistics::reset() noexcept {
  for(auto& m : metrics) {
    m.reset();
  }
}

}}
