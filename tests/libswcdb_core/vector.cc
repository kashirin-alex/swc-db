/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Vector.h"
#include <map>

namespace SWC {

/*
template<typename T>
void test_vector_huge(size_t sz) {
  uint64_t took;
  Time::Measure_ns track;

  /// Core::Vector<T>
  Core::Vector<T> swc_vec;
  for(T n=sz; n; --n)
    swc_vec.add(n);
  SWC_ASSERT(sz == swc_vec.size());
  took = track.elapsed();
  std::cout << " swc_vec add: elapse=" <<  took << " avg=" << (took/sz) << std::endl;


  track.restart();
  for(T n=1; n <= sz; ++n) {
    const T* p = swc_vec.cfind_(n);
    SWC_ASSERT(p && *p == n);
  }
  took = track.elapsed();
  std::cout << " swc_vec cfind_: elapse=" <<  took << " avg=" << (took/sz) << std::endl;


  track.restart();
  for(T n=1; n <= sz; ++n) {
    T* p = swc_vec.find(n);
    SWC_ASSERT(p && *p == n);
  }
  took = track.elapsed();
  std::cout << " swc_vec find: elapse=" <<  took << " avg=" << (took/sz) << std::endl;


  track.restart();
  for(T n=1; n <= sz; ++n) {
    const T* p = swc_vec.cfind(n);
    SWC_ASSERT(p && *p == n);
  }
  took = track.elapsed();
  std::cout << " swc_vec cfind: elapse=" <<  took << " avg=" << (took/sz) << std::endl;


  /// std::unordered_map
  track.restart();
  std::unordered_map<T, T> std_unomap;
  for(T n=sz; n; --n)
    std_unomap[n] = n;
  SWC_ASSERT(sz == std_unomap.size());
  took = track.elapsed();
  std::cout << " std_unomap add: elapse=" <<  took << " avg=" << (took/sz) << std::endl;

  track.restart();
  for(T n=1; n <= sz; ++n) {
    auto it = std_unomap.find(n);
    SWC_ASSERT(it != std_unomap.end() && it->second == n);
  }
  took = track.elapsed();
  std::cout << " std_unomap find: elapse=" <<  took << " avg=" << (took/sz) << std::endl;


  /// std::map
  track.restart();
  std::map<T, T> std_map;
  for(T n=sz; n; --n)
    std_map[n] = n;
  SWC_ASSERT(sz == std_map.size());
  took = track.elapsed();
  std::cout << " std_map add: elapse=" <<  took << " avg=" << (took/sz) << std::endl;

  track.restart();
  for(T n=1; n <= sz; ++n) {
    auto it = std_map.find(n);
    SWC_ASSERT(it != std_map.end() && it->second == n);
  }
  took = track.elapsed();
  std::cout << " std_map find: elapse=" <<  took << " avg=" << (took/sz) << std::endl;

}
*/

template<typename VecT, typename T = typename  VecT::value_type>
void test_check_eq(const VecT& vec, size_t sz) {
  auto it = vec.cbegin();
  for(T n=sz; n; --n, ++it) {
    if(n != *it) {
      std::cout << "n=" << n << " *it=" << *it << std::endl;
      SWC_ASSERT(n == *it);
    }
  }
}

template<typename VecT, typename T = typename  VecT::value_type>
void test_vector(const size_t sz) {

  // ctor arg
  {
  Time::Measure_ns track;
  VecT vec(sz, sz);
  uint64_t took = track.elapsed();
  SWC_ASSERT(sz == vec.size());
  for(auto& v : vec)
    SWC_ASSERT(v == sz);
  std::cout << typeid(VecT).name() << " arg ctor: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }


  // push_back
  {
  Time::Measure_ns track;
  VecT vec;
  for(T n=sz; n; --n)
    vec.push_back(n);
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " cp push_back: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }

  {
  Time::Measure_ns track;
  VecT vec;
  for(T n=sz; n; --n) {
    T v(n);
    vec.push_back(std::move(v));
  }
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " mv push_back: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }


  // emplace_back
  {
  Time::Measure_ns track;
  VecT vec;
  for(T n=sz; n; --n)
    vec.emplace_back(n);
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " cp emplace_back: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }

  {
  Time::Measure_ns track;
  VecT vec;
  for(T n=sz; n; --n) {
    T v(n);
    vec.emplace_back(std::move(v));
  }
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " mv emplace_back: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }


  // cp insert at it
  {
  Time::Measure_ns track;
  VecT vec;
  for(T n=0; n < sz; )
    vec.insert(vec.cbegin(), ++n);
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " cp insert it: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }

  // mv insert at it
  {
  Time::Measure_ns track;
  VecT vec;
  for(T n=0; n < sz; ) {
    T v(++n);
    vec.insert(vec.cbegin(), std::move(v));
    //std::cout << n << " ?= " << vec.front() << std::endl;
  }
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " mv insert it: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }


  // insert range at it
  {
  VecT vec1(sz);
  auto it = vec1.begin();
  for(T n=sz; n; --n, ++it)
    *it = n;
  SWC_ASSERT(sz == vec1.size());

  Time::Measure_ns track;
  {
  std::cout << typeid(VecT).name() << " range insert it(all) " << std::endl;
  VecT vec;
  vec.insert(vec.begin(), vec1.cbegin(), vec1.cend());
  SWC_ASSERT(sz == vec.size());
  test_check_eq(vec, sz);
  }
  {
  std::cout << typeid(VecT).name() << " range insert it(end()+1) " << std::endl;
  VecT vec(sz-1);
  auto it = vec.begin();
  T end(1);
  for(T n=sz; n > end; --n, ++it)
    *it = n;
  SWC_ASSERT(sz - 1 == vec.size());
  vec.insert(vec.end(), vec1.cend() - 1, vec1.cend());
  SWC_ASSERT(sz == vec.size());
  test_check_eq(vec, sz);
  }
  {
  std::cout << typeid(VecT).name() << " range insert it(offset=1) " << std::endl;
  VecT vec(1);
  *vec.begin() = sz;
  vec.insert(vec.begin() + 1, vec1.cbegin() + 1, vec1.cend());
  SWC_ASSERT(sz == vec.size());
  test_check_eq(vec, sz);
  }
  {
  std::cout << typeid(VecT).name() << " range insert it(offset=1,end()-1) " << std::endl;
  VecT vec(2);
  *vec.begin() = sz;
  *(vec.begin() + 1) = 1;
  vec.insert(vec.begin() + 1, vec1.cbegin() + 1, vec1.cend() - 1);
  SWC_ASSERT(sz == vec.size());
  test_check_eq(vec, sz);
  }
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " range insert it: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }


  // reserve & push_back
  {
  Time::Measure_ns track;
  VecT vec;
  vec.reserve(sz);

  uint64_t took_resv = track.elapsed();
  track.restart();
  for(T n=sz; n; --n)
    vec.push_back(n);
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " resv=" << took_resv << " push_back: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }

  // size ctor & push_back
  {
  Time::Measure_ns track;
  VecT vec(sz);
  auto it = vec.begin();
  for(T n=sz; n; --n, ++it)
    *it = n;
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " size ctor push_back: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }


  // assign
  {
  VecT vec1(sz);
  auto it = vec1.begin();
  for(T n=sz; n; --n, ++it)
    *it = n;
  SWC_ASSERT(sz == vec1.size());

  Time::Measure_ns track;
  VecT vec;
  vec.assign(vec1.begin(), vec1.end());
  SWC_ASSERT(sz == vec.size());
  uint64_t took = track.elapsed();
  std::cout << typeid(VecT).name() << " assign: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  test_check_eq(vec, sz);
  }


  // erase it
  {
  VecT vec(sz);
  auto it = vec.begin();
  for(T n=0; n < sz; ++n, ++it)
    *it = n;
  SWC_ASSERT(sz == vec.size());

  Time::Measure_ns track;
  vec.erase(vec.begin() + 4);
  SWC_ASSERT(sz - 1 == vec.size());
  uint64_t took = track.elapsed();

  it = vec.begin();
  for(T n=0; n < sz; ++n, ++it) {
    if(n == size_t(4)) {
      ++n;
      continue;
    }
    if(n != *it) {
      std::cout << "n=" << n << " *it=" << *it << std::endl;
      SWC_ASSERT(n == *it);
    }
  }
  std::cout << typeid(VecT).name() << " it erase: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }


  // erase range
  {
  VecT vec(sz);
  auto it = vec.begin();
  for(T n=0; n < sz; ++n, ++it)
    *it = n;
  SWC_ASSERT(sz == vec.size());

  Time::Measure_ns track;
  size_t removed = (vec.end()-3) - (vec.begin()+2);
  vec.erase(vec.begin()+2, vec.end()-3);
  SWC_ASSERT(sz - removed == vec.size());
  uint64_t took = track.elapsed();

  it = vec.begin();
  for(T n=0; n < sz; ++n, ++it) {
    if(n > size_t(1) || n < size_t(sz - 2)) {
      ++n;
      continue;
    }
    if(n != *it) {
      std::cout << "n=" << n << " *it=" << *it << " removed=" << removed << std::endl;
      SWC_ASSERT(n == *it);
    }
  }
  std::cout << typeid(VecT).name() << " range erase: elapse=" <<  took << " avg=" << (took/sz) << std::endl;
  }


}



template<typename VecT>
void test_vector(size_t sz, uint8_t probes) {
  std::cout << std::endl << "Testing with size=" << sz << std::endl;

  Time::Measure_ns track;
  for(uint8_t n=0; n<probes; ++n)
    SWC::test_vector<VecT>(sz);
  uint64_t took = track.elapsed();

  std::cout << typeid(VecT).name()
            << " sizeof=" << sizeof(VecT)
            << " elapse=" << took
            << " avg=" << (took/sz) << std::endl;
}

}

struct Type1 {
  Type1() noexcept { }
  Type1(size_t sz) : sz(sz), n(std::to_string(sz)) {
    n.reserve(32);
  }
  Type1(const Type1& other) : sz(other.sz), n(other.n) { }
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


  // uint64_t vector
  SWC::test_vector< std::vector<uint64_t>                       >(UINT8_MAX, probes);
  SWC::test_vector< SWC::Core::Vector<uint64_t, uint8_t, 127>   >(UINT8_MAX, probes);

  SWC::test_vector< std::vector<uint64_t>                       >(UINT16_MAX, probes);
  SWC::test_vector< SWC::Core::Vector<uint64_t, uint16_t, 4096> >(UINT16_MAX, probes);

  //SWC::test_vector< std::vector<uint64_t>                 >(SWC::UINT24_MAX, probes);
  //SWC::test_vector< SWC::Core::Vector<uint64_t, uint32_t> >(SWC::UINT24_MAX, probes);


  // Type1 vector
  SWC::test_vector< std::vector<Type1>                          >(UINT8_MAX, probes);
  SWC::test_vector< SWC::Core::Vector<Type1, uint8_t, 127>      >(UINT8_MAX, probes);

  SWC::test_vector< std::vector<Type1>                          >(UINT16_MAX/4, probes);
  SWC::test_vector< SWC::Core::Vector<Type1, uint16_t, 4096>    >(UINT16_MAX/4, probes);


  // Type2 vector
  SWC::test_vector< std::vector<Type2>                          >(UINT8_MAX, probes);
  SWC::test_vector< SWC::Core::Vector<Type2, uint8_t, 127>      >(UINT8_MAX, probes);

  SWC::test_vector< std::vector<Type2>                          >(UINT16_MAX/4, probes);
  SWC::test_vector< SWC::Core::Vector<Type2, uint16_t, 4096>    >(UINT16_MAX/4, probes);


  std::cout << " OK! \n";
  return 0;
}