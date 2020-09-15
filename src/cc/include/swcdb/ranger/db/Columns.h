/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Columns_h
#define swcdb_ranger_db_Columns_h

#include "swcdb/db/Columns/Schema.h"
#include "swcdb/ranger/db/ColumnCfg.h"
#include "swcdb/ranger/db/Callbacks.h"
#include "swcdb/ranger/db/Column.h"
#include "swcdb/ranger/db/ColumnsReqDelete.h"

#include <unordered_map>

namespace SWC { namespace Ranger {


class Columns final : private std::unordered_map<cid_t, Column::Ptr> {

  public:

  enum State {
    NONE,
    OK
  };

  typedef Columns* Ptr;

  explicit Columns();

  ~Columns();

  
  Column::Ptr initialize(int &err, const cid_t cid, 
                         const DB::Schema& schema);

  void get_cids(std::vector<cid_t>& cids);

  Column::Ptr get_column(int&, const cid_t cid);
  
  Column::Ptr get_next(size_t& idx);

  RangePtr get_range(int &err, const cid_t cid, const rid_t rid);
 
  void load_range(int &err, const cid_t cid, const rid_t rid, 
                  const DB::Schema& schema, 
                  const ResponseCallback::Ptr& cb);

  void unload_range(int &err, const cid_t cid, const rid_t rid,
                    const Callback::RangeUnloaded_t& cb);

  void unload(cid_t cid_begin, cid_t cid_end,
              const Callback::ColumnsUnloadedPtr& cb);

  void unload_all(bool validation);

  void remove(ColumnsReqDelete* req);
  
  size_t release(size_t bytes=0);

  void print(std::ostream& out, bool minimal=true);

  private:
  Mutex                           m_mutex;
  std::atomic<bool>               m_releasing;
  // State                           m_state;
  QueueSafe<ColumnsReqDelete*>    m_q_remove;

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
//#endif

#endif
