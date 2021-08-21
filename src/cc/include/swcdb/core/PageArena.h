/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_PageArena_h
#define swcdb_core_PageArena_h


#include "swcdb/core/Comparators.h"
#include <unordered_set>


namespace SWC { namespace Core { namespace Mem {


struct Item final {
  public:

  typedef Item*           Ptr;

  Core::Atomic<uint64_t>  count;
  const uint32_t          size_;
  const uint8_t*          data_;
  const size_t            hash_;


  Item()                        = delete;
  Item(const Item&)             = delete;
  Item(Item&& other)            = delete;
  Item& operator=(const Item&)  = delete;
  Item& operator=(Item&&)       = delete;


  static Item::Ptr make(const uint8_t* buf, uint32_t size);

  Item(const uint8_t* ptr, uint32_t size) noexcept
      : count(0), size_(size), data_(ptr), hash_(_hash()) {
  }

  size_t _hash() const noexcept {
    size_t ret = 0;
    const uint8_t* dp = data_;
    for(uint32_t sz = size_; sz; --sz, ++dp)
      ret += (ret << 3) + *dp;
    return ret;
    //return std::hash<std::string_view>{}(
    //  std::string_view((const char*)data_, size_));
  }

  void allocate() {
    data_ = static_cast<uint8_t*>(memcpy(new uint8_t[size_], data_, size_));
  }

  ~Item() {
    if(data_)
      delete[] data_;
  }

  uint32_t size() const noexcept {
    return size_;
  }

  const uint8_t* data() const noexcept {
    return data_;
  }

  size_t hash() const noexcept {
    return hash_;
  }

  Ptr use() noexcept {
    count.fetch_add(1);
    return this;
  }

  bool unused() noexcept {
    return count.fetch_sub(1) == 1;
  }

  void release();

  std::string_view to_string() const noexcept {
    return std::string_view(reinterpret_cast<const char*>(data_), size_);
  }

  bool less(const Item& other) const noexcept {
    return size_ < other.size_ ||
           (size_ == other.size() &&
            Condition::mem_cmp(data_, other.data(), size_) < 0);
  }

  bool equal(const Item& other) const noexcept {
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
class Page final : public PageBase {
  //public std::set<Item::Ptr, Item::Less> {
  public:
  Page() : PageBase(8) { }

  //~Page() { }

  Item::Ptr use(const uint8_t* buf, uint32_t size) {
    Core::ScopedLock lock(m_mutex);

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
    Core::ScopedLock lock(m_mutex);

    auto it = find(ptr);
    if(it != end() && !ptr->count) {
      erase(it);
      delete ptr;
    }
  }

  size_t count() const {
    Core::ScopedLock lock(m_mutex);

    size_t sz = size();
    //for(Page* nxt = m_page; nxt ; nxt->next_page(nxt))
    //  sz += nxt->count();
    return sz;
  }

  /*
  void next_page(Page*& page) const {
    Core::ScopedLock lock(m_mutex);
    page = m_page;
  }
  */

  private:
  mutable std::mutex            m_mutex;
  //Page*                         m_page = nullptr; // upper ranges

};

/* Vector with narrow on hash
typedef std::vector<Item::Ptr> PageBase;
class Page final : public PageBase {
  //public std::set<Item::Ptr, Item::Less> {
  public:


  Item::Ptr use(const uint8_t* buf, uint32_t sz) {
    Core::ScopedLock lock(m_mutex);

    auto ptr = new Item(buf, sz);

    auto it = begin() + _narrow(*ptr);
    for(; it != end() && (*it)->hash() <= ptr->hash(); ++it) {
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
    Core::ScopedLock lock(m_mutex);

    for(auto it = begin() + _narrow(*ptr);
        it != end() && (*it)->hash() <= ptr->hash(); ++it) {
      if((*it)->hash() == ptr->hash() && (*it)->equal(*ptr)) {
        delete *it;
        erase(it);
        return;
      }
    }
    SWC_ASSERT(!ptr);
  }

  size_t count() const {
    Core::ScopedLock lock(m_mutex);
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
      if(!(sz >>= 1))
        ++sz;
      if(offset < sz) {
        offset = 0;
        break;
      }
      offset -= sz;
    }
    return offset;
  }

  mutable std::mutex            m_mutex;
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

  size_t count() const {
    size_t sz = 0;
    for(auto& p : _pages)
      sz += p.count();
    return sz;
  }

  size_t pages() const {
    return 256;
  }

  const Page& page(uint8_t idx) const {
    return _pages[idx];
  }

  private:
  Page  _pages[256];
};

} }}




namespace SWC { namespace Env {
  SWC::Core::Mem::Arena PageArena;
} }



namespace SWC { namespace Core { namespace Mem {



Item::Ptr Item::make(const uint8_t* buf, uint32_t size) {
  return Env::PageArena.use(buf, size);
}

void Item::release() {
  if(unused())
    Env::PageArena.free(this);
}





struct ItemPtr final { // Item as SmartPtr

  ItemPtr() : ptr(nullptr) { }

  ItemPtr(const ItemPtr& other) noexcept
          : ptr(other.ptr ? other.ptr->use() : nullptr) {
  }

  ItemPtr(ItemPtr&& other) noexcept : ptr(other.ptr) {
    other.ptr = nullptr;
  }

  ItemPtr(const uint8_t* buf, uint32_t size)
          : ptr(Item::make(buf, size)) {
  }

  ItemPtr operator=(const ItemPtr& other) {
    ptr = other.ptr->use();
    return *this;
  }

  ItemPtr operator=(ItemPtr&& other) {
    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  ~ItemPtr() {
    if(ptr)
      ptr->release();
  }

  void release() {
    if(ptr) {
      ptr->release();
      ptr = nullptr;
    }
  }

  void use(const ItemPtr& other) {
    if(ptr)
      ptr->release();
    ptr = other.ptr->use();
  }

  void use(const uint8_t* buf, uint32_t size) {
    if(ptr)
      ptr->release();
    ptr = Item::make(buf, size);
  }

  const uint8_t* data() const {
    return ptr->data();
  }

  uint32_t size() const {
    return ptr->size();
  }

  std::string_view to_string() const {
    return ptr->to_string();
  }

  mutable Item::Ptr ptr = nullptr;
};

bool operator==(ItemPtr l, ItemPtr r) {
  return l.ptr == r.ptr;
}



}}}


#endif // swcdb_core_PageArena_h
