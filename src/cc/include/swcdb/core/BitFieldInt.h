/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_BitFieldInt_H
#define swc_core_BitFieldInt_H

namespace SWC {


//#pragma pack(push, 1)
template <class T, uint8_t SZ>
struct BitFieldInt final {

  T data : SZ;

  BitFieldInt<T, SZ>() { }

  ~BitFieldInt<T, SZ>() { }
  
  operator bool() const {
    return data;
  }

  template<typename FROM_T>
  BitFieldInt<T, SZ>(const FROM_T& v) : data(v) { }


  BitFieldInt<T, SZ>& operator++() {
    ++data;
    return *this;
  }
  BitFieldInt<T, SZ> operator++(int) {
    BitFieldInt<T, SZ> tmp(data);
    ++data;
    return tmp;
  }
  BitFieldInt<T, SZ>& operator--() {
    --data;
    return *this;
  }
  BitFieldInt<T, SZ> operator--(int) {
    BitFieldInt<T, SZ> tmp(data);
    --data;
    return tmp;
  }

  
  BitFieldInt<T, SZ>& operator+=(const BitFieldInt<T, SZ>& v) {
    data += v.data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator-=(const BitFieldInt<T, SZ>& v) {
    data -= v.data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator/=(const BitFieldInt<T, SZ>& v) {
    data /= v.data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator*=(const BitFieldInt<T, SZ>& v) {
    data *= v.data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator>>=(const BitFieldInt<T, SZ>& v) {
    data >>= v.data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator<<=(const BitFieldInt<T, SZ>& v) {
    data <<= v.data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator|=(const BitFieldInt<T, SZ>& v) {
    data |= v.data;
    return *this;
  }


  template<typename IN_T>
  BitFieldInt<T, SZ>& operator+=(const IN_T& v) {
    data += v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator-=(const IN_T& v) {
    data -= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator/=(const IN_T& v) {
    data /= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator*=(const IN_T& v) {
    data *= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator<<=(const IN_T& v) {
    data <<= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator>>=(const IN_T& v) {
    data >>= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator|=(const IN_T& v) {
    data |= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator&=(const IN_T& v) {
    data &= v;
    return *this;
  }
  template<typename IN_T>
  BitFieldInt<T, SZ>& operator^=(const IN_T& v) {
    data ^= v;
    return *this;
  }

  template<typename TO_T>
  operator TO_T() const {
    return data;
  }
  template<class T2, uint8_t SZ2>
  operator BitFieldInt<T2, SZ2>() const {
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
extern SWC_CAN_INLINE \
BitFieldInt<T, SZ> \
operator _op_ (const BitFieldInt<T, SZ>& v1, \
               const IN_T& v2) { \
  return BitFieldInt<T, SZ>(v1.data _op_ v2);\
}
SWC_BITFIELD_op_T(+);
SWC_BITFIELD_op_T(-);
SWC_BITFIELD_op_T(/);
SWC_BITFIELD_op_T(*);
SWC_BITFIELD_op_T(<<);
SWC_BITFIELD_op_T(>>);
SWC_BITFIELD_op_T(&);
SWC_BITFIELD_op_T(|);
SWC_BITFIELD_op_T(^);

/* IN_T = IN_T op BitFieldInt<T, SZ>*/
#define SWC_T_op_BITFIELD(_op_) \
template<typename IN_T, class T, uint8_t SZ> \
extern SWC_CAN_INLINE \
BitFieldInt<T, SZ> \
operator _op_ (const IN_T& v1, \
               const BitFieldInt<T, SZ>& v2) { \
  return v1 _op_ v2.data;\
}
SWC_T_op_BITFIELD(+);
SWC_T_op_BITFIELD(-);
SWC_T_op_BITFIELD(/);
SWC_T_op_BITFIELD(*);
SWC_T_op_BITFIELD(<<);
SWC_T_op_BITFIELD(>>);
SWC_T_op_BITFIELD(&);
SWC_T_op_BITFIELD(|);
SWC_T_op_BITFIELD(^);



/* BitFieldInt<T, SZ> = BitFieldInt<T1, SZ1> op BitFieldInt<T2, SZ2> */
#define SWC_BITFIELD1_op_BITFIELD2(_op_) \
template<class T1, uint8_t SZ1, class T2, uint8_t SZ2> \
extern SWC_CAN_INLINE \
BitFieldInt<T1, SZ1> \
operator _op_ (const BitFieldInt<T1, SZ1>& v1, \
               const BitFieldInt<T2, SZ2>& v2) { \
  return BitFieldInt<T1, SZ1>(v1.data _op_ v2.data); \
}
SWC_BITFIELD1_op_BITFIELD2(+);
SWC_BITFIELD1_op_BITFIELD2(-);
SWC_BITFIELD1_op_BITFIELD2(/);
SWC_BITFIELD1_op_BITFIELD2(*);
SWC_BITFIELD1_op_BITFIELD2(<<);
SWC_BITFIELD1_op_BITFIELD2(>>);
SWC_BITFIELD1_op_BITFIELD2(&);
SWC_BITFIELD1_op_BITFIELD2(|);
SWC_BITFIELD1_op_BITFIELD2(^);



/* bool IN_T vs BitFieldInt<T, SZ> */
#define SWC_T_vs_BITFIELD(_vs_) \
template<typename IN_T, class T, uint8_t SZ> \
extern SWC_CAN_INLINE \
bool operator _vs_ (const IN_T& v1, const BitFieldInt<T, SZ>& v2) { \
  return v1 _vs_ v2.data; \
}
SWC_T_vs_BITFIELD(==);
SWC_T_vs_BITFIELD(!=);
SWC_T_vs_BITFIELD(<=);
SWC_T_vs_BITFIELD(>=);
SWC_T_vs_BITFIELD(<);
SWC_T_vs_BITFIELD(>);



/* bool BitFieldInt<T, SZ> vs IN_T */
#define SWC_BITFIELD_vs_T(_vs_) \
template<class T, uint8_t SZ, typename IN_T> \
extern SWC_CAN_INLINE \
bool operator _vs_ (const BitFieldInt<T, SZ>& v1, const IN_T& v2) { \
  return v1.data _vs_ v2; \
}
SWC_BITFIELD_vs_T(==);
SWC_BITFIELD_vs_T(!=);
SWC_BITFIELD_vs_T(<=);
SWC_BITFIELD_vs_T(>=);
SWC_BITFIELD_vs_T(<);
SWC_BITFIELD_vs_T(>);



/* bool BitFieldInt<T1, SZ1> vs BitFieldInt<T2, SZ2> */
#define SWC_BITFIELD1_vs_BITFIELD2(_vs_) \
template<class T1, uint8_t SZ1, class T2, uint8_t SZ2> \
extern SWC_CAN_INLINE \
bool operator _vs_ (const BitFieldInt<T1, SZ1>& v1, \
                    const BitFieldInt<T2, SZ2>& v2) { \
  return v1.data _vs_ v2.data; \
}
SWC_BITFIELD1_vs_BITFIELD2(==);
SWC_BITFIELD1_vs_BITFIELD2(!=);
SWC_BITFIELD1_vs_BITFIELD2(<=);
SWC_BITFIELD1_vs_BITFIELD2(>=);
SWC_BITFIELD1_vs_BITFIELD2(<);
SWC_BITFIELD1_vs_BITFIELD2(>);




typedef BitFieldInt<uint32_t, 24> uint24_t;
typedef BitFieldInt<int32_t,  24> int24_t;

typedef BitFieldInt<uint64_t, 40> uint40_t;
typedef BitFieldInt<int64_t,  40> int40_t;

typedef BitFieldInt<uint64_t, 48> uint48_t;
typedef BitFieldInt<int64_t,  48> int48_t;

typedef BitFieldInt<uint64_t, 56> uint56_t;
typedef BitFieldInt<int64_t,  56> int56_t;


} // namespace SWC




typedef SWC::uint24_t uint24_t;
typedef SWC::int24_t  int24_t;
static const uint24_t UINT24_MAX(UINT32_MAX >> 8);  // 16777215
static const uint24_t UINT24_MIN(UINT24_MAX + 1);   // 0 
static const int24_t   INT24_MAX(UINT24_MAX / 2);   // 8388607
static const int24_t   INT24_MIN(INT24_MAX + 1);    // -8388608


typedef SWC::uint40_t uint40_t;
typedef SWC::int40_t  int40_t;
static const uint40_t UINT40_MAX(UINT64_MAX >> 24); // 1099511627775
static const uint40_t UINT40_MIN(UINT40_MAX + 1);   // 0 
static const int40_t   INT40_MAX(UINT40_MAX / 2);   // 549755813887
static const int40_t   INT40_MIN(INT40_MAX + 1);    // -549755813888


typedef SWC::uint48_t uint48_t;
typedef SWC::int48_t  int48_t;
static const uint48_t UINT48_MAX(UINT64_MAX >> 16); // 281474976710655
static const uint48_t UINT48_MIN(UINT48_MAX + 1);   // 0 
static const int48_t   INT48_MAX(UINT48_MAX / 2);   // 140737488355327
static const int48_t   INT48_MIN(INT48_MAX + 1);    // -140737488355328


typedef SWC::uint56_t uint56_t;
typedef SWC::int56_t  int56_t;
static const uint56_t UINT56_MAX(UINT64_MAX >> 8); // 72057594037927935
static const uint56_t UINT56_MIN(UINT56_MAX + 1);   // 0 
static const int56_t   INT56_MAX(UINT56_MAX / 2);   // 36028797018963967
static const int56_t   INT56_MIN(INT56_MAX + 1);    // -36028797018963968


#endif
