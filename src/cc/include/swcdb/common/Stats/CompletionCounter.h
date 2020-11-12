/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_common_Stats_CompletionCounter_h
#define swcdb_common_Stats_CompletionCounter_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/MutexAtomic.h"


namespace SWC { namespace Common { namespace Stats {
  
template<class CountT=uint56_t>
class CompletionCounter final {
  public:

  explicit CompletionCounter(CountT start=0) 
                            : m_count(start) {
  };

  CompletionCounter(const CompletionCounter&) = delete;

  CompletionCounter(const CompletionCounter&&) = delete;
    
  CompletionCounter& operator=(const CompletionCounter&) = delete;
  
  ~CompletionCounter() {}

  void increment() {
    Core::MutexAtomic::scope lock(m_mutex);
    ++m_count;
  }

  bool is_last() {
    Core::MutexAtomic::scope lock(m_mutex);
    return !--m_count;
  }

  CountT count() {
    Core::MutexAtomic::scope lock(m_mutex);
    return m_count;
  }

  CountT increment_and_count() {
    Core::MutexAtomic::scope lock(m_mutex);
    return ++m_count;
  }

  CountT decrement_and_count() {
    Core::MutexAtomic::scope lock(m_mutex);
    return --m_count;
  }

  private:
  Core::MutexAtomic   m_mutex;
  CountT              m_count;
};

}}} // namespace SWC::Common::Stats



#endif // swcdb_common_Stats_CompletionCounter_h
