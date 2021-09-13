/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_types_Identifiers_h
#define swcdb_db_types_Identifiers_h

#include "swcdb/core/Compat.h"

namespace SWC {

namespace DB { namespace Types { } }


typedef uint64_t cid_t;
typedef uint64_t rid_t;
typedef uint64_t rgrid_t;
typedef uint32_t csid_t;


typedef Core::Vector<cid_t> cids_t;
typedef Core::Vector<rid_t> rids_t;



}

#endif // swcdb_db_types_Identifiers_h
