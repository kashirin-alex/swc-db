/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/KeyComparator.h"


namespace SWC { namespace DB {



KeyComparator* 
KeyComparator::create(Types::KeySeq seq) {
  switch(seq) {
    case Types::KeySeq::BITWISE_VOL:
      return new KeyCompBitwiseVol();
    case Types::KeySeq::BITWISE_FCOUNT:
      return new KeyCompBitwiseFcount();
    case Types::KeySeq::BITWISE_VOL_FCOUNT:
      return new KeyCompBitwiseVolFcount();
    default: //case Types::KeySeq::BITWISE:
      return new KeyCompBitwise();
  }
}



// KeyCompBitwise
bool 
KeyCompBitwise::align(const Cell::Key& key, Cell::KeyVec& start, 
                                            Cell::KeyVec& finish) const {
  return false;
}

Condition::Comp 
KeyCompBitwise::compare(const Cell::Key& key, const Cell::Key& other,
                        uint32_t max, bool empty_ok, 
                        bool empty_eq) const {
  return Condition::NONE;
}

bool 
KeyCompBitwise::compare(const Cell::Key& key, 
                        const Cell::KeyVec& other,
                        Condition::Comp break_if, uint32_t max, 
                        bool empty_ok) const {
  return false;
}



// KeyCompBitwiseFcount
Condition::Comp 
KeyCompBitwiseFcount::compare(const Cell::Key& key, const Cell::Key& other,
                              uint32_t max, bool empty_ok, 
                              bool empty_eq) const {
  return Condition::NONE;
}

bool 
KeyCompBitwiseFcount::compare(const Cell::Key& key, 
                              const Cell::KeyVec& other,
                              Condition::Comp break_if, uint32_t max, 
                              bool empty_ok) const {
  return false;
}



// KeyCompBitwiseVol
bool 
KeyCompBitwiseVol::align(const Cell::Key& key, Cell::KeyVec& start, 
                                               Cell::KeyVec& finish) const {
  return false;
}

Condition::Comp 
KeyCompBitwiseVol::compare(const Cell::Key& key, const Cell::Key& other,
                           uint32_t max, bool empty_ok, 
                           bool empty_eq) const {
  return Condition::NONE;
}

bool 
KeyCompBitwiseVol::compare(const Cell::Key& key, 
                           const Cell::KeyVec& other,
                           Condition::Comp break_if, uint32_t max, 
                           bool empty_ok) const {
  return false;
}



// KeyCompBitwiseVolFcount
Condition::Comp 
KeyCompBitwiseVolFcount::compare(const Cell::Key& key, const Cell::Key& other,
                          uint32_t max, bool empty_ok, 
                          bool empty_eq) const {
  return Condition::NONE;
}

bool 
KeyCompBitwiseVolFcount::compare(const Cell::Key& key, 
                                 const Cell::KeyVec& other,
                                 Condition::Comp break_if, uint32_t max, 
                                 bool empty_ok) const {
  return false;
}


}}