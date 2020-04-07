/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_BitFieldInt_H
#define swc_core_BitFieldInt_H

namespace SWC {

#pragma pack(push, 1)
template <class T, uint8_t SZ>
struct BitFieldInt final {

  T data : SZ;

  BitFieldInt<T, SZ>() { }

  template<typename FROM_T>
  BitFieldInt<T, SZ>(const FROM_T& v) : data(v) { }


  BitFieldInt<T, SZ>& operator++() {
    ++data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator++(int) {
    ++data;
    return *this;
  }

  BitFieldInt<T, SZ>& operator--() {
    --data;
    return *this;
  }
  BitFieldInt<T, SZ>& operator--(int) {
    --data;
    return *this;
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

  template<typename TO_T>
  operator TO_T() const {
    return data;
  } 

  operator BitFieldInt<uint32_t, 24>() const {
    return *this;
  } 

  operator BitFieldInt<int32_t, 24>() const {
    return *this;
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

  operator bool() const {
    return data;
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

};

template<typename T, uint8_t SZ>
inline std::ostream& operator<<(std::ostream& out, const BitFieldInt<T, SZ>& v) {
  out << v.data;
  return out;
}


typedef BitFieldInt<uint32_t, 24> uint24_t;
typedef BitFieldInt<int32_t, 24>  int24_t;


template<>
inline uint24_t::operator int24_t() const {
  return int24_t(data);
}
template<>
inline int24_t::operator uint24_t() const {
  return uint24_t(data);
}

inline std::string to_string(const uint24_t& v) {
  return std::to_string(v.data);
}
inline std::string to_string(const int24_t& v) {
  return std::to_string(v.data);
}


#pragma pack(pop)
} // namespace SWC




typedef SWC::uint24_t uint24_t;
typedef SWC::int24_t  int24_t;

static const uint24_t UINT24_MAX = UINT32_MAX >> 1; // 16777215
static const uint24_t UINT24_MIN = UINT24_MAX + 1;  // 0 
static const int24_t   INT24_MAX = UINT24_MAX / 2;  // 8388607
static const int24_t   INT24_MIN = INT24_MAX + 1;   // -8388608


#endif
