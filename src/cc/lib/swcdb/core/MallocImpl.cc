/* 
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/core/Compat.h"

#include <math.h> 
#include <sys/mman.h>
#include <cerrno>



namespace SWC { namespace Core {


/** INITIATE SINGLETONE MALLOC INSTANCE. **/
Malloc memory;


/* local namespace */
namespace {


#define NOT_USED_ADDR (void*)-1

const int mmap_prot = PROT_READ | PROT_WRITE;
const int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;  
                    // | MAP_LOCKED | MAP_HUGETLB

#define MMAP_PROT 
std::atomic<size_t> total_mmap_user  = 0;
std::atomic<size_t> total_mmap_allotments  = 0;
std::atomic<size_t> total_mmap_alloties  = 0;

} // local namespace







struct Malloc::Allotment final {

  SWC_CAN_INLINE
  void init() noexcept {
    set_safe(NOT_USED_ADDR, 0);
    _next.store(nullptr, std::memory_order_relaxed);
  }
  
  SWC_CAN_INLINE
  void set_safe(void* for_addr, size_t for_length) noexcept {
    _addr.store(for_addr, std::memory_order_relaxed);
    _length = for_length;
    _offset.store(for_length, std::memory_order_relaxed);
    _over_ask.store(0, std::memory_order_relaxed);
    //_processes.store(0, std::memory_order_relaxed);
  }

  SWC_CAN_INLINE
  bool can_be_used(void* for_addr, size_t for_length) noexcept {
    void* at = _addr.load(std::memory_order_relaxed);
    if(at != NOT_USED_ADDR ||
       !_addr.compare_exchange_weak(at, for_addr, std::memory_order_acq_rel))
      return false;
      
    _length = for_length;
    _offset.store(for_length, std::memory_order_relaxed);
    return true;
  }

  SWC_CAN_INLINE
  void get_addr_safe(size_t sz, void** addrp) noexcept {
    ssize_t offset = _offset.fetch_sub(sz, std::memory_order_relaxed);
    offset -= sz;
    *addrp = (uint8_t*)_addr.load(std::memory_order_relaxed) + offset;
  }

  SWC_CAN_INLINE
  Allotment* get_addr(ssize_t sz, void** addrp) noexcept {
    void* addrtmp = _addr;
    if(addrtmp == NOT_USED_ADDR) 
      return nullptr;

    if(_offset.load(std::memory_order_relaxed) > 0 &&
       !_over_ask.load(std::memory_order_relaxed)) {
      //++_processes;

      ssize_t offset = _offset.fetch_sub(sz, std::memory_order_relaxed);
      offset -= sz;
      if(offset >= 0) {
        *addrp = (uint8_t*)addrtmp + offset;
      } else {
        _over_ask += sz;
      }
      
      /* free
      if(_processes.fetch_sub(1, std::memory_order_seq_cst) == 1) {
        size_t asked = _over_ask.exchange(0, std::memory_order_relaxed);
        if(asked) {
          _offset += asked;
        }
      }
      */
      //--_processes;
    }
    return *addrp ? this : _next.load(std::memory_order_release);
  }

  std::atomic<void*>      _addr; 
  size_t                  _length;
  std::atomic<ssize_t>    _offset;
  std::atomic<size_t>     _over_ask;
  std::atomic<Allotment*> _next;
  //std::atomic<size_t>     _processes;

};


struct Malloc::Allotments final {
  std::atomic<Allotment*>  _allotment;
  std::atomic<Allotments*> _next;
  
  SWC_CAN_INLINE
  Allotments* init(void* m_addr, size_t len, 
                   void* for_addr, size_t for_length,
                   Allotment** allotmentp) noexcept {
    Allotment* allotment = (Allotment*)((uint8_t*)m_addr+sizeof(Allotments));
    
    _allotment.store(allotment, std::memory_order_relaxed);
    _next.store(nullptr, std::memory_order_relaxed);

    for(size_t n = sizeof(Allotments); 
        (n+=sizeof(Allotment)) <= len; ++allotment)
      allotment->init();

    *allotmentp = --allotment;
    (*allotmentp)->set_safe(for_addr, for_length);
    return this;
  }
};


struct Malloc::Alloti final {
  
  SWC_CAN_INLINE
  void init() noexcept {
    set_safe(nullptr, NOT_USED_ADDR, 0);
    _next.store(nullptr, std::memory_order_relaxed);
  }
  
  SWC_CAN_INLINE
  void set_safe(Allotment* for_allotment, 
                void* for_addr, size_t for_length) noexcept {
    _allotment.store(for_allotment, std::memory_order_relaxed);
    _addr = for_addr;
    _length.store(for_length, std::memory_order_relaxed);
  }

  SWC_CAN_INLINE
  bool can_be_used(Allotment* for_allotment, 
                   void* for_addr, size_t for_length) noexcept {
    Allotment* at = _allotment.load(std::memory_order_relaxed);
    if(at != nullptr ||
       !_allotment.compare_exchange_weak(
         at, for_allotment, std::memory_order_acq_rel))
      return false;

    _addr = for_addr;
    _length.store(for_length, std::memory_order_relaxed);
    return true;
  }

