
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/core/Compat.h"


namespace SWC { namespace Types {  namespace MetaColumn {


SWC_SHOULD_INLINE
bool is_master(int64_t cid) {
  return cid <= 4;
}

SWC_SHOULD_INLINE
bool is_meta(int64_t cid) {
  return cid >= 5 && cid <= 8;
}

SWC_SHOULD_INLINE
bool is_data(int64_t cid) {
  return cid > 8;
}


Range get_range_type(int64_t cid) {
  return cid <= 4 ? Range::MASTER : (cid <= 8 ? Range::META : Range::DATA);
}

KeySeq get_seq_type(int64_t cid) {
  return (
    (cid == 4 || cid == 8) 
    ? KeySeq::FC_VOLUME
    : ((cid == 3 || cid == 7) 
      ? KeySeq::FC_LEXIC 
      : ((cid == 2 || cid == 6) 
        ? KeySeq::VOLUME 
        :  KeySeq::LEXIC
        )
      )
    ); 
}


int64_t get_master_cid(KeySeq col_seq) {
  switch(col_seq) {
    case KeySeq::FC_VOLUME:
      return 4;
    case KeySeq::FC_LEXIC:
      return 3;
    case KeySeq::VOLUME:
      return 2;
    default:
      return 1;
  }
} 

const char* get_meta_cid(KeySeq col_seq) {
  switch(col_seq) {
    case KeySeq::FC_VOLUME:
      return "8";
    case KeySeq::FC_LEXIC:
      return "7";
    case KeySeq::VOLUME:
      return "6";
    default:
      return "5";
  }
}  

uint8_t get_sys_cid(KeySeq col_seq, Range col_type) {
  switch(col_seq) {
    case KeySeq::FC_VOLUME:
      return col_type == Range::DATA ? 8 : 4;
    case KeySeq::FC_LEXIC:
      return col_type == Range::DATA ? 7 : 3;
    case KeySeq::VOLUME:
      return col_type == Range::DATA ? 6 : 2;
    default:
      return col_type == Range::DATA ? 5 : 1;
  }
}  

}}}