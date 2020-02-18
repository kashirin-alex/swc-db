/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_PageArena_h
#define swc_core_PageArena_h

#include <mutex>
#include "swcdb/core/LockAtomicUnique.h"
#include "swcdb/core/Comparators.h"

#include <unordered_set>
#include <string>
#include <cstring>
#include <cassert>



namespace SWC { namespace Mem { 


struct Item final {
  public:
  
  typedef Item*           Ptr;

  std::atomic<uint64_t>   count;
  const uint32_t          size_;
  const uint8_t*          data_;
  const size_t            hash_;


  Item() = delete;

  Item(const Item& other) = delete;

  Item operator=(const Item& other) = delete;


  static Item::Ptr make(const uint8_t* buf, uint32_t size);

  Item(const uint8_t* ptr, uint32_t size)
      : count(0), size_(size), data_(ptr), hash_(_hash()) {
  }

  const size_t _hash() {
    size_t ret = 0;
    const uint8_t* dp = data_;
    for(uint32_t sz = size_; sz; --sz)
      ret += (ret << 3) + *dp++;
    return ret;
    //return std::hash<std::string_view>{}(
    //  std::string_view((const char*)data_, size_));
  }

  void allocate() {
    data_ = (uint8_t*)memcpy(new uint8_t[size_], data_, size_);
  }

  ~Item() { 
    if(data_)
      delete data_;
  } 

  const uint32_t size() const {
    return size_;
  }
  
  const uint8_t* data() const {
    return data_;
  }

  const size_t hash() const {
    return hash_;
  }
  
  Ptr use() {
    ++count;
    return this;
  }

  const bool unused() {
    return !--count;
  }

  void release();

  const std::string_view to_string() const {
    return std::string_view((const char*)data_, size_);
  }

  bool less(const Item& other) const {
    return size_ < other.size_ ||
           (size_ == other.size() && memcmp(data_, other.data(), size_) < 0);
  }

  const bool equal(const Item& other) const {
    return Condition::eq(data_, size_, other.data(), other.size());
  }

  struct Equal {
    bool operator()(const Ptr lhs, const Ptr rhs) const {
      return lhs->equal(*rhs);
    }
  };

  struct Hash {
    size_t operator()(const Ptr arr) const noexcept {
      return arr->hash();
    }
  };

  struct Less {
    bool operator()(const Ptr lhs, const Ptr rhs) const {
      return lhs->less(*rhs);
    }
  };

};




typedef std::unordered_set<Item::Ptr, Item::Hash, Item::Equal> PageBase;
class Page : public PageBase {
  //public std::set<Item::Ptr, Item::Less> {
  public:
  Page() : PageBase(8) { }
  
  Item::Ptr use(const uint8_t* buf, uint32_t size) {
    LockAtomic::Unique::Scope lock(m_mutex);
    //std::scoped_lock lock(m_mutex);

    auto tmp = new Item(buf, size);
    auto r = insert(tmp);
    if(r.second) {
      (*r.first)->allocate();
    } else { 
      tmp->data_ = nullptr;
      delete tmp;
    }
    return (*r.first)->use();
  }
  
  void free(Item::Ptr ptr) {
    LockAtomic::Unique::Scope lock(m_mutex);
    //std::scoped_lock lock(m_mutex);
    
    auto it = find(ptr);
    if(it != end() && !ptr->count) {
      erase(it);
      delete ptr;
    }
  }

