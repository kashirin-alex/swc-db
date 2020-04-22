/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_manager_db_ColumnCfg_h
#define swc_manager_db_ColumnCfg_h

#include "swcdb/db/Cells/KeyComparator.h"

namespace SWC { namespace Manager { 

class ColumnCfg final {
  
  public:
  const int64_t       cid;
  const Types::KeySeq key_seq;

  ColumnCfg(DB::Schema::Ptr schema) 
            : cid(schema->cid), key_seq(schema->col_seq) { 
  }

  ~ColumnCfg() { }

  std::string to_string() const {
    std::string s("cid=");
    s.append(std::to_string(cid));
    s.append(" seq=");
    s.append(Types::to_string(key_seq));
    return s;
  }
};

}}

#endif // swc_manager_db_ColumnCfg_h