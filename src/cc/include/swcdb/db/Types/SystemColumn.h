/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_SystemColumn_h
#define swcdb_db_types_SystemColumn_h

#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Types/Range.h"


namespace SWC { namespace DB { namespace Types { namespace SystemColumn {


const cid_t CID_MASTER_BEGIN     = 1;
const cid_t CID_MASTER_END       = 4;
const cid_t CID_META_BEGIN       = 5;
const cid_t CID_META_END         = 8;

const cid_t SYS_CID_STATS        = 9;
const cid_t SYS_CID_DEFINE_LEXIC = 10;
const cid_t SYS_CID_END = SYS_CID_DEFINE_LEXIC;


SWC_CAN_INLINE
bool is_master(cid_t cid) noexcept {
  return cid <= CID_MASTER_END;
}

SWC_CAN_INLINE
bool is_meta(cid_t cid) noexcept {
  return cid >= CID_META_BEGIN && cid <= CID_META_END;
}

SWC_CAN_INLINE
bool is_data(cid_t cid) noexcept {
  return cid > CID_META_END;
}

SWC_CAN_INLINE
Range get_range_type(cid_t cid) noexcept {
  if(cid <= CID_MASTER_END)
    return Range::MASTER;
  if(cid <= CID_META_END)
    return Range::META;
  return Range::DATA;
}


KeySeq get_seq_type(cid_t cid) noexcept;


cid_t get_master_cid(KeySeq col_seq) noexcept;

cid_t get_meta_cid(KeySeq col_seq) noexcept;

const char* get_meta_cid_str(KeySeq col_seq) noexcept;

uint8_t get_sys_cid(KeySeq col_seq, Range col_type) noexcept;


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/SystemColumn.cc"
#endif

#endif // swcdb_db_types_SystemColumn_h
