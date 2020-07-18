/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_CompletionCounter_h
#define swc_core_CompletionCounter_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/LockAtomicUnique.h"


namespace SWC { 
  
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
    LockAtomic::Unique::scope lock(m_mutex);
    ++m_count;
  }

  bool is_last() {
    LockAtomic::Unique::scope lock(m_mutex);
    return !--m_count;
  }

  uint64_t count() {
    LockAtomic::Unique::scope lock(m_mutex);
    return m_count;
  }

  private:
  LockAtomic::Unique  m_mutex;
  CountT              m_count;
};

}



#endif // swc_core_CompletionCounter_h
