/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Columns_h
#define swcdb_ranger_db_Columns_h

#include "swcdb/db/Columns/Schema.h"

namespace SWC { namespace Ranger {
class Column;
typedef std::shared_ptr<Column> ColumnPtr;
class Range;
typedef std::shared_ptr<Range> RangePtr;
}}

#include "swcdb/ranger/callbacks/ManageBase.h"
#include "swcdb/ranger/callbacks/RangeLoad.h"
#include "swcdb/ranger/callbacks/RangeUnload.h"
#include "swcdb/ranger/callbacks/RangeUnloadInternal.h"
#include "swcdb/ranger/callbacks/ColumnsUnload.h"
#include "swcdb/ranger/callbacks/ColumnsUnloadAll.h"
#include "swcdb/ranger/callbacks/ColumnDelete.h"

#include "swcdb/ranger/db/ColumnCfg.h"
#include "swcdb/ranger/db/Column.h"

#include <unordered_map>


namespace SWC { namespace Ranger {


class Columns final : private std::unordered_map<cid_t, ColumnPtr> {
  public:

  typedef Columns* Ptr;

  SWC_CAN_INLINE
  Columns() noexcept { }

  //~Columns() { }

  ColumnPtr get_column(const cid_t cid);

  RangePtr get_range(int &err, const cid_t cid, const rid_t rid);

  ColumnPtr get_next(size_t& idx);

  void get_cids(std::vector<cid_t>& cids);

  void load_range(const DB::Schema& schema,
                  const Callback::RangeLoad::Ptr& req);

  void unload(cid_t cid_begin, cid_t cid_end,
              Callback::ColumnsUnload::Ptr req);

  void unload_all(bool validation);

  void erase_if_empty(cid_t cid);

  void internal_delete(cid_t cid);

  size_t release(size_t bytes=0);

  void print(std::ostream& out, bool minimal=true);

  private:

  Core::MutexSptd    m_mutex;
  Core::StateRunning m_release;

};

}} // namespace SWC::Ranger


//#ifdef SWC_IMPL_SOURCE
#include "swcdb/ranger/db/Range.cc"
#include "swcdb/ranger/db/Column.cc"
#include "swcdb/ranger/db/Columns.cc"

#include "swcdb/ranger/db/RangeBlock.cc"
#include "swcdb/ranger/db/RangeBlocks.cc"
#include "swcdb/ranger/db/RangeBlockLoader.cc"

#include "swcdb/ranger/db/CellStoreReaders.cc"
#include "swcdb/ranger/db/CellStore.cc"
#include "swcdb/ranger/db/CellStoreBlock.cc"

#include "swcdb/ranger/db/CommitLog.cc"
#include "swcdb/ranger/db/CommitLogFragment.cc"
#include "swcdb/ranger/db/CommitLogCompact.cc"

#include "swcdb/ranger/db/RangeData.cc"

#include "swcdb/ranger/callbacks/RangeLoad.cc"
#include "swcdb/ranger/callbacks/RangeUnload.cc"
#include "swcdb/ranger/callbacks/ColumnDelete.cc"
#include "swcdb/ranger/callbacks/ColumnsUnload.cc"
#include "swcdb/ranger/callbacks/ColumnsUnloadAll.cc"

//#endif


#endif // swcdb_ranger_db_Columns_h
