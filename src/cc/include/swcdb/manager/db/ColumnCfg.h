/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_manager_db_ColumnCfg_h
#define swc_manager_db_ColumnCfg_h

#include "swcdb/db/Cells/KeyComparator.h"

namespace SWC { namespace Manager { 

class ColumnCfg final {
  
  public:
  const cid_t         cid;
  const Types::KeySeq key_seq;

  ColumnCfg(const DB::Schema::Ptr& schema) 
            : cid(schema->cid), key_seq(schema->col_seq) { 
  }

  ~ColumnCfg() { }

  void print(std::ostream& out) const {
    out << "cid=" << cid << " seq=" << Types::to_string(key_seq);
  }
};

}}

#endif // swc_manager_db_ColumnCfg_h