  std::atomic<Allotment*> _allotment;
  void*                   _addr;
  std::atomic<size_t>     _length;
  std::atomic<Alloti*>    _next;
};


struct Malloc::Alloties final {
  std::atomic<Alloti*>   _alloti;
  std::atomic<Alloties*> _next;
  
  SWC_CAN_INLINE
  Alloties* init(void* m_addr, size_t len, 
                 Allotment* for_allotment,
                 void* for_addr, size_t for_length, 
                 Alloti** allotip) noexcept {
    Alloti* alloti = (Alloti*)((uint8_t*)m_addr+sizeof(Alloties));
    
    _alloti.store(alloti, std::memory_order_relaxed);
    _next.store(nullptr, std::memory_order_relaxed);

    for(size_t n = sizeof(Alloties); 
        (n+=sizeof(Alloti)) <= len; ++alloti)
      alloti->init();

    *allotip = --alloti;
    (*allotip)->set_safe(for_allotment, for_addr, for_length);
    return this;
  }
};



/*
Malloc::MaxAvail::MaxAvail() : avail(0), allotment(nullptr) { }
    
void Malloc::MaxAvail::adjust(size_t remain, Malloc::Allotment* by) {
  if(avail < remain || allotment == by) {
    avail = remain;
    allotment = by;
  }
}
*/






Malloc::Malloc() 
        : p_sz(sysconf(_SC_PAGE_SIZE)),
          p_sz_allotments(p_sz * 20),
          p_sz_alloties(p_sz * 40),
          p_sz_allotment(p_sz * 1024),
          _allotments(nullptr),
          _alloties(nullptr),
          _allotment(nullptr) {
  printf(
    "Malloc Initialization page-size=%lu"
    " Allotments-SZ(%lu)-slots(%lu) Allotment-SZ(%lu)"
    " Alloties-SZ(%lu)-slots(%lu) Alloti-SZ(%lu)\n",
    p_sz, 
    sizeof(Allotments), 
    (p_sz_allotments-sizeof(Allotments))/sizeof(Allotment),
    sizeof(Allotment), 
    sizeof(Alloties), 
    (p_sz_alloties-sizeof(Alloties))/sizeof(Alloti),
    sizeof(Alloti)
  );

  if(!p_sz || p_sz == size_t(-1)) {
    printf("Bad Malloc Initialization _SC_PAGE_SIZE exiting\n");
    exit(1);
  }

  for(auto& alloti : _alloti_used)
    alloti = nullptr;
  for(auto& alloti : _alloti_free)
    alloti = nullptr;
  /*
  // Initial Allotments , optional sanity check
  _allocate_allotment(NOT_USED_ADDR, 0);
  if(!_allotments || errno) {
    printf("Init Allotment-SYS-ALLOTMENTS Bad err=%d exiting\n", errno);
    exit(1);
  }
  */
}

Malloc::~Malloc() {
  printf("total_mmap_allotments=%lu \n", total_mmap_allotments.load());
  printf("total_mmap_alloties=%lu \n", total_mmap_alloties.load());
  printf("total_mmap_user=%lu \n", total_mmap_user.load());
  printf("total_mmap=%lu \n", total_mmap_user.load() + total_mmap_alloties.load() +total_mmap_allotments.load());
  if(_allotments)
    munmap(_allotments, p_sz_allotments);
}



SWC_CAN_INLINE
Malloc::Allotment* 
Malloc::_allocate_allotment(void* for_addr, size_t for_length) noexcept {
  Allotment* allotment;
  for(Allotments* current;;) {
    for(current = _allotments.load(std::memory_order_release);
        current;
        current = current->_next) {
      allotment = current->_allotment.load(std::memory_order_relaxed);
      for(size_t n = sizeof(Allotments);
          (n+=sizeof(Allotment)) <= p_sz_allotments; ++allotment) {
        if(allotment->can_be_used(for_addr, for_length))
          return allotment;
      }
    }
    if(_allotments_mutex.try_lock()) 
      // allocate one Allotments at a time
      // good for p_sz_allotments / sizeof(Allotments) = N-Allotment
      break;
  }
  allotment = nullptr;

  errno = 0;
  void* m_addr = mmap(0, p_sz_allotments, mmap_prot, mmap_flags, -1, 0);
  if(!errno) {
    if(mlock(m_addr, p_sz_allotments)) {
      munmap(m_addr, p_sz_allotments);

    } else {
      Allotments* current = ((Allotments*)m_addr)
        ->init(m_addr, p_sz_allotments, for_addr, for_length, &allotment);

      current->_next.store(
        _allotments.exchange(current, std::memory_order_release),
        std::memory_order_relaxed
      );
      total_mmap_allotments += p_sz_allotments;
    }
  }
  _allotments_mutex.unlock();
  return allotment;
}


