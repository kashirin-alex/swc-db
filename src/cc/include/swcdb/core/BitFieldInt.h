/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_BitFieldInt_h
#define swcdb_core_BitFieldInt_h

namespace SWC { namespace Core {


//#pragma pack(push, 1)
template <class T, uint8_t SZ>
struct BitFieldInt final {

  T data : SZ;

  constexpr SWC_CAN_INLINE
  BitFieldInt() noexcept { }

  template<typename FROM_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt(const FROM_T& v) noexcept : data(v) { }

  constexpr SWC_CAN_INLINE
  operator bool() const noexcept {
    return data;
  }

  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator++() noexcept {
    ++data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ> operator++(int) noexcept {
    BitFieldInt<T, SZ> tmp(data);
    ++data;
    return tmp;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator--() noexcept {
    --data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ> operator--(int) noexcept {
    BitFieldInt<T, SZ> tmp(data);
    --data;
    return tmp;
  }


  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator+=(const BitFieldInt<T, SZ>& v) noexcept {
    data += v.data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator-=(const BitFieldInt<T, SZ>& v) noexcept {
    data -= v.data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator/=(const BitFieldInt<T, SZ>& v) noexcept {
    data /= v.data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator*=(const BitFieldInt<T, SZ>& v) noexcept {
    data *= v.data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator>>=(const BitFieldInt<T, SZ>& v) noexcept {
    data >>= v.data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator<<=(const BitFieldInt<T, SZ>& v) noexcept {
    data <<= v.data;
    return *this;
  }
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator|=(const BitFieldInt<T, SZ>& v) noexcept {
    data |= v.data;
    return *this;
  }


  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator+=(const IN_T& v) noexcept {
    data += v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator-=(const IN_T& v) noexcept {
    data -= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator/=(const IN_T& v) noexcept {
    data /= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator*=(const IN_T& v) noexcept {
    data *= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator<<=(const IN_T& v) noexcept {
    data <<= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator>>=(const IN_T& v) noexcept {
    data >>= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator|=(const IN_T& v) noexcept {
    data |= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator&=(const IN_T& v) noexcept {
    data &= v;
    return *this;
  }
  template<typename IN_T>
  constexpr SWC_CAN_INLINE
  BitFieldInt<T, SZ>& operator^=(const IN_T& v) noexcept {
    data ^= v;
    return *this;
  }

  template<typename TO_T>
  constexpr SWC_CAN_INLINE
  operator TO_T() const noexcept {
    return data;
  }
  template<class T2, uint8_t SZ2>
  constexpr SWC_CAN_INLINE
  operator BitFieldInt<T2, SZ2>() const noexcept {
    return BitFieldInt<T2, SZ2>(data);
  }

  /*
  BitFieldInt<T, SZ> operator+(const BitFieldInt<T, SZ>& v) const {
    return BitFieldInt<T, SZ>(data) += v.data;
  }
  BitFieldInt<T, SZ> operator-(const BitFieldInt<T, SZ>& v) const {
    return BitFieldInt<T, SZ>(data) -= v.data;
  }
  BitFieldInt<T, SZ> operator/(const BitFieldInt<T, SZ>& v) const {
    return BitFieldInt<T, SZ>(data) /= v.data;
  }
  BitFieldInt<T, SZ> operator*(const BitFieldInt<T, SZ>& v) const {
    return BitFieldInt<T, SZ>(data) *= v.data;
  }
  BitFieldInt<T, SZ> operator>>(const BitFieldInt<T, SZ>& v) const {
    return BitFieldInt<T, SZ>(data) >>= v.data;
  }
  BitFieldInt<T, SZ> operator<<(const BitFieldInt<T, SZ>& v) const {
    return BitFieldInt<T, SZ>(data) <<= v.data;
  }


  template<typename IN_T>
  BitFieldInt<T, SZ> operator+(const IN_T& v) const {
    return BitFieldInt<T, SZ>(data) += v;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ> operator-(const IN_T& v) const {
    return BitFieldInt<T, SZ>(data) -= v;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ> operator/(const IN_T& v) const {
    return BitFieldInt<T, SZ>(data) /= v;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ> operator*(const IN_T& v) const {
    return BitFieldInt<T, SZ>(data) *= v;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ> operator<<(const IN_T& v) const {
    return BitFieldInt<T, SZ>(data) <<= v;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ> operator>>(const IN_T& v) const {
    return BitFieldInt<T, SZ>(data) >>= v;
  }


  bool operator <=(const BitFieldInt<T, SZ>& v) const {
    return data <= v.data;
  }
  bool operator >=(const BitFieldInt<T, SZ>& v) const {
    return data >= v.data;
  }
  bool operator <(const BitFieldInt<T, SZ>& v) const {
    return data < v.data;
  }
  bool operator >(const BitFieldInt<T, SZ>& v) const {
    return data > v.data;
  }
  bool operator ==(const BitFieldInt<T, SZ>& v) const {
    return data == v.data;
  }
  bool operator !=(const BitFieldInt<T, SZ>& v) const {
    return data != v.data;
  }


  template<typename IN_T>
  bool operator <=(const IN_T& v) const {
    return data <= v;
  }
  template<typename IN_T>
  bool operator >=(const IN_T& v) const {
    return data >= v;
  }
  template<typename IN_T>
  bool operator <(const IN_T& v) const {
    return data < v;
  }
  template<typename IN_T>
  bool operator >(const IN_T& v) const {
    return data > v;
  }
  template<typename IN_T>
  bool operator ==(const IN_T& v) const {
    return data == v;
  }
  template<typename IN_T>
  bool operator !=(const IN_T& v) const {
    return data != v;
  }
  */

} __attribute__((packed));
//#pragma pack(pop)


template<typename T, uint8_t SZ>
extern SWC_CAN_INLINE
std::ostream& operator<<(std::ostream& out, const BitFieldInt<T, SZ>& v) {
  out << v.data;
  return out;
}

template<class T, uint8_t SZ>
extern SWC_CAN_INLINE
std::string to_string(const BitFieldInt<T, SZ>& v) {
  return std::to_string(v.data);
}


/* BitFieldInt<T, SZ> = BitFieldInt<T, SZ> op IN_T */
#define SWC_BITFIELD_op_T(_op_) \
template<class T, uint8_t SZ, typename IN_T> \
extern constexpr SWC_CAN_INLINE \
BitFieldInt<T, SZ> \
operator _op_ (const BitFieldInt<T, SZ>& v1, \
               const IN_T& v2) noexcept { \
  return BitFieldInt<T, SZ>(v1.data _op_ v2);\
}
SWC_BITFIELD_op_T(+)
SWC_BITFIELD_op_T(-)
SWC_BITFIELD_op_T(/)
SWC_BITFIELD_op_T(*)
SWC_BITFIELD_op_T(<<)
SWC_BITFIELD_op_T(>>)
SWC_BITFIELD_op_T(&)
SWC_BITFIELD_op_T(|)
SWC_BITFIELD_op_T(^)

/* IN_T = IN_T op BitFieldInt<T, SZ>*/
#define SWC_T_op_BITFIELD(_op_) \
template<typename IN_T, class T, uint8_t SZ> \
extern constexpr SWC_CAN_INLINE \
BitFieldInt<T, SZ> \
operator _op_ (const IN_T& v1, \
               const BitFieldInt<T, SZ>& v2) noexcept { \
  return v1 _op_ v2.data;\
}
SWC_T_op_BITFIELD(+)
SWC_T_op_BITFIELD(-)
SWC_T_op_BITFIELD(/)
SWC_T_op_BITFIELD(*)
SWC_T_op_BITFIELD(<<)
SWC_T_op_BITFIELD(>>)
SWC_T_op_BITFIELD(&)
SWC_T_op_BITFIELD(|)
SWC_T_op_BITFIELD(^)



/* BitFieldInt<T, SZ> = BitFieldInt<T1, SZ1> op BitFieldInt<T2, SZ2> */
#define SWC_BITFIELD1_op_BITFIELD2(_op_) \
template<class T1, uint8_t SZ1, class T2, uint8_t SZ2> \
extern constexpr SWC_CAN_INLINE \
BitFieldInt<T1, SZ1> \
operator _op_ (const BitFieldInt<T1, SZ1>& v1, \
               const BitFieldInt<T2, SZ2>& v2) noexcept { \
  return BitFieldInt<T1, SZ1>(v1.data _op_ v2.data); \
}
SWC_BITFIELD1_op_BITFIELD2(+)
SWC_BITFIELD1_op_BITFIELD2(-)
SWC_BITFIELD1_op_BITFIELD2(/)
SWC_BITFIELD1_op_BITFIELD2(*)
SWC_BITFIELD1_op_BITFIELD2(<<)
SWC_BITFIELD1_op_BITFIELD2(>>)
SWC_BITFIELD1_op_BITFIELD2(&)
SWC_BITFIELD1_op_BITFIELD2(|)
SWC_BITFIELD1_op_BITFIELD2(^)



/* bool IN_T vs BitFieldInt<T, SZ> */
#define SWC_T_vs_BITFIELD(_vs_) \
template<typename IN_T, class T, uint8_t SZ> \
extern constexpr SWC_CAN_INLINE \
bool operator _vs_ (const IN_T& v1, const BitFieldInt<T, SZ>& v2) noexcept { \
  return v1 _vs_ v2.data; \
}
SWC_T_vs_BITFIELD(==)
SWC_T_vs_BITFIELD(!=)
SWC_T_vs_BITFIELD(<=)
SWC_T_vs_BITFIELD(>=)
SWC_T_vs_BITFIELD(<)
SWC_T_vs_BITFIELD(>)



/* bool BitFieldInt<T, SZ> vs IN_T */
#define SWC_BITFIELD_vs_T(_vs_) \
template<class T, uint8_t SZ, typename IN_T> \
extern constexpr SWC_CAN_INLINE \
bool operator _vs_ (const BitFieldInt<T, SZ>& v1, const IN_T& v2) noexcept { \
  return v1.data _vs_ v2; \
}
SWC_BITFIELD_vs_T(==)
SWC_BITFIELD_vs_T(!=)
SWC_BITFIELD_vs_T(<=)
SWC_BITFIELD_vs_T(>=)
SWC_BITFIELD_vs_T(<)
SWC_BITFIELD_vs_T(>)



/* bool BitFieldInt<T1, SZ1> vs BitFieldInt<T2, SZ2> */
#define SWC_BITFIELD1_vs_BITFIELD2(_vs_) \
template<class T1, uint8_t SZ1, class T2, uint8_t SZ2> \
extern constexpr SWC_CAN_INLINE \
bool operator _vs_ (const BitFieldInt<T1, SZ1>& v1, \
                    const BitFieldInt<T2, SZ2>& v2) noexcept { \
  return v1.data _vs_ v2.data; \
}
SWC_BITFIELD1_vs_BITFIELD2(==)
SWC_BITFIELD1_vs_BITFIELD2(!=)
SWC_BITFIELD1_vs_BITFIELD2(<=)
SWC_BITFIELD1_vs_BITFIELD2(>=)
SWC_BITFIELD1_vs_BITFIELD2(<)
SWC_BITFIELD1_vs_BITFIELD2(>)




typedef BitFieldInt<uint32_t, 24> uint24_t;
typedef BitFieldInt<int32_t,  24> int24_t;

typedef BitFieldInt<uint64_t, 40> uint40_t;
typedef BitFieldInt<int64_t,  40> int40_t;

typedef BitFieldInt<uint64_t, 48> uint48_t;
typedef BitFieldInt<int64_t,  48> int48_t;

typedef BitFieldInt<uint64_t, 56> uint56_t;
typedef BitFieldInt<int64_t,  56> int56_t;


} // namespace Core



/*!
 *  \addtogroup Core
 *  @{
 */

typedef Core::uint24_t uint24_t;
typedef Core::int24_t  int24_t;
constexpr const uint24_t UINT24_MAX = uint24_t(UINT32_MAX >> 8);   // 16777215
constexpr const uint24_t UINT24_MIN = UINT24_MAX + 1;              // 0
constexpr const int24_t   INT24_MAX = int24_t(UINT24_MAX / 2);     // 8388607
constexpr const int24_t   INT24_MIN = INT24_MAX + 1;               // -8388608


typedef Core::uint40_t uint40_t;
typedef Core::int40_t  int40_t;
constexpr const uint40_t UINT40_MAX = uint40_t(UINT64_MAX >> 24);  // 1099511627775
constexpr const uint40_t UINT40_MIN = UINT40_MAX + 1;              // 0
constexpr const int40_t   INT40_MAX = int40_t(UINT40_MAX / 2);     // 549755813887
constexpr const int40_t   INT40_MIN = INT40_MAX + 1;               // -549755813888


typedef Core::uint48_t uint48_t;
typedef Core::int48_t  int48_t;
constexpr const uint48_t UINT48_MAX = uint48_t(UINT64_MAX >> 16);  // 281474976710655
constexpr const uint48_t UINT48_MIN = UINT48_MAX + 1;              // 0
constexpr const int48_t   INT48_MAX = int48_t(UINT48_MAX / 2);     // 140737488355327
constexpr const int48_t   INT48_MIN = INT48_MAX + 1;               // -140737488355328


typedef Core::uint56_t uint56_t;
typedef Core::int56_t  int56_t;
constexpr const uint56_t UINT56_MAX = uint56_t(UINT64_MAX >> 8);   // 72057594037927935
constexpr const uint56_t UINT56_MIN = UINT56_MAX + 1;              // 0
constexpr const int56_t   INT56_MAX = int56_t(UINT56_MAX / 2);     // 36028797018963967
constexpr const int56_t   INT56_MIN = INT56_MAX + 1;               // -36028797018963968


/*! @} End of Core Group*/


} // namespace SWC

#endif // swcdb_core_BitFieldInt_h
