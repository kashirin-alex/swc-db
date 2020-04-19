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
KeyComp::is_matching(Condition::Comp comp,
                     const char* p1, uint32_t p1_len, 
                     const char* p2, uint32_t p2_len) const { 
  return is_matching(
    comp, (const uint8_t*)p1, p1_len, (const uint8_t*)p2, p2_len);
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

bool 
KeyComp::is_matching(const Specs::Key& key, const Cell::Key &other) const {
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = other.data;
  uint32_t c = other.count;
  uint32_t len;
  for(auto it = key.begin(); c && it < key.end(); ++it, --c, ptr += len) {
    if(!is_matching(
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

bool  // NOT-READY
KeyComp::is_matching_fcount(const Specs::Key& key, const Cell::Key &other) const {
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = other.data;
  uint32_t c = other.count;
  uint32_t len;
  for(auto it = key.begin(); c && it < key.end(); ++it, --c, ptr += len) {
    if(!is_matching(
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


namespace Comparator { namespace Key {

const Bitwise          bitwise;
const BitwiseFcount    bitwise_fcount;
const BitwiseVol       bitwise_vol;
const BitwiseVolFcount bitwise_vol_fcount;

// Bitwise
Types::KeySeq 
Bitwise::get_type() const {
  return Types::KeySeq::BITWISE;
}

Condition::Comp 
Bitwise::condition(const uint8_t *p1, uint32_t p1_len, 
                   const uint8_t *p2, uint32_t p2_len) const {
  return Condition::condition_bitwise(p1, p1_len, p2, p2_len);
}

bool
Bitwise::is_matching(Condition::Comp comp,
                     const uint8_t *p1, uint32_t p1_len, 
                     const uint8_t *p2, uint32_t p2_len) const {
  return Condition::is_matching_bitwise(comp, p1, p1_len, p2, p2_len);
}


// BitwiseVol
Types::KeySeq 
BitwiseVol::get_type() const {
  return Types::KeySeq::BITWISE_VOL;
}

Condition::Comp 
BitwiseVol::condition(const uint8_t *p1, uint32_t p1_len, 
                      const uint8_t *p2, uint32_t p2_len) const {
  return Condition::condition_bitwise_vol(p1, p1_len, p2, p2_len);
}

bool
BitwiseVol::is_matching(Condition::Comp comp,
                        const uint8_t *p1, uint32_t p1_len, 
                        const uint8_t *p2, uint32_t p2_len) const {
  return Condition::is_matching_bitwise_vol(comp, p1, p1_len, p2, p2_len);
}



// BitwiseFcount
Types::KeySeq 
BitwiseFcount::get_type() const {
  return Types::KeySeq::BITWISE_FCOUNT;
}

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

bool 
BitwiseFcount::is_matching(const Specs::Key& key, 
                           const Cell::Key &other) const {
  return KeyComp::is_matching_fcount(key, other);
}



// BitwiseVolFcount
Types::KeySeq 
BitwiseVolFcount::get_type() const {
  return Types::KeySeq::BITWISE_VOL_FCOUNT;
}

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

bool 
BitwiseVolFcount::is_matching(const Specs::Key& key, 
                              const Cell::Key &other) const {
  return KeyComp::is_matching_fcount(key, other);
}


}}}}
