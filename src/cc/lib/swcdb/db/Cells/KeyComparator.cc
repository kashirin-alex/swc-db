/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"


# define SWC_CAN_INLINE  \
  __attribute__((__always_inline__)) \
  static inline

#define SWC_NOINLINE  \
  __attribute__((__noinline__))


namespace SWC { namespace DB { namespace KeySeq {


///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE Condition::Comp 
condition(const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len);

template<> 
inline __attribute__((__always_inline__)) 
Condition::Comp 
condition<Types::KeySeq::BITWISE>(const uint8_t *p1, uint32_t p1_len, 
                                  const uint8_t *p2, uint32_t p2_len) {
  return Condition::condition_bitwise(p1, p1_len, p2, p2_len);
}

template<> 
inline __attribute__((__always_inline__)) 
Condition::Comp 
condition<Types::KeySeq::VOLUME>(const uint8_t *p1, uint32_t p1_len, 
                                 const uint8_t *p2, uint32_t p2_len) {
  return Condition::condition_volume(p1, p1_len, p2, p2_len);
}

SWC_NOINLINE Condition::Comp  
condition(const Types::KeySeq seq, 
          const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
    case Types::KeySeq::BITWISE_FCOUNT:
      return condition<Types::KeySeq::BITWISE>(p1, p1_len, p2, p2_len);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::VOLUME_FCOUNT:
      return condition<Types::KeySeq::VOLUME>(p1, p1_len, p2, p2_len);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///


///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE Condition::Comp 
compare(const Cell::Key& key,  const Cell::Key& other) {
  if(uint24_t min = key.count < other.count ? key.count : other.count) {
    const uint8_t* p1 = key.data;
    const uint8_t* p2 = other.data;
    uint24_t sz1;
    uint24_t sz2;
    for(Condition::Comp comp; min; --min, p1 += sz1, p2 += sz2) {
      if((comp = condition<T_seq>(
            p1, sz1 = Serialization::decode_vi24(&p1), 
            p2, sz2 = Serialization::decode_vi24(&p2) 
          )) != Condition::EQ)
        return comp;
    }
  }
  return key.count == other.count
        ? Condition::EQ
        : (key.count > other.count ? Condition::LT : Condition::GT);
}

template<> 
inline __attribute__((__always_inline__)) 
Condition::Comp 
compare<Types::KeySeq::BITWISE_FCOUNT>(const Cell::Key& key, 
                                       const Cell::Key& other) {
  if(key.count < other.count)
    return Condition::GT;
  if(key.count > other.count)
    return Condition::LT;
  return compare<Types::KeySeq::BITWISE>(key, other);
}

template<> 
inline __attribute__((__always_inline__)) 
Condition::Comp 
compare<Types::KeySeq::VOLUME_FCOUNT>(const Cell::Key& key, 
                                      const Cell::Key& other) {
  if(key.count < other.count)
    return Condition::GT;
  if(key.count > other.count)
    return Condition::LT;
  return compare<Types::KeySeq::VOLUME>(key, other);
}


SWC_NOINLINE Condition::Comp 
compare(const Types::KeySeq seq, const Cell::Key& key, 
                                 const Cell::Key& other) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
      return compare<Types::KeySeq::BITWISE>(key, other);

    case Types::KeySeq::VOLUME:
      return compare<Types::KeySeq::VOLUME>(key, other);

    case Types::KeySeq::BITWISE_FCOUNT:
      return compare<Types::KeySeq::BITWISE_FCOUNT>(key, other);

    case Types::KeySeq::VOLUME_FCOUNT:
      return compare<Types::KeySeq::VOLUME_FCOUNT>(key, other);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq>
SWC_CAN_INLINE Condition::Comp 
compare(const Cell::Key& key, const Cell::Key& other,
        uint32_t max, bool empty_ok, bool empty_eq) {
  if(uint24_t min = key.count < other.count ? key.count : other.count) {
    if(max && min > max)
      min = max;  
    const uint8_t* p1 = key.data;
    const uint8_t* p2 = other.data;
    uint24_t sz1;
    uint24_t sz2;
    for(Condition::Comp comp; min; --min, p1 += sz1, p2 += sz2) {
      sz2 = Serialization::decode_vi24(&p2);
      if(!(sz1 = Serialization::decode_vi24(&p1)) && empty_ok) {
        if(empty_eq)
          return Condition::EQ;
        continue;
      }
      if((comp = condition<T_seq>(p1, sz1, p2, sz2)) != Condition::EQ)
        return comp;
    }
  }
  return key.count != other.count && 
         (!max || max > key.count || max > other.count)
        ? key.count > other.count ? Condition::LT : Condition::GT
          : Condition::EQ;
}

template<> 
inline __attribute__((__always_inline__)) 
Condition::Comp 
compare<Types::KeySeq::BITWISE_FCOUNT>(
        const Cell::Key& key, const Cell::Key& other,
        uint32_t max, bool empty_ok, bool empty_eq) {
  if(!max || max > key.count || max > other.count ) {
    if(key.count < other.count)
      return Condition::GT;
    if(key.count > other.count)
      return Condition::LT;
  }
  return compare<Types::KeySeq::BITWISE>(key, other, max, empty_ok, empty_eq);
}

template<> 
inline __attribute__((__always_inline__)) 
Condition::Comp 
compare<Types::KeySeq::VOLUME_FCOUNT>(
        const Cell::Key& key, const Cell::Key& other,
        uint32_t max, bool empty_ok, bool empty_eq) {
  if(!max || max > key.count || max > other.count ) {
    if(key.count < other.count)
      return Condition::GT;
    if(key.count > other.count)
      return Condition::LT;
  }
  return compare<Types::KeySeq::VOLUME>(key, other, max, empty_ok, empty_eq);
}


SWC_NOINLINE Condition::Comp 
compare(const Types::KeySeq seq, const Cell::Key& key, const Cell::Key& other,
        uint32_t max, bool empty_ok, bool empty_eq) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
      return compare<Types::KeySeq::BITWISE>(
        key, other, max, empty_ok, empty_eq);

    case Types::KeySeq::VOLUME:
      return compare<Types::KeySeq::VOLUME>(
        key, other, max, empty_ok, empty_eq);

    case Types::KeySeq::BITWISE_FCOUNT:
      return compare<Types::KeySeq::BITWISE_FCOUNT>(
        key, other, max, empty_ok, empty_eq);

    case Types::KeySeq::VOLUME_FCOUNT:
      return compare<Types::KeySeq::VOLUME_FCOUNT>(
        key, other, max, empty_ok, empty_eq);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE bool
compare(const Cell::Key& key, const Cell::KeyVec& other, 
        Condition::Comp break_if, uint32_t max, bool empty_ok) {
  const uint8_t* ptr = key.data;
  uint24_t len;
  if(!max)
    max = key.count > other.size() ? (uint32_t)key.count : other.size();
  for(uint32_t c = 0; c<max; ++c, ptr += len) {

    if(c == key.count || c == other.size())
      return key.count > other.size() 
            ? break_if != Condition::LT 
            : break_if != Condition::GT;

    if(!(len = Serialization::decode_vi24(&ptr)) && empty_ok)
      continue;

    auto& r = other[c];
    if(condition<T_seq>(ptr, len, (const uint8_t*)r.data(), r.length())
       == break_if)
      return false;
  }
  return true;
}

template<> 
inline __attribute__((__always_inline__)) 
bool
compare<Types::KeySeq::BITWISE_FCOUNT>(
        const Cell::Key& key, const Cell::KeyVec& other, 
        Condition::Comp break_if, uint32_t max, bool empty_ok) {
  if(!max || max > key.count || max > other.size() ) {
    if(key.count < other.size())
      return break_if != Condition::GT;
    if(key.count > other.size())
      return break_if != Condition::LT;
  }
  return compare<Types::KeySeq::BITWISE>(key, other, break_if, max, empty_ok);
}

template<> 
inline __attribute__((__always_inline__)) 
bool
compare<Types::KeySeq::VOLUME_FCOUNT>(
        const Cell::Key& key, const Cell::KeyVec& other, 
        Condition::Comp break_if, uint32_t max, bool empty_ok) {
  if(!max || max > key.count || max > other.size() ) {
    if(key.count < other.size())
      return break_if != Condition::GT;
    if(key.count > other.size())
      return break_if != Condition::LT;
  }
  return compare<Types::KeySeq::VOLUME>(key, other, break_if, max, empty_ok);
}


SWC_NOINLINE bool 
compare(const Types::KeySeq seq, 
        const Cell::Key& key, const Cell::KeyVec& other,
        Condition::Comp break_if, uint32_t max, bool empty_ok) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
      return compare<Types::KeySeq::BITWISE>(
        key, other, break_if, max, empty_ok);

    case Types::KeySeq::VOLUME:
      return compare<Types::KeySeq::VOLUME>(
        key, other, break_if, max, empty_ok);

    case Types::KeySeq::BITWISE_FCOUNT:
      return compare<Types::KeySeq::BITWISE_FCOUNT>(
        key, other, break_if, max, empty_ok);

    case Types::KeySeq::VOLUME_FCOUNT:
      return compare<Types::KeySeq::VOLUME_FCOUNT>(
        key, other, break_if, max, empty_ok);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE bool
align(const Cell::Key& key, Cell::KeyVec& start, Cell::KeyVec& finish) {
  const uint8_t* ptr = key.data;
  uint24_t len;
  bool chg = false;
  for(uint24_t c = 0; c < key.count; ++c, ptr += len) {
    len = Serialization::decode_vi24(&ptr);

    if(c == start.size()) {
      start.add(ptr, len);
      chg = true;
    } else if(condition<T_seq>(
        (const uint8_t*)start[c].data(), start[c].length(),
        ptr, len) == Condition::LT) {
      start.set(c, ptr, len);
      chg = true;
    }

    if(c == finish.size()) {
      finish.add(ptr, len);
      chg = true;
    } else if(condition<T_seq>(
        (const uint8_t*)finish[c].data(), finish[c].length(),
        ptr, len) == Condition::GT) {
      finish.set(c, ptr, len);
      chg = true;
    }
  }
  return chg;
}


SWC_NOINLINE bool 
align(const Types::KeySeq seq, const Cell::Key& key, 
      Cell::KeyVec& start, Cell::KeyVec& finish) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
    case Types::KeySeq::BITWISE_FCOUNT:
      return align<Types::KeySeq::BITWISE>(key, start, finish);

    case Types::KeySeq::VOLUME:
    case Types::KeySeq::VOLUME_FCOUNT:
      return align<Types::KeySeq::VOLUME>(key, start, finish);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE bool
align(Cell::KeyVec& key, const Cell::KeyVec& other, Condition::Comp comp) {
  bool chg;
  if(chg = key.empty()) {
    if(chg = !other.empty())
      key.assign(other.begin(), other.end());
    return chg;
  }
  bool smaller = key.size() < other.size();
  uint32_t min = smaller ? key.size() : other.size();
  for(uint32_t c = 0; c < min; ++c) {
    std::string& r1 = key[c];
    const std::string& r2 = other[c];
    if(condition<T_seq>((const uint8_t*)r1.data(), r1.length(),
                        (const uint8_t*)r2.data(), r2.length()) == comp) {
      r1 = r2;
      chg = true;
    }
  }
  if(smaller) {
    for(uint32_t c = key.size(); c < other.size(); ++c) {
      key.add(other[c]);
      chg = true;
    }
  }
  return chg;
}


SWC_NOINLINE bool 
align(const Types::KeySeq seq, Cell::KeyVec& key, 
      const Cell::KeyVec& other, Condition::Comp comp) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
    case Types::KeySeq::BITWISE_FCOUNT:
      return align<Types::KeySeq::BITWISE>(key, other, comp);

    case Types::KeySeq::VOLUME:
    case Types::KeySeq::VOLUME_FCOUNT:
      return align<Types::KeySeq::VOLUME>(key, other, comp);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE bool
is_matching(Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len);

template<> 
inline __attribute__((__always_inline__)) 
bool
is_matching<Types::KeySeq::BITWISE>(Condition::Comp comp,
                                    const uint8_t *p1, uint32_t p1_len, 
                                    const uint8_t *p2, uint32_t p2_len) {
  return Condition::is_matching_bitwise(comp, p1, p1_len, p2, p2_len);
}

template<> 
inline __attribute__((__always_inline__)) 
bool
is_matching<Types::KeySeq::VOLUME>(Condition::Comp comp,
                                   const uint8_t *p1, uint32_t p1_len, 
                                   const uint8_t *p2, uint32_t p2_len) {
  return Condition::is_matching_volume(comp, p1, p1_len, p2, p2_len);
}

SWC_NOINLINE bool
is_matching(const Types::KeySeq seq, Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
    case Types::KeySeq::BITWISE_FCOUNT:
      return is_matching<Types::KeySeq::BITWISE>(comp, p1, p1_len, p2, p2_len);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::VOLUME_FCOUNT:
      return is_matching<Types::KeySeq::VOLUME>(comp, p1, p1_len, p2, p2_len);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE bool
is_matching(const Specs::Key& key, const Cell::Key &other) {
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = other.data;
  uint32_t c = other.count;
  uint32_t len;
  for(auto it = key.begin(); c && it < key.end(); ++it, --c, ptr += len) {
    if(!is_matching<T_seq>(
        comp = it->comp, 
        (const uint8_t*)it->value.data(), it->value.size(),
        ptr, len = Serialization::decode_vi32(&ptr) ))
      return false;
  }

  if(key.size() == other.count) 
    return true;

  switch(comp) {
    case Condition::LT:
    case Condition::LE:
      return key.empty() || key.size() > other.count;
    case Condition::GT:
    case Condition::GE:
      return key.empty() || key.size() < other.count;
    case Condition::PF:
    case Condition::RE:
      return key.size() < other.count;
    case Condition::NE:
    case Condition::NONE:
      return true;
    default: // Condition::EQ:
      return false;
  }
}


SWC_NOINLINE bool 
is_matching(const Types::KeySeq seq, const Specs::Key& key, 
                                     const Cell::Key &other) {
  switch(seq) {

    case Types::KeySeq::BITWISE:
    case Types::KeySeq::BITWISE_FCOUNT:
      return is_matching<Types::KeySeq::BITWISE>(key, other);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::VOLUME_FCOUNT:
      return is_matching<Types::KeySeq::VOLUME>(key, other);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///




}}}


# undef SWC_CAN_INLINE 
# undef SWC_NOINLINE 

