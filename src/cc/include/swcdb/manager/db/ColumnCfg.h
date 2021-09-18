/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_db_ColumnCfg_h
#define swcdb_manager_db_ColumnCfg_h

#include "swcdb/core/NotMovableSharedPtr.h"
#include "swcdb/db/Cells/KeyComparator.h"

namespace SWC { namespace Manager {

class ColumnCfg final : public Core::NotMovableSharedPtr<ColumnCfg> {
  public:

  typedef Core::NotMovableSharedPtr<ColumnCfg>    Ptr;

  const cid_t             cid;
  const DB::Types::KeySeq key_seq;

  SWC_CAN_INLINE
  ColumnCfg(const DB::Schema::Ptr& schema) noexcept
            : cid(schema->cid), key_seq(schema->col_seq) {
  }

  ~ColumnCfg() noexcept { }

  void print(std::ostream& out) const {
    out << "cid=" << cid << " seq=" << DB::Types::to_string(key_seq);
  }
};

}}

#endif // swcdb_manager_db_ColumnCfg_h
