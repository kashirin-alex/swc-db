
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_MetaColumn_h
#define swcdb_db_types_MetaColumn_h

#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Types/Range.h"

namespace SWC { namespace DB { namespace Types { namespace MetaColumn {


bool is_master(cid_t cid);

bool is_meta(cid_t cid);

bool is_data(cid_t cid);


Range get_range_type(cid_t cid);

KeySeq get_seq_type(cid_t cid);


cid_t get_master_cid(KeySeq col_seq);

const char* get_meta_cid(KeySeq col_seq);

uint8_t get_sys_cid(KeySeq col_seq, Range col_type); 


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/MetaColumn.cc"
#endif 

#endif // swcdb_db_types_MetaColumn_h