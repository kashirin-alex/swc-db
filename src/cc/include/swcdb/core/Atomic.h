/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Atomic_h
#define swcdb_core_Atomic_h


#include <atomic>


namespace SWC { namespace Core {



template<typename T, std::memory_order OrderT=std::memory_order_relaxed>
struct AtomicBase : protected std::atomic<T> {

  constexpr SWC_CAN_INLINE
  explicit AtomicBase() noexcept { }

  template<typename ValueT>
  constexpr SWC_CAN_INLINE
  explicit AtomicBase(ValueT initial) noexcept : std::atomic<T>(initial) { }

  AtomicBase(const AtomicBase<T>&) = delete;

  AtomicBase(const AtomicBase<T>&&) = delete;

  template<typename ValueT> AtomicBase<T>& operator=(ValueT v) = delete;

  ~AtomicBase() noexcept { }


  constexpr SWC_CAN_INLINE
  void store(T v) noexcept {
    std::atomic<T>::store(v, OrderT);
  }

  constexpr SWC_CAN_INLINE
  T load() const noexcept {
    return std::atomic<T>::load(OrderT);
  }

  constexpr SWC_CAN_INLINE
  T exchange(T value) noexcept {
    return std::atomic<T>::exchange(value, OrderT);
  }

  constexpr SWC_CAN_INLINE
  bool compare_exchange_weak(T& at, T value) noexcept {
    return std::atomic<T>::compare_exchange_weak(at, value, OrderT);
  }

  constexpr SWC_CAN_INLINE
  operator T() const noexcept {
    return load();
  }

};

typedef AtomicBase<bool> AtomicBool;




template<typename T, std::memory_order OrderT=std::memory_order_relaxed>
struct Atomic : public AtomicBase<T> {

  constexpr SWC_CAN_INLINE
  explicit Atomic() noexcept { }

  template<typename ValueT>
  constexpr SWC_CAN_INLINE
  explicit Atomic(ValueT initial) noexcept : AtomicBase<T>(initial) { }

  Atomic(const Atomic<T>&) = delete;

  Atomic(const Atomic<T>&&) = delete;

  template<typename ValueT> Atomic<T>& operator=(ValueT v) = delete;

  ~Atomic() noexcept {}


  constexpr SWC_CAN_INLINE
  T fetch_sub(T v) noexcept {
    return AtomicBase<T>::fetch_sub(v, OrderT);
  }

  constexpr SWC_CAN_INLINE
  T fetch_add(T v) noexcept {
    return AtomicBase<T>::fetch_add(v, OrderT);
  }

  constexpr SWC_CAN_INLINE
  T fetch_xor(T v) noexcept {
    return AtomicBase<T>::fetch_xor(v, OrderT);
  }

  constexpr SWC_CAN_INLINE
  T fetch_and(T v) noexcept {
    return AtomicBase<T>::fetch_and(v, OrderT);
  }

  constexpr SWC_CAN_INLINE
  T fetch_or(T v) noexcept {
    return AtomicBase<T>::fetch_or(v, OrderT);
  }



  constexpr SWC_CAN_INLINE
  T sub_rslt(T v) noexcept {
    return fetch_sub(v) - v;
  }

  constexpr SWC_CAN_INLINE
  T add_rslt(T v) noexcept {
    return fetch_add(v) + v;
  }

  constexpr SWC_CAN_INLINE
  T xor_rslt(T v) noexcept {
    return fetch_xor(v) ^ v;
  }

  constexpr SWC_CAN_INLINE
  T and_rslt(T v) noexcept {
    return fetch_and(v) & v;
  }

  constexpr SWC_CAN_INLINE
  T or_rslt(T v) noexcept {
    return fetch_or(v) | v;
  }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

  constexpr SWC_CAN_INLINE
  T operator++(int) noexcept {
    return fetch_add(1);
  }

  constexpr SWC_CAN_INLINE
  T operator--(int) noexcept {
    return fetch_sub(1);
  }

#pragma GCC diagnostic pop


  /* Is/Should post OP value ever used ?

  constexpr SWC_CAN_INLINE
  T operator++() noexcept {
    return add_rslt(1);
  }
  constexpr SWC_CAN_INLINE
  T operator--() noexcept {
    return sub_rslt(1);
  }

  constexpr SWC_CAN_INLINE
  T operator+=(T v) noexcept {
    return add_rslt(v);
  }

  constexpr SWC_CAN_INLINE
  T operator-=(T v) noexcept {
    return sub_rslt(v);
  }

  constexpr SWC_CAN_INLINE
  T operator^=(T v) noexcept {
    return xor_rslt(v);
  }

  constexpr SWC_CAN_INLINE
  T operator&=(T v) noexcept {
    return and_rslt(v);
  }

  constexpr SWC_CAN_INLINE
  T operator|=(T v) noexcept {
    return or_rslt(v);
  }
  */

};



template<typename T, std::memory_order OrderT=std::memory_order_relaxed>
extern SWC_CAN_INLINE
std::ostream& operator<<(std::ostream& out, const AtomicBase<T, OrderT>& v) {
  return out << v.load();
}

}} // namespace SWC::Core


#endif // swcdb_core_Atomic_h
