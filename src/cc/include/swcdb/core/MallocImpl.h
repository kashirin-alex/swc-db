/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_MallocImpl_h
#define swcdb_core_MallocImpl_h


#include "swcdb/core/MutexAtomic.h"
#include "swcdb/core/MutexSptd.h"


namespace SWC { namespace Core {

class Malloc;
extern Malloc memory;


class Malloc final {
  public:

  static void* allocate(size_t sz) noexcept;

  static void free(void* ptr) noexcept;

  static void release(size_t sz) noexcept;

  Malloc();

  ~Malloc();
  
  private:

  void* _allocate(size_t sz) noexcept;

  void _free(void* ptr) noexcept;

  void _release(size_t sz) noexcept;


  struct Allotments;
  struct Allotment;

  struct Alloties;
  struct Alloti;
  
  Allotment* _allocate_allotment(void* for_addr, size_t for_length) noexcept;

  Alloti* _allocate_alloti(Allotment* for_allotment,
                           void* for_addr, size_t for_length) noexcept;

  void* _reg_alloti_used(uint8_t group, 
                         Allotment* for_allotment,
                        void* addr, size_t sz) noexcept;

  const size_t          p_sz;
  const size_t          p_sz_allotments;
  const size_t          p_sz_alloties;
  const size_t          p_sz_allotment;

  /*
  struct MaxAvail final {
    std::atomic<size_t>     avail;
    std::atomic<Allotment*> allotment;
    MaxAvail() ;
    void adjust(size_t remain, Allotment* by);
  };
  MaxAvail                  _max;
  */

  Core::MutexAtomic         _allotments_mutex;
  std::atomic<Allotments*>  _allotments;

  Core::MutexAtomic         _alloties_mutex;
  std::atomic<Alloties*>    _alloties;

  Core::MutexAtomic         _allotment_mutex;
  std::atomic<Allotment*>   _allotment;
  
  std::atomic<Alloti*>      _alloti_used[11];
  std::atomic<Alloti*>      _alloti_free[11];

}; 


}}


#endif // swcdb_core_MallocImpl_h
