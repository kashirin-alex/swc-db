/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Cells_KeyComparator_h
#define swcdb_db_Cells_KeyComparator_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Cells/CellKeyVec.h"
#include "swcdb/db/Cells/SpecsKey.h"


namespace SWC { namespace DB { 


class KeyComp {
  public:
 
  static const KeyComp* get(Types::KeySeq seq);
  
  virtual Types::KeySeq get_type() const = 0;
  
  virtual Condition::Comp 
  condition(const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) const = 0;

  virtual bool 
  is_matching(Condition::Comp comp,
              const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_le) const = 0;

  bool 
  is_matching(Condition::Comp comp,
              const char* p1, uint32_t p1_len, 
              const char* p2, uint32_t p2_le) const;

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

  virtual bool 
  is_matching(const Specs::Key& key, const Cell::Key &other) const;

  virtual Condition::Comp 
  compare_fcount(const Cell::Key& key, const Cell::Key& other, uint32_t max=0, 
                 bool empty_ok=false, bool empty_eq=false) const;

  virtual bool 
  compare_fcount(const Cell::Key& key, const Cell::KeyVec& other,
                 Condition::Comp break_if, uint32_t max = 0, 
                 bool empty_ok=false) const;

  virtual bool 
  is_matching_fcount(const Specs::Key& key, const Cell::Key &other) const;

};
  

namespace Comparator { namespace Key {


class Bitwise : public KeyComp {
  public:

  Types::KeySeq get_type() const override;

  Condition::Comp 
  condition(const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) const override;
            
  bool 
  is_matching(Condition::Comp comp,
              const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_le) const override;
};


class BitwiseVol : public KeyComp {
  public:

  Types::KeySeq get_type() const override;

  Condition::Comp 
  condition(const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) const override;
            
  bool 
  is_matching(Condition::Comp comp,
              const uint8_t *p1, uint32_t p1_len, 
              const uint8_t *p2, uint32_t p2_le) const override;
};


class BitwiseFcount : public Bitwise {
  public:

  Types::KeySeq get_type() const override;

  Condition::Comp 
  compare(const Cell::Key& key, const Cell::Key& other,
          uint32_t max=0, bool empty_ok=false, 
          bool empty_eq=false) const override;

  bool 
  compare(const Cell::Key& key, const Cell::KeyVec& other, 
          Condition::Comp break_if, uint32_t max = 0, 
          bool empty_ok=false) const override;
          
  bool 
  is_matching(const Specs::Key& key, const Cell::Key &other) const override;
};


class BitwiseVolFcount : public BitwiseVol {
  public:

  Types::KeySeq get_type() const override;

  Condition::Comp 
  compare(const Cell::Key& key, const Cell::Key& other,
          uint32_t max=0, bool empty_ok=false, 
          bool empty_eq=false) const override;

  bool 
  compare(const Cell::Key& key, const Cell::KeyVec& other, 
          Condition::Comp break_if, uint32_t max = 0, 
          bool empty_ok=false) const override;

  bool 
  is_matching(const Specs::Key& key, const Cell::Key &other) const override;
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