  const size_t count() const {
    LockAtomic::Unique::Scope lock(m_mutex);
    //std::scoped_lock lock(m_mutex);

    size_t sz = size();
    //for(Page* nxt = m_page; nxt ; nxt->next_page(nxt))
    //  sz += nxt->count();
    return sz;
  }
  /*
  void next_page(Page*& page) const {
    LockAtomic::Unique::Scope lock(m_mutex);
    //std::scoped_lock lock(m_mutex);
    page = m_page;
  }
  */
  private:
  mutable LockAtomic::Unique    m_mutex;
  //mutable std::mutex            m_mutex;
  //Page*                         m_page = nullptr; // upper ranges

};

/* Vector with narrow on hash
typedef std::vector<Item::Ptr> PageBase;
class Page : public PageBase {
  //public std::set<Item::Ptr, Item::Less> {
  public:


  Item::Ptr use(const uint8_t* buf, uint32_t sz) {
    LockAtomic::Unique::Scope lock(m_mutex);
    //std::scoped_lock lock(m_mutex);

    auto ptr = new Item(buf, sz);

    auto it = begin() + _narrow(*ptr);
    for(; it < end() && (*it)->hash() <= ptr->hash(); ++it) {
      if((*it)->hash() == ptr->hash() && (*it)->equal(*ptr)) {
        ptr->data_ = nullptr;
        delete ptr;
        return (*it)->use();
      }
    }
    ptr->allocate();
    insert(it, ptr);
    return ptr->use();
  }
  
  void free(Item::Ptr ptr) {
    LockAtomic::Unique::Scope lock(m_mutex);

    for(auto it = begin() + _narrow(*ptr); 
        it < end() && (*it)->hash() <= ptr->hash(); ++it) {
      if((*it)->hash() == ptr->hash() && (*it)->equal(*ptr)) {
        delete *it;
        erase(it);
        return;
      }
    }
    assert(!ptr);
  }

  const size_t count() const {
    LockAtomic::Unique::Scope lock(m_mutex);
    return size();
  }

  private:
    
  size_t _narrow(const Item& item) const {
    size_t offset = 0;
    if(size() < narrow_sz || !item.size())
      return offset;

    size_t sz = size() >> 1;
    offset = sz; 
    for(;;) {
      if((*(begin() + offset))->hash() < item.hash()) {
        if(sz < narrow_sz)
          break;
        offset += sz >>= 1; 
        continue;
      }
      if((sz >>= 1) == 0)
        ++sz;  
      if(offset < sz) {
        offset = 0;
        break;
      }
      offset -= sz;
    }
    return offset;
  }

  mutable LockAtomic::Unique    m_mutex;
  static const size_t           narrow_sz = 20;

};
*/


class Arena final {
  
  public:
  Item::Ptr use(const uint8_t* buf, uint32_t size) {
    return page_by_sz(size).use(buf, size);
  }
  
  void free(Item::Ptr ptr) {
    page_by_sz(ptr->size()).free(ptr);
  }

  Page& page_by_sz(uint32_t sz) {
    return _pages[(sz > 1022 ? 1023 : (sz ? sz-1 : 0) ) >> 2];
  } 

  const size_t count() const {
    size_t sz = 0;
    for(uint16_t i=0;i<256;++i)
      sz += _pages[i].count();
    return sz;
  }

  const size_t pages() const {
    return 256;
  }

  const Page& page(uint8_t idx) const {
    return _pages[idx];
  }

  private:
  Page  _pages[256];
};

} }



namespace SWC { namespace Env {
  SWC::Mem::Arena PageArena;
} }

namespace SWC { namespace Mem { 



Item::Ptr Item::make(const uint8_t* buf, uint32_t size) { 
  return Env::PageArena.use(buf, size);
}

void Item::release() { 
  if(unused())
    Env::PageArena.free(this);
}





struct ItemPtr final { // Item as SmartPtr

  ItemPtr() : ptr(nullptr) { }

  ItemPtr(const ItemPtr& other) 
          : ptr(other.ptr ? other.ptr->use() : nullptr) { 
  }

  ItemPtr(const uint8_t* buf, uint32_t size) 
          : ptr(Item::make(buf, size)) {
  }

  ItemPtr operator=(const ItemPtr& other) {
    ptr = other.ptr->use();
    return *this;
  }

  ~ItemPtr() { 
    release();
  }

  void release() {
    if(ptr)
      ptr->release();
  }

  void use(const ItemPtr& other) { 
    release();
    ptr = other.ptr->use();
  }

  void use(const uint8_t* buf, uint32_t size) { 
    release();
    ptr = Item::make(buf, size);
  }

  const uint8_t* data() const {
    return ptr->data();
  }

  const uint32_t size() const {
    return ptr->size();
  }

  const std::string_view to_string() const {
    return ptr->to_string();
  }
  
  mutable Item::Ptr ptr = nullptr;
};

const bool operator==(ItemPtr l, ItemPtr r) {
  return l.ptr == r.ptr;
}


} }
#endif // swc_core_PageArena_h





