/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Semaphore.h"


namespace SWC { namespace Core {

  
Semaphore::Semaphore(size_t sz) 
          : m_sz(sz), m_count(0) {
}

Semaphore::~Semaphore() {
  m_mutex.lock();
  if(!m_count) {
    m_mutex.unlock();
    return;
  }
  m_count = 0;
  m_cv.notify_all();
  m_mutex.unlock();

  m_mutex.lock(); 
  // let cv finish
  m_mutex.unlock();
} 

void Semaphore::acquire() {
  std::unique_lock lock_wait(m_mutex);
  if(m_count >= m_sz)
    m_cv.wait(lock_wait, [this] {return m_count < m_sz;});
  ++m_count;
}

void Semaphore::release() {
  std::scoped_lock lock(m_mutex);
  --m_count;
  m_cv.notify_all();
}

void Semaphore::wait_until_under_and_acquire(size_t sz) {
  std::unique_lock lock_wait(m_mutex);
  if(m_count >= sz)
    m_cv.wait(lock_wait, [this, sz] {return m_count < sz;});
  ++m_count;
}

void Semaphore::wait_until_under(size_t sz) {
  std::unique_lock lock_wait(m_mutex);
  if(m_count >= sz)
    m_cv.wait(lock_wait, [this, sz] {return m_count < sz;});
}

void Semaphore::wait_all() {
  std::unique_lock lock_wait(m_mutex);
  if(m_count)
    m_cv.wait(lock_wait, [this] {return !m_count;});
}

}}
