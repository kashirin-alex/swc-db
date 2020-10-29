/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Range_h
#define swcdb_ranger_db_Range_h

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

  static constexpr const char RANGE_FILE[]          = "range.data";
  static constexpr const char CELLSTORES_DIR[]      = "cs";
  static constexpr const char CELLSTORES_BAK_DIR[]  = "cs_bak";
  static constexpr const char CELLSTORES_TMP_DIR[]  = "cs_tmp";
  static constexpr const char LOG_DIR[]             = "log"; 
  static constexpr const char LOG_TMP_DIR[]         = "log_tmp"; 

  struct ReqAdd final {
    public:

    ReqAdd(StaticBuffer& input, const Callback::RangeQueryUpdate::Ptr& cb) 
          : input(input), cb(cb) {
      Env::Rgr::res().more_mem_usage(size_of());
    }

    ~ReqAdd() {
      Env::Rgr::res().less_mem_usage(size_of());
    }

    size_t size_of() const {
      return sizeof(*this) + sizeof(this) + 
             sizeof(*cb.get()) + input.size;
    }

    StaticBuffer                          input;
    const Callback::RangeQueryUpdate::Ptr cb;

  };

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

  ~Range();

  size_t size_of() const;

  const std::string get_path(const std::string suff) const;

  const std::string get_path_cs(const csid_t csid) const;

  const std::string get_path_cs_on(const std::string folder, 
                                   const csid_t csid) const;

  Common::Files::RgrData::Ptr get_last_rgr(int &err);

  void get_interval(DB::Cells::Interval& interval);

  void get_interval(DB::Cell::Key& key_begin, DB::Cell::Key& key_end);

  void get_key_end(DB::Cell::Key& key);
  
  bool is_any_begin();

  bool is_any_end();

  uint24_t known_interval_count();

  bool align(const DB::Cells::Interval& interval);
  
  bool align(const DB::Cell::Key& key);

  void schema_update(bool compact);

  void set_state(State new_state);

  bool is_loaded();

  bool deleted();

  void state(int& err) const;

  void add(ReqAdd* req);

  void scan(const ReqScan::Ptr& req);

  void scan_internal(const ReqScan::Ptr& req);

  void load(const Callback::RangeLoad::Ptr& req);

  void internal_take_ownership(int &err, const Callback::RangeLoad::Ptr& req);

  void internal_unload(bool completely);

  void remove(const Callback::ColumnDelete::Ptr& req);

  void internal_remove(int &err, bool meta=true);

  void wait_queue();

  bool compacting();

  bool compacting_is(uint8_t state);

  void compacting(uint8_t state);
  
  bool compact_possible(bool minor=true);

  void compact_require(bool require);

  bool compact_required();

  void on_change(int &err, bool removal, 
                 const DB::Cell::Key* old_key_begin=nullptr,
                 const client::Query::Update::Cb_t& cb=0);

  void apply_new(int &err,
                CellStore::Writers& w_cellstores, 
                CommitLog::Fragments::Vec& fragments_old, 
                bool w_update);
  
  void expand_and_align(int &err, bool w_chg_chk);
  
  void internal_create_folders(int& err);

  void internal_create(int &err, const CellStore::Writers& w_cellstores);

  void internal_create(int &err, CellStore::Readers::Vec& mv_css);

  void print(std::ostream& out, bool minimal=true);

  private:

  void last_rgr_chk(int &err, const Callback::RangeLoad::Ptr& req);

  void load(int &err, const Callback::RangeLoad::Ptr& req);
  
  void loaded(int err, const Callback::RangeLoad::Ptr& req);

  bool wait(uint8_t from_state=COMPACT_CHECKING);

  void run_add_queue();

  const std::string             m_path;
  Core::MutexAtomic             m_mutex_intval;
  DB::Cells::Interval           m_interval;

  std::shared_mutex             m_mutex;

  std::atomic<State>            m_state;
  uint8_t                       m_compacting;
  bool                          m_require_compact;

  Core::QueueSafe<ReqScan::Ptr> m_q_scans;
  Core::QueueSafe<ReqAdd*>      m_q_adding;

  std::condition_variable_any   m_cv;

  std::atomic<size_t>           m_inbytes;
};


}}



#endif //swcdb_ranger_db_Range_h
