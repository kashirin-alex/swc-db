
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Types/MetaColumn.h"


namespace SWC { namespace Types {  namespace MetaColumn {


bool is_master(int64_t cid) {
  return cid <= 4;
}

bool is_meta(int64_t cid) {
  return cid >= 5 && cid <= 8;
}

bool is_data(int64_t cid) {
  return cid > 8;
}


Range get_range_type(int64_t cid) {
  return cid <= 4 ? Range::MASTER : (cid <= 8 ? Range::META : Range::DATA);
}

KeySeq get_seq_type(int64_t cid) {
  return (
    (cid == 4 || cid == 8) 
    ? KeySeq::BITWISE_VOL_FCOUNT
    : ((cid == 3 || cid == 7) 
      ? KeySeq::BITWISE_FCOUNT 
      : ((cid == 2 || cid == 6) 
        ? KeySeq::BITWISE_VOL 
        :  KeySeq::BITWISE
        )
      )
    ); 
}


int64_t get_master_cid(KeySeq col_seq) {
  switch(col_seq) {
    case KeySeq::BITWISE_VOL_FCOUNT:
      return 4;
    case KeySeq::BITWISE_FCOUNT:
      return 3;
    case KeySeq::BITWISE_VOL:
      return 2;
    default:
      return 1;
  }
} 

const char* get_meta_cid(KeySeq col_seq) {
  switch(col_seq) {
    case KeySeq::BITWISE_VOL_FCOUNT:
      return "8";
    case KeySeq::BITWISE_FCOUNT:
      return "7";
    case KeySeq::BITWISE_VOL:
      return "6";
    default:
      return "5";
  }
}  

uint8_t get_sys_cid(KeySeq col_seq, Range col_type) {
  switch(col_seq) {
    case KeySeq::BITWISE_VOL_FCOUNT:
      return col_type == Range::DATA ? 8 : 4;
    case KeySeq::BITWISE_FCOUNT:
      return col_type == Range::DATA ? 7 : 3;
    case KeySeq::BITWISE_VOL:
      return col_type == Range::DATA ? 6 : 2;
    default:
      return col_type == Range::DATA ? 5 : 1;
  }
}  

}}}