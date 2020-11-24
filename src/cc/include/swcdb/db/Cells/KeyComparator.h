/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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


//! The SWC-DB Key Comparator C++ namespace 'SWC::DB::KeySeq'
namespace KeySeq {



Condition::Comp
compare(const Types::KeySeq seq, 
        const Cell::Key& key, const Cell::Key& other);

Condition::Comp
compare_opt(const Cell::Key& key, const Cell::Key& other,
            uint24_t max, bool empty_ok, bool empty_eq);

Condition::Comp
compare_upto(const Types::KeySeq seq, 
             const Cell::Key& key, const Cell::Key& other, uint24_t max);

Condition::Comp
compare_incl(const Types::KeySeq seq,
             const Cell::Key& key, const Cell::Key& other);

Condition::Comp
compare_incl(const Types::KeySeq seq,
             const Cell::Key& key, const Cell::Key& other, bool rest);

bool
compare(const Types::KeySeq seq, 
        const Cell::Key& key, const Cell::KeyVec& other,
        Condition::Comp break_if, uint32_t max = 0, bool empty_ok=false);

bool
align(const Types::KeySeq seq, const Cell::Key& key, 
      Cell::KeyVec& start, Cell::KeyVec& finish);

bool
align(const Types::KeySeq seq, Cell::KeyVec& key, 
      const Cell::KeyVec& other, Condition::Comp comp);

bool
is_matching(const Types::KeySeq seq, const Specs::Key& key, 
                                     const Cell::Key &other);

//




///
template<Types::KeySeq T_seq>
SWC_CAN_INLINE
Condition::Comp 
condition(const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len) noexcept;

template<>
SWC_CAN_INLINE
Condition::Comp 
condition<Types::KeySeq::LEXIC>(const uint8_t *p1, uint32_t p1_len, 
                                const uint8_t *p2, uint32_t p2_len) noexcept {
  return Condition::condition_lexic(p1, p1_len, p2, p2_len);
}

template<> 
SWC_CAN_INLINE
Condition::Comp 
condition<Types::KeySeq::VOLUME>(const uint8_t *p1, uint32_t p1_len, 
                                 const uint8_t *p2, uint32_t p2_len) noexcept {
  return Condition::condition_volume(p1, p1_len, p2, p2_len);
}

extern SWC_CAN_INLINE
Condition::Comp
condition(const Types::KeySeq seq, 
          const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len)
          noexcept __attribute__((optimize("-O3")));

extern SWC_CAN_INLINE
Condition::Comp
condition(const Types::KeySeq seq, 
          const uint8_t *p1, uint32_t p1_len, 
          const uint8_t *p2, uint32_t p2_len) noexcept {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return condition<Types::KeySeq::LEXIC>(p1, p1_len, p2, p2_len);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return condition<Types::KeySeq::VOLUME>(p1, p1_len, p2, p2_len);

    default:
      return Condition::NONE;
  }
}
///



///
template<Types::KeySeq T_seq> 
SWC_CAN_INLINE
bool
is_matching(Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) noexcept;

template<>
SWC_CAN_INLINE
bool
is_matching<Types::KeySeq::LEXIC>(Condition::Comp comp,
                                  const uint8_t *p1, uint32_t p1_len, 
                                  const uint8_t *p2, uint32_t p2_len) noexcept {
  return Condition::is_matching_lexic(comp, p1, p1_len, p2, p2_len);
}

template<>
SWC_CAN_INLINE
bool
is_matching<Types::KeySeq::VOLUME>(Condition::Comp comp,
                                   const uint8_t *p1, uint32_t p1_len, 
                                   const uint8_t *p2, uint32_t p2_len) noexcept {
  return Condition::is_matching_volume(comp, p1, p1_len, p2, p2_len);
}


extern SWC_CAN_INLINE
bool
is_matching(const Types::KeySeq seq, Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) 
            noexcept __attribute__((optimize("-O3")));
            
extern SWC_CAN_INLINE
bool
is_matching(const Types::KeySeq seq, Condition::Comp comp,
            const uint8_t *p1, uint32_t p1_len, 
            const uint8_t *p2, uint32_t p2_len) noexcept {
  switch(seq) {

    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return is_matching<Types::KeySeq::LEXIC>(comp, p1, p1_len, p2, p2_len);
      
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return is_matching<Types::KeySeq::VOLUME>(comp, p1, p1_len, p2, p2_len);

    default:
      return false;
  }
}
///


}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/KeyComparator.cc"
#endif 


#endif // swcdb_db_Cells_KeyComparator_h
