/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB {


const KeyComp* 
KeyComp::get(Types::KeySeq seq) {
  switch(seq) {
    case Types::KeySeq::BITWISE_VOL:
      return &Comparator::Key::bitwise_vol;
    case Types::KeySeq::BITWISE_FCOUNT:
      return &Comparator::Key::bitwise_fcount;
    case Types::KeySeq::BITWISE_VOL_FCOUNT:
      return &Comparator::Key::bitwise_vol_fcount;
    default: //case Types::KeySeq::BITWISE:
      return &Comparator::Key::bitwise;
  }
}


bool 
KeyComp::align(const Cell::Key& key, 
               Cell::KeyVec& start, Cell::KeyVec& finish) const {
  const uint8_t* ptr = key.data;
  uint24_t len;
  bool chg = false;
  for(uint24_t c = 0; c < key.count; ++c, ptr += len) {
    len = Serialization::decode_vi24(&ptr);

    if(c == start.size()) {
      start.add(ptr, len);
      chg = true;
    } else if(condition((const uint8_t*)start[c].data(), start[c].length(),
                        ptr, len) == Condition::LT) {
      start.set(c, ptr, len);
      chg = true;
    }

    if(c == finish.size()) {
      finish.add(ptr, len);
      chg = true;
    } else if(condition((const uint8_t*)finish[c].data(), finish[c].length(),
                        ptr, len) == Condition::GT) {
      finish.set(c, ptr, len);
      chg = true;
    }
  }
  return chg;
}

bool 
KeyComp::align(Cell::KeyVec& key, const Cell::KeyVec& other, 
               Condition::Comp comp) const {
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
    if(condition((const uint8_t*)r1.data(), r1.length(),
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

Condition::Comp 
KeyComp::compare(const Cell::Key& key, const Cell::Key& other,
                 uint32_t max, bool empty_ok, bool empty_eq) const {
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
      if((comp = condition(p1, sz1, p2, sz2)) != Condition::EQ)
        return comp;
    }
  }
  return key.count != other.count && (!max || max > key.count || max > other.count)
        ? key.count > other.count ? Condition::LT : Condition::GT
          : Condition::EQ;
}

bool 
KeyComp::compare(const Cell::Key& key, const Cell::KeyVec& other,
                 Condition::Comp break_if, uint32_t max,
                 bool empty_ok) const {
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
    if(condition(ptr, len, (const uint8_t*)r.data(), r.length()) == break_if)
      return false;
  }
  return true;
}

Condition::Comp // NOT-READY
KeyComp::compare_fcount(const Cell::Key& key, const Cell::Key& other,
                        uint32_t max, bool empty_ok, 
                        bool empty_eq) const {
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
      if((comp = condition(p1, sz1, p2, sz2)) != Condition::EQ)
        return comp;
    }
  }
  return key.count != other.count && (!max || max > key.count || max > other.count)
        ? key.count > other.count ? Condition::LT : Condition::GT
          : Condition::EQ;
}

bool  // NOT-READY
KeyComp::compare_fcount(const Cell::Key& key, const Cell::KeyVec& other,
                        Condition::Comp break_if, uint32_t max,
                        bool empty_ok) const {
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
    if(condition(ptr, len, (const uint8_t*)r.data(), r.length()) == break_if)
      return false;
  }
  return true;
}


namespace Comparator { namespace Key {

const Bitwise          bitwise;
const BitwiseFcount    bitwise_fcount;
const BitwiseVol       bitwise_vol;
const BitwiseVolFcount bitwise_vol_fcount;

// Bitwise
Condition::Comp 
Bitwise::condition(const uint8_t *p1, uint32_t p1_len, 
                   const uint8_t *p2, uint32_t p2_len) const {
  return Condition::condition_bitwise(p1, p1_len, p2, p2_len);
}



// BitwiseVol
Condition::Comp 
BitwiseVol::condition(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) const {
  return Condition::condition_bitwise_vol(p1, p1_len, p2, p2_len);
}



// BitwiseFcount
Condition::Comp 
BitwiseFcount::compare(const Cell::Key& key, const Cell::Key& other,
                       uint32_t max, bool empty_ok, 
                       bool empty_eq) const {
  return KeyComp::compare_fcount(key, other, max, empty_ok, empty_eq);
}

bool 
BitwiseFcount::compare(const Cell::Key& key, 
                       const Cell::KeyVec& other,
                       Condition::Comp break_if, uint32_t max,
                       bool empty_ok) const {
  return KeyComp::compare_fcount(key, other, break_if, max, empty_ok);
}



// BitwiseVolFcount
Condition::Comp 
BitwiseVolFcount::compare(const Cell::Key& key, const Cell::Key& other,
                          uint32_t max, bool empty_ok, 
                          bool empty_eq) const {
  return KeyComp::compare_fcount(key, other, max, empty_ok, empty_eq);
}

bool 
BitwiseVolFcount::compare(const Cell::Key& key, 
                          const Cell::KeyVec& other,
                          Condition::Comp break_if, uint32_t max, 
                          bool empty_ok) const {
  return KeyComp::compare_fcount(key, other, break_if, max, empty_ok);
}


}}}}
