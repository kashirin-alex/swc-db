/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Time.h"


namespace SWC {



template<typename MapT, typename T = typename MapT::value_type>
void test_map(const typename MapT::size_type _sz) {
  const size_t sz = _sz;

  // operator[]
  {
  Time::Measure_ns track;
  MapT map;
  for(size_t n=sz; n; --n)
    map[n] = n;
  uint64_t took = track.elapsed();
  std::cout << typeid(MapT).name() << " operator[]-w: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }

  // emplace
  {
  Time::Measure_ns track;
  MapT map;
  for(size_t n=sz; n; --n)
      map.emplace(n, n);
  uint64_t took = track.elapsed();
  std::cout << typeid(MapT).name() << " emplace: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }


  // find
  {
  MapT map;
  for(size_t n=sz; n; --n)
    map.emplace(n, n);
  Time::Measure_ns track;
  for(size_t n=sz; n; --n) {
    SWC_ASSERT(map.find(n) != map.cend());
  }
  uint64_t took = track.elapsed();
  std::cout << typeid(MapT).name() << " find: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }

  // operator[]
  {
  MapT map;
  for(size_t n=sz; n; --n)
    map.emplace(n, n);
  Time::Measure_ns track;
  for(size_t n=sz; n; --n) {
    SWC_ASSERT(map[n] == n);
  }
  uint64_t took = track.elapsed();
  std::cout << typeid(MapT).name() << " operator[]-r: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }

}




template<typename MapT>
void test_map(size_t sz, uint8_t probes) {
  std::cout << std::endl << "Testing with size=" << sz << std::endl;

  Time::Measure_ns track;
  for(uint8_t n=0; n<probes; ++n)
    SWC::test_map<MapT>(sz);
  uint64_t took = track.elapsed();

  std::cout << typeid(MapT).name()
            << " sizeof=" << sizeof(MapT)
            << " elapse=" << took
            << " avg=" << (took/sz) << std::endl;
}


}

struct Type1 {
  Type1() noexcept { }
  Type1(size_t a_sz) : sz(a_sz), n(std::to_string(sz)) {
    n.reserve(32);
  }
  Type1(const Type1& other) : sz(other.sz), n(other.n) {
    n.reserve(32);
  }
  Type1(Type1&& other) noexcept : sz(other.sz), n(std::move(other.n)) { }

  Type1& operator=(const Type1& other) {
    n = other.n;
    sz = other.sz;
    return *this;
  }
  Type1& operator=(Type1&& other) noexcept {
    n = std::move(other.n);
    sz = other.sz;
    return *this;
  }

  Type1& operator=(size_t _sz) {
    sz = _sz;
    n = std::to_string(sz);
    n.reserve(32);
    return *this;
  }

  Type1& operator--() {
    n = std::to_string(--sz);
    n.reserve(32);
    return *this;
  }

  Type1& operator++() {
    n = std::to_string(++sz);
    n.reserve(32);
    return *this;
  }

  bool operator==(const Type1& other) const noexcept {
    return sz == other.sz && n.compare(other.n) == 0;
  }
  bool operator!=(const Type1& other) const noexcept {
    return !operator==(other);
  }
  bool operator>(const Type1& other) const noexcept {
    return sz > other.sz;
  }
  bool operator<(const Type1& other) const noexcept {
    return sz < other.sz;
  }

  bool operator==( size_t other) const noexcept {
    return sz == other;
  }
  bool operator<( size_t other) const noexcept {
    return sz < other;
  }
  bool operator>( size_t other) const noexcept {
    return sz > other;
  }

  std::ostream& operator<<(std::ostream& out) const {
    out << "Type1(n=" << n << " sz=" << sz << ')';
    return out;
  }

  const char* to_str() const noexcept {
    return n.c_str();
  }

  operator bool() const noexcept {
    return bool(sz);
  }

  size_t sz;
  std::string n;
};

std::ostream& operator<<(std::ostream& out, const Type1& v) {
  return v.operator<<(out);
}



struct Type2 {

  Type2() noexcept { }
  Type2(size_t sz) : ptr(new Type1(sz)) { }
  Type2(const Type2& other) : ptr(new Type1(*other.ptr.get())) { }
  Type2(Type2&& other) noexcept : ptr(std::move(other.ptr)) { }

  Type2& operator=(const Type2& other) {
    ptr.reset(new Type1(*other.ptr.get()));
    return *this;
  }
  Type2& operator=(Type2&& other) {
    ptr = std::move(other.ptr);
    return *this;
  }

  Type2& operator=(size_t _sz) {
    ptr.reset(new Type1(_sz));
    return *this;
  }

  Type2& operator--() {
    --(*ptr.get());
    return *this;
  }

  Type2& operator++() {
    ++(*ptr.get());
    return *this;
  }



  bool operator==(const Type2& other) const noexcept {
    return *ptr.get() == *other.ptr.get();
  }
  bool operator!=(const Type2& other) const noexcept {
    return !operator==(other);
  }
  bool operator>(const Type2& other) const noexcept {
    return *ptr.get() > *other.ptr.get();
  }
  bool operator<(const Type2& other) const noexcept {
    return *ptr.get() < *other.ptr.get();
  }

  bool operator==( size_t other) const noexcept {
    return *ptr.get() == other;
  }
  bool operator<( size_t other) const noexcept {
    return *ptr.get() < other;
  }
  bool operator>( size_t other) const noexcept {
    return *ptr.get() > other;
  }

  std::ostream& operator<<(std::ostream& out) const {
    return out << *ptr.get();
  }

  const char* to_str() const noexcept {
    return ptr->to_str();
  }

  operator bool() const noexcept {
    return bool(*ptr.get());
  }

  std::unique_ptr<Type1> ptr;
};


int main() {
  uint8_t probes = 3;


  // uint64_t map
  SWC::test_map< std::unordered_map<uint64_t, uint64_t> >(UINT8_MAX, probes);
  SWC::test_map< std::unordered_map<uint64_t, Type1>    >(UINT8_MAX, probes);
  SWC::test_map< std::unordered_map<uint64_t, Type2>    >(UINT8_MAX, probes);


  SWC::test_map< std::unordered_map<uint64_t, uint64_t> >(UINT16_MAX, probes);
  SWC::test_map< std::unordered_map<uint64_t, Type1>    >(UINT16_MAX, probes);
  SWC::test_map< std::unordered_map<uint64_t, Type2>    >(UINT16_MAX, probes);


  SWC::test_map< std::unordered_map<uint64_t, uint64_t> >(10000000, probes);
  SWC::test_map< std::unordered_map<uint64_t, Type1>    >(10000000, probes);
  SWC::test_map< std::unordered_map<uint64_t, Type2>    >(10000000, probes);

  // ? diff - unordered faster by~ 1/3
  SWC::test_map< std::unordered_map<uint64_t, uint64_t> >(1000000, probes);
  SWC::test_map< std::map<uint64_t, uint64_t>           >(1000000, probes);


  std::cout << " OK! \n";
  return 0;
}
