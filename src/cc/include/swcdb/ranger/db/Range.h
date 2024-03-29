/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Range_h
#define swcdb_ranger_db_Range_h


#include "swcdb/core/QueuePointer.h"
#include "swcdb/core/QueueSafe.h"
#include "swcdb/db/Types/Range.h"
#include "swcdb/db/Columns/RangeBase.h"

#include "swcdb/ranger/db/ColumnCfg.h"
#include "swcdb/ranger/db/ReqScan.h"
#include "swcdb/ranger/db/RangeBlocks.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"
#include "swcdb/ranger/queries/select/CheckMeta.h"
#include "swcdb/ranger/queries/update/BaseMeta.h"


namespace SWC { namespace Ranger {


class Range final : public std::enable_shared_from_this<Range> {

  class MetaRegOnLoadReq;
  class MetaRegOnAddReq;

  struct TaskRunQueueScan;
  struct TaskRunQueueAdd;

  public:

  static constexpr const char CELLSTORES_BAK_DIR[]  = "cs_bak";
  static constexpr const char CELLSTORES_RCVD_DIR[] = "cs_rcvd";
  static constexpr const char CELLSTORES_TMP_DIR[]  = "cs_tmp";
  static constexpr const char LOG_TMP_DIR[]         = "log_tmp";

  enum State : uint8_t {
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

  const ColumnCfg::Ptr    cfg;
  const rid_t             rid;
  Blocks                  blocks;
  DB::Cell::Key           prev_range_end;

  Range(const ColumnCfg::Ptr& cfg, const rid_t rid);

  void init();

  ~Range() noexcept;

  std::string get_path(const std::string suff) const;

  std::string get_path_cs(const csid_t csid) const;

  std::string get_path_cs_on(const std::string folder,
                             const csid_t csid) const;

  void set_rgr(int &err) const noexcept;

  void remove_rgr(int &err) const noexcept;

  uint24_t known_interval_count();

  void get_interval(DB::Cells::Interval& interval);

  bool can_be_merged();

  void _get_interval(DB::Cells::Interval& interval) const;

  void _get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end) const;

  bool _is_any_begin() const;

  bool _is_any_end() const;

  void schema_update(bool compact);

  void set_state(State new_state);

  void reset_load_revision();

  int64_t get_load_revision();

  bool is_loaded();

  bool deleted();

  void state(int& err) const;

  bool state_unloading() const noexcept;

  void add(Callback::RangeQueryUpdate* req);

  void scan(ReqScan::Ptr&& req);

  void scan_internal(ReqScan::Ptr&& req);

  void load(const Callback::RangeLoad::Ptr& req);

  void internal_take_ownership(int &err, const Callback::RangeLoad::Ptr& req);

  void internal_unload(bool completely, bool& chk_empty);

  void issue_unload();

  void remove(const Callback::ColumnDelete::Ptr& req);

  void internal_remove(int &err);

  void wait_queue();

  bool compacting();

  bool compact_apply();

  void compacting(uint8_t state);

  bool compacting_ifnot_applying(uint8_t state);

  bool compact_possible(bool minor=true);

  void compact_require(bool require);

  bool compact_required();

  void apply_new(int &err,
                 CellStore::Writers& w_cellstores,
                 CommitLog::Fragments::Vec& fragments_old,
                 const Query::Update::BaseMeta::Ptr& hdlr=nullptr);

  void expand_and_align(bool w_chg_chk,
                        const Query::Update::BaseMeta::Ptr& hdlr);

  void internal_create_folders(int& err);

  void internal_create(int &err, const CellStore::Writers& w_cellstores);

  void internal_create(int &err, CellStore::Readers::Vec& mv_css);

  void check_meta(const Query::Select::CheckMeta::Ptr& hdlr);

  void print(std::ostream& out, bool minimal=true);

  private:

  void last_rgr_chk(int &err, const Callback::RangeLoad::Ptr& req);

  void load(int &err, const Callback::RangeLoad::Ptr& req);

  void loaded(int err, const Callback::RangeLoad::Ptr& req);

  void on_change(bool removal,
                 const Query::Update::BaseMeta::Ptr& hdlr,
                 const DB::Cell::Key* old_key_begin=nullptr);

  bool wait(uint8_t from_state=COMPACT_CHECKING, bool incr=false);

  void _run_scan_queue();

  void run_add_queue();

  void _run_add_queue();

  const std::string             m_path;
  Core::MutexAtomic             m_mutex_intval;
  Core::MutexAtomic             m_mutex_intval_alignment;
  DB::Cells::Interval           m_interval;
  Core::Atomic<int64_t>         m_load_revision;

  std::shared_mutex             m_mutex;

  Core::Atomic<State>           m_state;
  uint8_t                       m_compacting;
  Core::AtomicBool              m_require_compact;

  bool                          m_q_run_add;
  bool                          m_q_run_scan;

  Core::Atomic<uint32_t>                           m_adding;
  Core::QueuePointer<Callback::RangeQueryUpdate*>  m_q_add;
  Core::QueueSafe<ReqScan::Ptr>                    m_q_scan;

  std::condition_variable_any   m_cv;

  //Core::Atomic<size_t>          m_inbytes;
};


}}



#endif //swcdb_ranger_db_Range_h
