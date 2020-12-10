/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_CompletionCounter_h
#define swcdb_core_CompletionCounter_h


#include "swcdb/core/Compat.h"


namespace SWC { namespace Core {
  
template<class CountT=uint32_t>
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
    ++m_count;
  }

  bool is_last() {
    return m_count.fetch_sub(1, std::memory_order_relaxed) == 1;
  }

  CountT count() {
    return m_count;
  }

  CountT increment_and_count() {
    return m_count.fetch_add(1, std::memory_order_relaxed) + 1;
  }

  CountT decrement_and_count() {
    return m_count.fetch_sub(1, std::memory_order_relaxed) - 1;
  }

  private:
  std::atomic<CountT>              m_count;
};

}} // namespace SWC::Core



#endif // swcdb_core_CompletionCounter_h
