/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_Range_h
#define swcdb_ranger_db_Range_h

namespace SWC { namespace Ranger {
class Range;
typedef std::shared_ptr<Range> RangePtr;
}}

#include "swcdb/core/QueueSafe.h"
#include "swcdb/db/Types/Range.h"
#include "swcdb/db/Columns/RangeBase.h"

#include "swcdb/ranger/db/ColumnCfg.h"
#include "swcdb/ranger/db/ReqScan.h"
#include "swcdb/ranger/db/RangeBlocks.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"


namespace SWC { namespace Ranger {

class Range final : public std::enable_shared_from_this<Range> {

  public:

  static constexpr const char* RANGE_FILE = "range.data";
  static constexpr const char* CELLSTORES_DIR = "cs";
  static constexpr const char* CELLSTORES_BAK_DIR = "cs_bak";
  static constexpr const char* CELLSTORES_TMP_DIR = "cs_tmp";
  static constexpr const char* LOG_DIR = "log"; 
  static constexpr const char* LOG_TMP_DIR = "log_tmp"; 

  struct ReqAdd final {
    public:
    ReqAdd(StaticBuffer& input, const Callback::RangeQueryUpdate::Ptr& cb) 
          : input(input), cb(cb) {}
    ~ReqAdd() {}
    StaticBuffer                          input;
    const Callback::RangeQueryUpdate::Ptr cb;
  };

  enum State {
    NOTLOADED,
    LOADING,
    LOADED,
    UNLOADING,
    DELETED,
  };

  // Compact states by process weight 
  static const uint8_t COMPACT_NONE        = 0x00;
  static const uint8_t COMPACT_CHECKING    = 0x01;
  static const uint8_t COMPACT_COMPACTING  = 0x02;
  static const uint8_t COMPACT_PREPARING   = 0x03;
  static const uint8_t COMPACT_APPLYING    = 0x04;

  const ColumnCfg*    cfg;
  const rid_t         rid;
  const Types::Range  type;
  const uint8_t       meta_cid;
  Blocks              blocks;

  Range(const ColumnCfg* cfg, const rid_t rid);

  void init();

  ~Range();
  
  const std::string get_path(const std::string suff) const;

  const std::string get_path_cs(const csid_t csid) const;

  const std::string get_path_cs_on(const std::string folder, 
                                   const csid_t csid) const;

  Files::RgrData::Ptr get_last_rgr(int &err);

  void get_interval(DB::Cells::Interval& interval);

  void get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end);

  void get_key_end(DB::Cell::Key& key);
  
  bool is_any_begin();

  bool is_any_end();

  uint24_t known_interval_count();

  void get_prev_key_end(DB::Cell::Key& key);
  
  void set_prev_key_end(const DB::Cell::Key& key);

  bool align(const DB::Cells::Interval& interval);
  
  bool align(const DB::Cell::Key& key);

  void schema_update(bool compact);

  void set_state(State new_state);

  bool is_loaded();

  bool deleted();

  void add(ReqAdd* req);

  void scan(const ReqScan::Ptr& req);

  void scan_internal(const ReqScan::Ptr& req);

  void create_folders(int& err);

  void load(const ResponseCallback::Ptr& cb);

  void take_ownership(int &err, const ResponseCallback::Ptr& cb);

  void on_change(int &err, bool removal, 
                 const DB::Cell::Key* old_key_begin=nullptr,
                 const client::Query::Update::Cb_t& cb=0);

  void unload(const Callback::RangeUnloaded_t& cb, bool completely);
  
  void remove(int &err, bool meta=true);

  void wait_queue();

  bool compacting();

  bool compacting_is(uint8_t state);

  void compacting(uint8_t state);
  
  bool compact_possible(bool minor=true);

  void compact_require(bool require);

  bool compact_required();

  void apply_new(int &err,
                CellStore::Writers& w_cellstores, 
                CommitLog::Fragments::Vec& fragments_old, 
                bool w_update);
  
  void expand_and_align(int &err, bool w_chg_chk);
  
  void create(int &err, const CellStore::Writers& w_cellstores);

  std::string to_string();

  private:

  void loaded(int &err, const ResponseCallback::Ptr& cb);

  void last_rgr_chk(int &err, const ResponseCallback::Ptr& cb);

  void load(int &err, const ResponseCallback::Ptr& cb);

  void loaded_ack(int err, const ResponseCallback::Ptr& cb);

  bool wait(uint8_t from_state=COMPACT_CHECKING);

  void run_add_queue();

  private:
  const std::string             m_path;
  LockAtomic::Unique            m_mutex_intval;
  DB::Cells::Interval           m_interval;
  DB::Cell::Key                 m_prev_key_end;

  std::shared_mutex             m_mutex;

  std::atomic<State>            m_state;
  uint8_t                       m_compacting;
  bool                          m_require_compact;
  QueueSafe<ReqScan::Ptr>       m_q_scans;
  QueueSafe<ReqAdd*>            m_q_adding;

  std::condition_variable_any   m_cv;

  std::atomic<size_t>           m_inbytes;
};



}}




//#ifdef SWC_IMPL_SOURCE
#include "swcdb/ranger/db/Range.cc"

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


#endif //swcdb_ranger_db_Range_h