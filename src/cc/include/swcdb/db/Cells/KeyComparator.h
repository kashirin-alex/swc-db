/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_cells_KeyComparator_h
#define swcdb_db_cells_KeyComparator_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Cells/CellKeyVec.h"
#include "swcdb/db/Types/KeySeq.h"


namespace SWC { namespace DB {



class KeyComparator {
  public:

  static KeyComparator* create(Types::KeySeq seq);


  virtual bool align(const Cell::Key& key, Cell::KeyVec& start, 
                                           Cell::KeyVec& finish) const = 0;

  virtual Condition::Comp compare(const Cell::Key& key, const Cell::Key& other,
                                  uint32_t max=0, bool empty_ok=false, 
                                  bool empty_eq=false) const = 0;

  virtual bool compare(const Cell::Key& key, const Cell::KeyVec& other, 
                       Condition::Comp break_if, uint32_t max = 0, 
                       bool empty_ok=false) const = 0;
};




class KeyCompBitwise : public KeyComparator {
  public:

  bool align(const Cell::Key& key, Cell::KeyVec& start, 
                                   Cell::KeyVec& finish) const override;

  Condition::Comp compare(const Cell::Key& key, const Cell::Key& other,
                          uint32_t max=0, bool empty_ok=false, 
                          bool empty_eq=false) const override;

  bool compare(const Cell::Key& key, const Cell::KeyVec& other, 
               Condition::Comp break_if, uint32_t max = 0, 
               bool empty_ok=false) const override;
};



class KeyCompBitwiseFcount : public KeyCompBitwise {
  public:

  Condition::Comp compare(const Cell::Key& key, const Cell::Key& other,
                          uint32_t max=0, bool empty_ok=false, 
                          bool empty_eq=false) const override;

  bool compare(const Cell::Key& key, const Cell::KeyVec& other, 
               Condition::Comp break_if, uint32_t max = 0, 
               bool empty_ok=false) const override;
};



class KeyCompBitwiseVol : public KeyComparator {
  public:

  bool align(const Cell::Key& key, Cell::KeyVec& start, 
                                   Cell::KeyVec& finish) const override;

  Condition::Comp compare(const Cell::Key& key, const Cell::Key& other,
                          uint32_t max=0, bool empty_ok=false, 
                          bool empty_eq=false) const override;

  bool compare(const Cell::Key& key, const Cell::KeyVec& other, 
               Condition::Comp break_if, uint32_t max = 0, 
               bool empty_ok=false) const override;
};



class KeyCompBitwiseVolFcount : public KeyCompBitwiseVol {
  public:

  Condition::Comp compare(const Cell::Key& key, const Cell::Key& other,
                          uint32_t max=0, bool empty_ok=false, 
                          bool empty_eq=false) const override;

  bool compare(const Cell::Key& key, const Cell::KeyVec& other, 
               Condition::Comp break_if, uint32_t max = 0, 
               bool empty_ok=false) const override;
};




}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/KeyComparator.cc"
#endif 

#endif // swcdb_db_Cells_KeyComparator_h