/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"



namespace SWC { namespace DB { namespace KeySeq {


///
template<Types::KeySeq T_seq>
Condition::Comp 
condition(const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len);

template<> 
Condition::Comp 
condition<Types::KeySeq::LEXIC>(const uint8_t *p1, uint32_t p1_len, 
                                const uint8_t *p2, uint32_t p2_len) {
  return Condition::condition_lexic(p1, p1_len, p2, p2_len);
}

template<> 
Condition::Comp 
condition<Types::KeySeq::VOLUME>(const uint8_t *p1, uint32_t p1_len, 
                                 const uint8_t *p2, uint32_t p2_len) {
  return Condition::condition_volume(p1, p1_len, p2, p2_len);
}

Condition::Comp  
condition(const Types::KeySeq seq, 
          const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return condition<Types::KeySeq::LEXIC>(p1, p1_len, p2, p2_len);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return condition<Types::KeySeq::VOLUME>(p1, p1_len, p2, p2_len);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///


///
template<Types::KeySeq T_seq>
Condition::Comp 
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
Condition::Comp 
compare<Types::KeySeq::FC_LEXIC>(const Cell::Key& key, 
                                 const Cell::Key& other) {
  if(key.count < other.count)
    return Condition::GT;
  if(key.count > other.count)
    return Condition::LT;
  return compare<Types::KeySeq::LEXIC>(key, other);
}

template<>
Condition::Comp 
compare<Types::KeySeq::FC_VOLUME>(const Cell::Key& key, 
                                  const Cell::Key& other) {
  if(key.count < other.count)
    return Condition::GT;
  if(key.count > other.count)
    return Condition::LT;
  return compare<Types::KeySeq::VOLUME>(key, other);
}


Condition::Comp 
compare(const Types::KeySeq seq, const Cell::Key& key, 
                                 const Cell::Key& other) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
      return compare<Types::KeySeq::LEXIC>(key, other);

    case Types::KeySeq::VOLUME:
      return compare<Types::KeySeq::VOLUME>(key, other);

    case Types::KeySeq::FC_LEXIC:
      return compare<Types::KeySeq::FC_LEXIC>(key, other);

    case Types::KeySeq::FC_VOLUME:
      return compare<Types::KeySeq::FC_VOLUME>(key, other);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq>
Condition::Comp 
compare(const Cell::Key& key, const Cell::Key& other,
        uint24_t max, bool empty_ok, bool empty_eq) {
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
Condition::Comp 
compare<Types::KeySeq::FC_LEXIC>(
        const Cell::Key& key, const Cell::Key& other,
        uint24_t max, bool empty_ok, bool empty_eq) {
  if(!max || max > key.count || max > other.count ) {
    if(key.count < other.count)
      return Condition::GT;
    if(key.count > other.count)
      return Condition::LT;
  }
  return compare<Types::KeySeq::LEXIC>(key, other, max, empty_ok, empty_eq);
}

template<>
Condition::Comp 
compare<Types::KeySeq::FC_VOLUME>(
        const Cell::Key& key, const Cell::Key& other,
        uint24_t max, bool empty_ok, bool empty_eq) {
  if(!max || max > key.count || max > other.count ) {
    if(key.count < other.count)
      return Condition::GT;
    if(key.count > other.count)
      return Condition::LT;
  }
  return compare<Types::KeySeq::VOLUME>(key, other, max, empty_ok, empty_eq);
}


Condition::Comp 
compare(const Types::KeySeq seq, const Cell::Key& key, const Cell::Key& other,
        int32_t max, bool empty_ok, bool empty_eq) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
      if(max == -1)
        max = key.count;
      return compare<Types::KeySeq::LEXIC>(
        key, other, max, empty_ok, empty_eq);

    case Types::KeySeq::VOLUME:
      if(max == -1)
        max = key.count;
      return compare<Types::KeySeq::VOLUME>(
        key, other, max, empty_ok, empty_eq);

    case Types::KeySeq::FC_LEXIC:
      if(max == -1)
        max = other.count;
      return compare<Types::KeySeq::FC_LEXIC>(
        key, other, max, empty_ok, empty_eq);

    case Types::KeySeq::FC_VOLUME:
      if(max == -1)
        max = other.count;
      return compare<Types::KeySeq::FC_VOLUME>(
        key, other, max, empty_ok, empty_eq);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
bool
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
bool
compare<Types::KeySeq::FC_LEXIC>(
        const Cell::Key& key, const Cell::KeyVec& other, 
        Condition::Comp break_if, uint32_t max, bool empty_ok) {
  if(!max || max > key.count || max > other.size() ) {
    if(key.count < other.size())
      return break_if != Condition::GT;
    if(key.count > other.size())
      return break_if != Condition::LT;
  }
  return compare<Types::KeySeq::LEXIC>(key, other, break_if, max, empty_ok);
}

template<>
bool
compare<Types::KeySeq::FC_VOLUME>(
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


bool 
compare(const Types::KeySeq seq, 
        const Cell::Key& key, const Cell::KeyVec& other,
        Condition::Comp break_if, uint32_t max, bool empty_ok) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
      return compare<Types::KeySeq::LEXIC>(
        key, other, break_if, max, empty_ok);

    case Types::KeySeq::VOLUME:
      return compare<Types::KeySeq::VOLUME>(
        key, other, break_if, max, empty_ok);

    case Types::KeySeq::FC_LEXIC:
      return compare<Types::KeySeq::FC_LEXIC>(
        key, other, break_if, max, empty_ok);

    case Types::KeySeq::FC_VOLUME:
      return compare<Types::KeySeq::FC_VOLUME>(
        key, other, break_if, max, empty_ok);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq>
bool
align(const Cell::Key& key, Cell::KeyVec& start, Cell::KeyVec& finish) {
  const uint8_t* ptr = key.data;
  uint24_t len;
  bool chg = false;
  auto it_min = start.begin();
  auto it_max = finish.begin();
  for(uint24_t c = 0; c < key.count; ++c, ptr += len) {
    len = Serialization::decode_vi24(&ptr);

    if(it_min == start.end()) {
      start.add(ptr, len);
      chg = true;
      it_min = start.end();
    } else {
      if(condition<T_seq>((const uint8_t*)it_min->data(), it_min->size(),
                          ptr, len) == Condition::LT) {
        *it_min = std::string((const char*)ptr, len);
        chg = true;
      }
      ++it_min;
    }

    if(it_max == finish.end()) {
      finish.add(ptr, len);
      chg = true;
      it_max = finish.end();
    } else {
      if(condition<T_seq>((const uint8_t*)it_max->data(), it_max->size(),
                          ptr, len) == Condition::GT) {
        *it_max = std::string((const char*)ptr, len);
        chg = true;
      }
      ++it_max;
    }
  }
  return chg;
}


bool 
align(const Types::KeySeq seq, const Cell::Key& key, 
      Cell::KeyVec& start, Cell::KeyVec& finish) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return align<Types::KeySeq::LEXIC>(key, start, finish);

    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return align<Types::KeySeq::VOLUME>(key, start, finish);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
bool
align(Cell::KeyVec& key, const Cell::KeyVec& other, Condition::Comp comp) {
  bool chg;
  if(chg = key.empty()) {
    if(chg = !other.empty())
      key.assign(other.cbegin(), other.cend());
    return chg;
  }
  bool small;
  uint24_t min = (small=key.size() < other.size())? key.size() : other.size();
  auto it2 = other.cbegin();
  for(auto it1 = key.begin(); min; --min, ++it1, ++it2) {
    if(condition<T_seq>((const uint8_t*)it1->data(), it1->size(),
                        (const uint8_t*)it2->data(), it2->size())
                         == comp) {
      *it1 = *it2;
      chg = true;
    }
  }
  if(small) {
    chg = true;
    do key.add(*it2);
    while(++it2 < other.cend());
  }
  return chg;
}


bool 
align(const Types::KeySeq seq, Cell::KeyVec& key, 
      const Cell::KeyVec& other, Condition::Comp comp) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return align<Types::KeySeq::LEXIC>(key, other, comp);

    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return align<Types::KeySeq::VOLUME>(key, other, comp);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
bool
is_matching(Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len);

template<>
bool
is_matching<Types::KeySeq::LEXIC>(Condition::Comp comp,
                                  const uint8_t *p1, uint32_t p1_len, 
                                  const uint8_t *p2, uint32_t p2_len) {
  return Condition::is_matching_lexic(comp, p1, p1_len, p2, p2_len);
}

template<>
bool
is_matching<Types::KeySeq::VOLUME>(Condition::Comp comp,
                                   const uint8_t *p1, uint32_t p1_len, 
                                   const uint8_t *p2, uint32_t p2_len) {
  return Condition::is_matching_volume(comp, p1, p1_len, p2, p2_len);
}

bool
is_matching(const Types::KeySeq seq, Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return is_matching<Types::KeySeq::LEXIC>(comp, p1, p1_len, p2, p2_len);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return is_matching<Types::KeySeq::VOLUME>(comp, p1, p1_len, p2, p2_len);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
bool
is_matching(const Specs::Key& key, const Cell::Key &other) {
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = other.data;
  uint32_t len;
  auto it = key.cbegin();
  for(uint24_t c = other.count; c && it < key.cend(); ++it, --c, ptr += len) {
    if(!is_matching<T_seq>(
        comp = it->comp, 
        (const uint8_t*)it->data(), it->size(),
        ptr, len = Serialization::decode_vi24(&ptr) ))
      return false;
  }
  if(key.size() == other.count || ( // [,,>=''] spec incl. prior-match
      key.size() == other.count + 1 && it->empty() && 
      it->comp == Condition::GE))
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


bool 
is_matching(const Types::KeySeq seq, const Specs::Key& key, 
                                     const Cell::Key &other) {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return is_matching<Types::KeySeq::LEXIC>(key, other);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return is_matching<Types::KeySeq::VOLUME>(key, other);

    default:
      SWC_ASSERT(seq != Types::KeySeq::UNKNOWN);
      return Condition::NONE;
  }
}
///




}}}

