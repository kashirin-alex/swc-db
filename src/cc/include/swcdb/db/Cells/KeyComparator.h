/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Cells_KeyComparator_h
#define swcdb_db_Cells_KeyComparator_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Cells/CellKeyVec.h"
#include "swcdb/db/Types/KeySeq.h"


namespace SWC { namespace DB { 


class KeyComp {
  public:
 
  static const KeyComp* get(Types::KeySeq seq);

  virtual Condition::Comp 
  condition(const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) const = 0;

  bool 
  align(const Cell::Key& key, Cell::KeyVec& start, Cell::KeyVec& finish) const;
        
  bool 
  align(Cell::KeyVec& key, const Cell::KeyVec& other, 
        Condition::Comp comp) const;

  virtual Condition::Comp 
  compare(const Cell::Key& key, const Cell::Key& other,
          uint32_t max=0, bool empty_ok=false, 
          bool empty_eq=false) const;

  virtual bool 
  compare(const Cell::Key& key, const Cell::KeyVec& other, 
          Condition::Comp break_if, uint32_t max = 0, 
          bool empty_ok=false) const;

  virtual Condition::Comp 
  compare_fcount(const Cell::Key& key, const Cell::Key& other, uint32_t max=0, 
                 bool empty_ok=false, bool empty_eq=false) const;

  virtual bool 
  compare_fcount(const Cell::Key& key, const Cell::KeyVec& other,
                 Condition::Comp break_if, uint32_t max = 0, 
                 bool empty_ok=false) const;
};
  

namespace Comparator { namespace Key {


class Bitwise : public KeyComp {
  public:

  Condition::Comp 
  condition(const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) const override;
};


class BitwiseVol : public KeyComp {
  public:

  Condition::Comp 
  condition(const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) const override;
};


class BitwiseFcount : public Bitwise {
  public:

  Condition::Comp 
  compare(const Cell::Key& key, const Cell::Key& other,
          uint32_t max=0, bool empty_ok=false, 
          bool empty_eq=false) const override;

  bool 
  compare(const Cell::Key& key, const Cell::KeyVec& other, 
          Condition::Comp break_if, uint32_t max = 0, 
          bool empty_ok=false) const override;
};


class BitwiseVolFcount : public BitwiseVol {
  public:

  Condition::Comp 
  compare(const Cell::Key& key, const Cell::Key& other,
          uint32_t max=0, bool empty_ok=false, 
          bool empty_eq=false) const override;

  bool 
  compare(const Cell::Key& key, const Cell::KeyVec& other, 
          Condition::Comp break_if, uint32_t max = 0, 
          bool empty_ok=false) const override;
};



extern const Bitwise          bitwise;
extern const BitwiseFcount    bitwise_fcount;
extern const BitwiseVol       bitwise_vol;
extern const BitwiseVolFcount bitwise_vol_fcount;


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/KeyComparator.cc"
#endif 

#endif // swcdb_db_Cells_KeyComparator_h