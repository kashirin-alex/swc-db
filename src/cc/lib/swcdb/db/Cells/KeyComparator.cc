/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/core/Serialization.h"



namespace SWC { namespace DB { namespace KeySeq {




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
      sz1 = Serialization::decode_vi24(&p1);
      sz2 = Serialization::decode_vi24(&p2);
      if((comp = condition<T_seq>(p1, sz1, p2, sz2)) != Condition::EQ)
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
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq>
Condition::Comp 
compare_opt(const Cell::Key& opt_empty, const Cell::Key& other,
            uint24_t max, bool empty_eq) {
  if(uint24_t min = opt_empty.count < other.count 
                      ? opt_empty.count : other.count) {
    if(max && min > max)
      min = max;  
    const uint8_t* p1 = opt_empty.data;
    const uint8_t* p2 = other.data;
    uint24_t sz1;
    uint24_t sz2;
    for(Condition::Comp comp; min; --min, p1 += sz1, p2 += sz2) {
      sz2 = Serialization::decode_vi24(&p2);
      if(!(sz1 = Serialization::decode_vi24(&p1)) && empty_eq)
        return Condition::EQ;
      if((comp = condition<T_seq>(p1, sz1, p2, sz2)) != Condition::EQ)
        return comp;
    }
  }
  return opt_empty.count != other.count && 
         (!max || max > opt_empty.count || max > other.count)
        ? opt_empty.count > other.count ? Condition::LT : Condition::GT
          : Condition::EQ;
}

template<>
Condition::Comp 
compare_opt<Types::KeySeq::FC_LEXIC>(
        const Cell::Key& opt_empty, const Cell::Key& other, 
        uint24_t max, bool empty_eq) {
  if(!max || max > opt_empty.count || max > other.count) {
    if(opt_empty.count < other.count)
      return Condition::GT;
    if(opt_empty.count > other.count)
      return Condition::LT;
  }
  return compare_opt<Types::KeySeq::LEXIC>(
    opt_empty, other, max, empty_eq);
}

template<>
Condition::Comp 
compare_opt<Types::KeySeq::FC_VOLUME>(
        const Cell::Key& opt_empty, const Cell::Key& other, 
        uint24_t max, bool empty_eq) {
  if(!max || max > opt_empty.count || max > other.count) {
    if(opt_empty.count < other.count)
      return Condition::GT;
    if(opt_empty.count > other.count)
      return Condition::LT;
  }
  return compare_opt<Types::KeySeq::VOLUME>(
    opt_empty, other, max, empty_eq);
}


Condition::Comp
compare_opt_lexic(const Cell::Key& opt_empty, const Cell::Key& other,
                  uint24_t max, bool empty_eq) {
  return compare_opt<Types::KeySeq::LEXIC>(
    opt_empty, other, max, empty_eq);
}

Condition::Comp
compare_opt_volume(const Cell::Key& opt_empty, const Cell::Key& other,
                   uint24_t max, bool empty_eq) {
  return compare_opt<Types::KeySeq::VOLUME>(
    opt_empty, other, max, empty_eq);
}

Condition::Comp
compare_opt_fc_lexic(const Cell::Key& opt_empty, const Cell::Key& other,
                     uint24_t max, bool empty_eq) {
  return compare_opt<Types::KeySeq::FC_LEXIC>(
    opt_empty, other, max, empty_eq);
}

Condition::Comp
compare_opt_fc_volume(const Cell::Key& opt_empty, const Cell::Key& other,
                      uint24_t max, bool empty_eq) {
  return compare_opt<Types::KeySeq::FC_VOLUME>(
    opt_empty, other, max, empty_eq);
}


Condition::Comp 
compare_upto(const Types::KeySeq seq, 
             const Cell::Key& key, const Cell::Key& other, 
             uint24_t max) {
  switch(seq) {
    case Types::KeySeq::LEXIC:
      return compare_opt<Types::KeySeq::LEXIC>(
        key, other, max, false);

    case Types::KeySeq::VOLUME:
      return compare_opt<Types::KeySeq::VOLUME>(
        key, other, max, false);

    case Types::KeySeq::FC_LEXIC:
      return compare_opt<Types::KeySeq::FC_LEXIC>(
        key, other, max, false);

    case Types::KeySeq::FC_VOLUME:
      return compare_opt<Types::KeySeq::FC_VOLUME>(
        key, other, max, false);

    default:
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE
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
    if(condition<T_seq>(ptr, len, r.data(), r.length())
       == break_if)
      return false;
  }
  return true;
}

template<>
SWC_CAN_INLINE
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
SWC_CAN_INLINE
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
      return false;
  }
}
///



///
template<Types::KeySeq T_seq>
SWC_SHOULD_NOT_INLINE
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
      if(condition<T_seq>(it_min->data(), it_min->size(),
                          ptr, len) == Condition::LT) {
        it_min->assign(ptr, len);
        chg = true;
      }
      ++it_min;
    }

    if(it_max == finish.end()) {
      finish.add(ptr, len);
      chg = true;
      it_max = finish.end();
    } else {
      if(condition<T_seq>(it_max->data(), it_max->size(),
                          ptr, len) == Condition::GT) {
        it_max->assign(ptr, len);
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
      return false;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_SHOULD_NOT_INLINE
bool
align(Cell::KeyVec& key, const Cell::KeyVec& other, Condition::Comp comp) {
  if(other.empty())
    return false;
  if(key.empty()) {
    key.assign(other.cbegin(), other.cend());
    return true;
  }
  bool small;
  uint24_t min = (small=key.size() < other.size())? key.size() : other.size();
  bool chg = false;
  auto it2 = other.cbegin();
  for(auto it1 = key.begin(); min; --min, ++it1, ++it2) {
    if(condition<T_seq>(it1->data(), it1->size(),
                        it2->data(), it2->size())
                         == comp) {
      it1->assign(*it2);
      chg = true;
    }
  }
  if(small) {
    do key.add(*it2);
    while(++it2 < other.cend());
    return true;
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
      return false;
  }
}
///






///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE
bool
is_matching(const Specs::Key& key, const Cell::Key &other) {
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = other.data;
  uint32_t len;
  auto it = key.cbegin();
  for(uint24_t c = other.count; c && it < key.cend(); ++it, --c, ptr += len) {
    len = Serialization::decode_vi24(&ptr);
    if(!is_matching<T_seq>(
        comp = it->comp, 
        (const uint8_t*)it->data(), it->size(),
        ptr, len ))
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
      return false;
  }
}
///




}}}