SWC_CAN_INLINE
Malloc::Alloti* 
Malloc::_allocate_alloti(Allotment* for_allotment,
                         void* for_addr, size_t for_length) noexcept {
  Alloti* alloti;
  for(Alloties* current;;) {
    for(current = _alloties.load(std::memory_order_release);
        current;
        current = current->_next) {
      alloti = current->_alloti.load(std::memory_order_relaxed);
      for(size_t n = sizeof(Alloties);
          (n+=sizeof(Alloti)) <= p_sz_alloties; ++alloti) {
        if(alloti->can_be_used(for_allotment, for_addr, for_length))
          return alloti;
      }
    }
    if(_alloties_mutex.try_lock()) 
      // allocate one Alloties at a time
      // good for p_sz_alloties / sizeof(Alloties) = N-Alloti
      break;
  }
  alloti = nullptr;

  errno = 0;
  void* m_addr = mmap(0, p_sz_alloties, mmap_prot, mmap_flags, -1, 0);
  if(!errno) {
    if(mlock(m_addr, p_sz_alloties)) {
      munmap(m_addr, p_sz_alloties);

    } else {
      Alloties* current = ((Alloties*)m_addr)
        ->init(m_addr, p_sz_alloties, 
               for_allotment, for_addr, for_length, &alloti);

      current->_next.store(
        _alloties.exchange(current, std::memory_order_release),
        std::memory_order_relaxed
      );
      total_mmap_alloties += p_sz_alloties;
    }
  }
  _alloties_mutex.unlock();
  return alloti;
}



///
SWC_CAN_INLINE
void* Malloc::_reg_alloti_used(uint8_t group, 
                               Allotment* allotment, 
                               void* addr, size_t sz) noexcept {
  Alloti* alloti = _allocate_alloti(allotment, addr, sz);
  alloti->_next.store(
    _alloti_used[group].exchange(alloti, std::memory_order_release),
    std::memory_order_relaxed
  );
  return addr;
}



///
enum AllocGroup : uint8_t {
  ALLOC_GROUP_1 = 0,
  ALLOC_GROUP_2 = 1,
  ALLOC_GROUP_3 = 2,
  ALLOC_GROUP_4 = 3,
  ALLOC_GROUP_5 = 4,
  ALLOC_GROUP_6 = 5,
  ALLOC_GROUP_7 = 6,
  ALLOC_GROUP_8 = 7,
  ALLOC_GROUP_9 = 8,
  ALLOC_GROUP_10 = 9,
  ALLOC_GROUP_11 = 10
};

SWC_CAN_INLINE
static uint8_t allocation_group(size_t sz) noexcept {
  switch(sz) {
    case 1 ... 10: 
      return ALLOC_GROUP_1;
    
    case 11 ... 30: 
      return ALLOC_GROUP_2;
    
    case 31 ... 100: 
      return ALLOC_GROUP_3;
    
    case 101 ... 200: 
      return ALLOC_GROUP_4;
    
    case 201 ... 500: 
      return ALLOC_GROUP_5;
    
    case 501 ... 1000: 
      return ALLOC_GROUP_6;
    
    case 1001 ... 50000: 
      return ALLOC_GROUP_7;
    
    case 50001 ... 1000000: 
      return ALLOC_GROUP_8;
    
    case 1000001 ... 10000000: 
      return ALLOC_GROUP_9;
    
    case 10000001 ... 50000000: 
      return ALLOC_GROUP_10;
    
    default:
      return ALLOC_GROUP_11;
  }
}



///
SWC_CAN_INLINE
void* Malloc::_allocate(size_t sz) noexcept {
  if(!sz)
    return nullptr;

  void* addr = nullptr;

  uint8_t group = allocation_group(sz);
  
  //
  (void)_alloti_free[group];


  //
  Allotment* current = _allotment.load(std::memory_order_release);
  while(current) {
    current = current->get_addr(sz, &addr);
    if(addr) 
      return _reg_alloti_used(group, current, addr, sz);
  }

  //
  size_t len = sz <= p_sz_allotment 
    ? p_sz_allotment
    : p_sz_allotment * (sz/p_sz_allotment);

  errno = 0;
  void* m_addr = mmap(0, len, mmap_prot, mmap_flags, -1, 0);
  if(!errno) {
    current = _allocate_allotment(m_addr, len);

    if(current) {
      if(mlock(m_addr, len)) {
        munmap(m_addr, len);

      } else {
        current->get_addr_safe(sz, &addr);

        //_allotment_mutex.lock();
        current->_next.store(
          _allotment.exchange(current, std::memory_order_release),
          std::memory_order_relaxed
        );
        //_allotment_mutex.unlock();

        total_mmap_user += len;
        return _reg_alloti_used(group, current, addr, sz);
      }
    }
  }
  return addr;
}


void Malloc::_free(void* ptr) noexcept {
  (void)ptr;
}


void Malloc::_release(size_t sz) noexcept {
  (void)sz;
}




//

void* Malloc::allocate(size_t sz) noexcept {
  return memory._allocate(sz);;
}

void Malloc::free(void* ptr) noexcept {
  memory._free(ptr);
}

void Malloc::release(size_t sz) noexcept {
  memory._release(sz);
}


}}
