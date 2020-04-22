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

namespace KeySeq {


Condition::Comp 
__attribute__((__noinline__))
compare(const Types::KeySeq seq, const Cell::Key& key, const Cell::Key& other);

Condition::Comp 
__attribute__((__noinline__))
compare(const Types::KeySeq seq, const Cell::Key& key, const Cell::Key& other,
        uint32_t max, bool empty_ok=false, bool empty_eq=false);

bool 
__attribute__((__noinline__))
compare(const Types::KeySeq seq, 
        const Cell::Key& key, const Cell::KeyVec& other,
        Condition::Comp break_if, uint32_t max = 0, bool empty_ok=false);

bool 
__attribute__((__noinline__))
is_matching(const Types::KeySeq seq, const Specs::Key& key, 
                                     const Cell::Key &other);

bool 
__attribute__((__noinline__))
align(const Types::KeySeq seq, const Cell::Key& key, 
      Cell::KeyVec& start, Cell::KeyVec& finish);
      
bool 
__attribute__((__noinline__))
align(const Types::KeySeq seq, Cell::KeyVec& key, 
      const Cell::KeyVec& other, Condition::Comp comp);
//



}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/KeyComparator.cc"
#endif 

#endif // swcdb_db_Cells_KeyComparator_h