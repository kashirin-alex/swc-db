/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/SystemColumn.h"
#include "swcdb/ranger/db/Range.h"

#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/Cells/SpecsValueSerialFields.h"
#include "swcdb/ranger/queries/update/CommonMeta.h"


namespace SWC { namespace Ranger {



class Range::MetaRegOnLoadReq : public Query::Update::BaseMeta {
  public:
  typedef std::shared_ptr<MetaRegOnLoadReq>     Ptr;

  SWC_CAN_INLINE
  static Ptr make(const RangePtr& range,
                  const Callback::RangeLoad::Ptr& req) {
    return Ptr(new MetaRegOnLoadReq(range, req));
  }

  Callback::RangeLoad::Ptr req;

  SWC_CAN_INLINE
  MetaRegOnLoadReq(const RangePtr& a_range,
                   const Callback::RangeLoad::Ptr& a_req)
                  : Query::Update::BaseMeta(a_range), req(a_req) {
  }

  virtual ~MetaRegOnLoadReq() { }

  virtual void response(int err=Error::OK) override {
    struct Task {
      Ptr ptr;
      SWC_CAN_INLINE
      Task(Ptr&& a_ptr) noexcept : ptr(std::move(a_ptr)) { }
      void operator()() { ptr->callback(); }
    };
    if(is_last_rsp(err)) {
      Env::Rgr::post(Task(
        std::dynamic_pointer_cast<MetaRegOnLoadReq>(shared_from_this())));
    }
  }

  virtual void callback() override {
    range->loaded(error(), req);
  }

};



const char* to_string(Range::State state) noexcept {
  switch(state) {
    case Range::State::NOTLOADED:
      return "NOTLOADED";
    case Range::State::LOADING:
      return "LOADING";
    case Range::State::LOADED:
      return "LOADED";
    case Range::State::UNLOADING:
      return "UNLOADING";
    case Range::State::DELETED:
      return "DELETED";
    default:
      return "UNKNOWN";
  }
}


struct Range::TaskRunQueueScan {
  RangePtr ptr;
  SWC_CAN_INLINE
  TaskRunQueueScan(RangePtr&& a_ptr) noexcept : ptr(std::move(a_ptr)) { }
  void operator()() { ptr->_run_scan_queue(); }
};

struct Range::TaskRunQueueAdd {
  RangePtr ptr;
  SWC_CAN_INLINE
  TaskRunQueueAdd(const RangePtr& a_ptr) noexcept : ptr(a_ptr) { }
  TaskRunQueueAdd(RangePtr&& a_ptr) noexcept : ptr(std::move(a_ptr)) { }
  void operator()() { ptr->_run_add_queue(); }
};


SWC_CAN_INLINE
Range::Range(const ColumnCfg::Ptr& a_cfg, const rid_t a_rid)
            : cfg(a_cfg), rid(a_rid),
              blocks(cfg->key_seq),
              m_path(DB::RangeBase::get_path(cfg->cid, rid)),
              m_interval(cfg->key_seq), m_load_revision(0),
              m_state(State::NOTLOADED),
              m_compacting(COMPACT_NONE), m_require_compact(false),
              m_q_run_add(false), m_q_run_scan(false),
              m_adding(0) { //, m_inbytes(0)
  Env::Rgr::in_process_ranges(1);
}

SWC_CAN_INLINE
void Range::init() {
  blocks.init(shared_from_this());
}

Range::~Range() {
  Env::Rgr::in_process_ranges(-1);
}

std::string Range::get_path(const std::string suff) const {
  std::string s;
  s.reserve(m_path.length() + suff.length());
  s.append(m_path);
  s.append(suff);
  return s;
}

SWC_CAN_INLINE
std::string Range::get_path_cs(const csid_t csid) const {
  return get_path_cs_on(DB::RangeBase::CELLSTORES_DIR, csid);
}

SWC_CAN_INLINE
std::string Range::get_path_cs_on(const std::string folder,
                                  const csid_t csid) const {
  return DB::RangeBase::get_path_cs(m_path, folder, csid);
}

void Range::set_rgr(int& err) const noexcept {
  DB::Types::SystemColumn::is_rgr_data_on_fs(cfg->cid)
    ? Common::Files::RgrData::set_rgr(
        *Env::Rgr::rgr_data(),
        err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication()
      )
    : Env::Rgr::rgr_data()->set_rgr(err, cfg->cid, rid);
}

void Range::remove_rgr(int& err) const noexcept {
  DB::Types::SystemColumn::is_rgr_data_on_fs(cfg->cid)
    ? Env::FsInterface::interface()->remove(
        err, DB::RangeBase::get_path_ranger(m_path))
    : DB::RgrData::remove(err, cfg->cid, rid);
}

SWC_CAN_INLINE
uint24_t Range::known_interval_count() {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  return m_interval.key_end.empty()
          ? m_interval.key_begin.count
          : m_interval.key_end.count;
}

SWC_CAN_INLINE
void Range::get_interval(DB::Cells::Interval& interval) {
  Core::MutexAtomic::scope lock(m_mutex_intval);
  Core::MutexAtomic::scope lock_align(m_mutex_intval_alignment);
  _get_interval(interval);
}

bool Range::can_be_merged() {
  {
    Core::MutexAtomic::scope lock(m_mutex_intval);
    if(_is_any_begin())
      return false;
  }
  if(!blocks.commitlog.empty())
    return false;
  Core::SharedLock lock(m_mutex);
  return m_state == State::LOADED &&
         m_compacting == COMPACT_NONE &&
        !blocks.cellstores.size_bytes(false);
}

SWC_CAN_INLINE
void Range::_get_interval(DB::Cells::Interval& interval) const {
  interval.copy(m_interval);
}

SWC_CAN_INLINE
void Range::_get_interval(DB::Cell::Key& key_begin,
                          DB::Cell::Key& key_end) const {
  key_begin.copy(m_interval.key_begin);
  key_end.copy(m_interval.key_end);
}

SWC_CAN_INLINE
bool Range::_is_any_begin() const {
  return m_interval.key_begin.empty();
}

SWC_CAN_INLINE
bool Range::_is_any_end() const {
  return m_interval.key_end.empty();
}

SWC_CAN_INLINE
void Range::schema_update(bool compact) {
  blocks.schema_update();
  if(compact)
    compact_require(true);
}

void Range::set_state(Range::State new_state) {
  Core::ScopedLock lock(m_mutex);
  m_state.store(new_state);
}

SWC_CAN_INLINE
void Range::reset_load_revision() {
  m_load_revision.store(Time::now_ns());
}

SWC_CAN_INLINE
int64_t Range::get_load_revision() {
  return m_load_revision.load();
}

SWC_CAN_INLINE
bool Range::is_loaded() {
  Core::SharedLock lock(m_mutex);
  return m_state == State::LOADED;
}

SWC_CAN_INLINE
bool Range::deleted() {
  Core::SharedLock lock(m_mutex);
  return m_state == State::DELETED;
}

SWC_CAN_INLINE
void Range::state(int& err) const {
  if(m_state != State::LOADED) {
    err = m_state == State::DELETED
      ? Error::COLUMN_MARKED_REMOVED
      : Error::RGR_NOT_LOADED_RANGE;
  }
}

SWC_CAN_INLINE
bool Range::state_unloading() const noexcept {
  return Env::Rgr::is_shuttingdown() ||
         (Env::Rgr::is_not_accepting() &&
          DB::Types::SystemColumn::is_data(cfg->cid));
}

SWC_CAN_INLINE
void Range::add(Callback::RangeQueryUpdate* req) {
  m_q_add.push(req);
  run_add_queue();
}

void Range::scan(ReqScan::Ptr&& req) {
  {
    Core::ScopedLock lock(m_mutex);
    if(m_compacting == COMPACT_APPLYING) {
      m_q_run_scan = true;
      m_q_scan.push(std::move(req));
      return;
    }
    blocks.processing_increment();
  }
  blocks.scan(std::move(req));
  blocks.processing_decrement();
}

void Range::_run_scan_queue() {
  {
    Core::ScopedLock lock(m_mutex);
    if(m_compacting == COMPACT_APPLYING) {
      m_q_run_scan = true;
      return;
    }
    blocks.processing_increment();
  }

  struct Task {
    RangePtr     ptr;
    ReqScan::Ptr req;
    SWC_CAN_INLINE
    Task(RangePtr&& a_ptr, ReqScan::Ptr&& a_req) noexcept
        : ptr(std::move(a_ptr)), req(std::move(a_req)) {
      ptr->blocks.processing_increment();
    }
    void operator()() {
      ptr->blocks.scan(std::move(req));
      ptr->blocks.processing_decrement();
    }
  };

  for(ReqScan::Ptr req; m_q_scan.pop(&req); ) {
    if(!req->expired()) {
      int err = Error::OK;
      state(err);
      if(err) {
        req->response(err);
      } else {
        Env::Rgr::post(Task(shared_from_this(), std::move(req)));
      }
    }
  }
  blocks.processing_decrement();
}

SWC_CAN_INLINE
void Range::scan_internal(ReqScan::Ptr&& req) {
  blocks.scan(std::move(req));
}

void Range::load(const Callback::RangeLoad::Ptr& req) {
  blocks.processing_increment();

  bool need;
  {
    auto at(State::NOTLOADED);
    Core::ScopedLock lock(m_mutex);
    need = m_state.compare_exchange_weak(at, State::LOADING);
  }

  int err = state_unloading() ? Error::SERVER_SHUTTING_DOWN : Error::OK;
  if(need && !err) {
    SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-STARTED", cfg->cid, rid);

    if(!Env::FsInterface::interface()->exists(
          err, get_path(DB::RangeBase::CELLSTORES_DIR)) && !err &&
       !Env::FsInterface::interface()->exists(
          err, get_path(Range::CELLSTORES_BAK_DIR)) && !err) {
      internal_create_folders(err);
      if(!err)
        return internal_take_ownership(err, req);
    } else if(!err) {
      return last_rgr_chk(err, req);
    }
  }
  return loaded(err, req);
}

void Range::internal_take_ownership(int &err, const Callback::RangeLoad::Ptr& req) {
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-TAKE OWNERSHIP", cfg->cid, rid);

  if(state_unloading())
    return loaded(Error::SERVER_SHUTTING_DOWN, req);

  if(DB::Types::SystemColumn::is_rgr_data_on_fs(cfg->cid)) {
    Common::Files::RgrData::set_rgr(
      *Env::Rgr::rgr_data(),
      err, DB::RangeBase::get_path_ranger(m_path), cfg->file_replication()
    );
    err ? loaded(err, req) : load(err, req);
    return;
  }

  class SetRgr : public DB::RgrData::BaseUpdater {
    public:
    typedef std::shared_ptr<SetRgr> Ptr;
    RangePtr                        range;
    Callback::RangeLoad::Ptr        req;
    SetRgr(RangePtr&& a_range,
           const Callback::RangeLoad::Ptr& a_req) noexcept
          : range(std::move(a_range)), req(a_req) { }
    virtual ~SetRgr() { }
    void callback() override {
      struct Task {
        SetRgr::Ptr ptr;
        SWC_CAN_INLINE
        Task(SetRgr::Ptr&& a_ptr) noexcept : ptr(std::move(a_ptr)) { }
        void operator()() {
          int err = ptr->error();
          err
            ? ptr->range->loaded(err, ptr->req)
            : ptr->range->load(err, ptr->req);
        }
      };
      Env::Rgr::post(Task(
        std::dynamic_pointer_cast<SetRgr>(shared_from_this())));
    }
  };
  SetRgr::Ptr(new SetRgr(shared_from_this(), req))
    ->set_rgr(*Env::Rgr::rgr_data(), cfg->cid, rid);
}

void Range::internal_unload(bool completely, bool& chk_empty) {
  {
    Core::ScopedLock lock(m_mutex);
    if(m_state != State::LOADED && !blocks.range)
      return;
    m_state.store(State::UNLOADING);
  }
  SWC_LOGF(LOG_DEBUG, "UNLOADING RANGE(%lu/%lu)", cfg->cid, rid);

  blocks.commitlog.stopping.store(true);

  wait();
  wait_queue();

  if(chk_empty)
    chk_empty = !blocks.size_bytes_total(false);
  blocks.unload();

  int err = Error::OK;
  if(completely) // whether to keep RANGER_FILE
    remove_rgr(err);

  {
    Core::ScopedLock lock(m_mutex);
    m_state.store(State::NOTLOADED);
  }
  SWC_LOGF(LOG_INFO, "UNLOADED RANGE(%lu/%lu) error=%d(%s)",
                      cfg->cid, rid, err, Error::get_text(err));
}

void Range::issue_unload() {
  auto col = Env::Rgr::columns()->get_column(cfg->cid);
  if(col && !col->removing())
    col->add_managing(Callback::RangeUnloadInternal::Ptr(
      new Callback::RangeUnloadInternal(cfg->cid, rid)));
}

void Range::remove(const Callback::ColumnDelete::Ptr& req) {
  {
    Core::ScopedLock lock(m_mutex);
    if(m_state.exchange(State::DELETED) == State::DELETED)
      return req->removed(shared_from_this());
  }
  SWC_LOGF(LOG_DEBUG, "REMOVING RANGE(%lu/%lu)", cfg->cid, rid);

  blocks.commitlog.stopping.store(true);

  on_change(
    true,
    Query::Update::CommonMeta::make(
      shared_from_this(),
      [req] (const Query::Update::CommonMeta::Ptr& hdlr) {
        int err = Error::OK;

        hdlr->range->wait();
        hdlr->range->wait_queue();
        hdlr->range->blocks.remove(err);

        Env::FsInterface::interface()->rmdir(err, hdlr->range->get_path(""));
        /* Manager clears all column ranges
        if(!DB::Types::SystemColumn::is_rgr_data_on_fs(hdlr->range->cfg->cid))
          DB::RgrData::remove(err, hdlr->range->cfg->cid, hdlr->range->rid);
        */
        req->removed(hdlr->range);
      }
    )
  );
}

void Range::internal_remove(int& err) {
  {
    Core::ScopedLock lock(m_mutex);
    if(m_state.exchange(State::DELETED) == State::DELETED)
      return;
  }
  SWC_LOGF(LOG_DEBUG, "REMOVING RANGE(%lu/%lu)", cfg->cid, rid);

  blocks.commitlog.stopping.store(true);

  wait();
  wait_queue();
  blocks.remove(err);

  Env::FsInterface::interface()->rmdir(err, get_path(""));

  SWC_LOG_OUT(LOG_INFO, print(SWC_LOG_OSTREAM << "REMOVED RANGE "); );
}

void Range::wait_queue() {
  while(m_adding || !m_q_add.empty() || !m_q_scan.empty())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

SWC_CAN_INLINE
bool Range::compacting() {
  Core::SharedLock lock(m_mutex);
  return m_compacting != COMPACT_NONE;
}

void Range::compacting(uint8_t state) {
  bool do_q_run_add;
  bool do_q_run_scan;
  {
    Core::ScopedLock lock(m_mutex);
    m_compacting = state;
    m_cv.notify_all();
    if(m_compacting == COMPACT_APPLYING)
      return;

    if((do_q_run_add = m_q_run_add && state < COMPACT_PREPARING))
      m_q_run_add = false;
    if((do_q_run_scan = m_q_run_scan))
      m_q_run_scan = false;
  }
  if(do_q_run_add)
    run_add_queue();
  if(do_q_run_scan)
    Env::Rgr::post(TaskRunQueueScan(shared_from_this()));
}

bool Range::compacting_ifnot_applying(uint8_t state) {
  bool do_q_run_add;
  bool do_q_run_scan;
  {
    Core::ScopedLock lock(m_mutex);
    if(m_compacting == COMPACT_APPLYING)
      return false;
    m_compacting = state;
    m_cv.notify_all();

    if((do_q_run_add = m_q_run_add && state < COMPACT_PREPARING))
      m_q_run_add = false;
    if((do_q_run_scan = m_q_run_scan))
      m_q_run_scan = false;
  }
  if(do_q_run_add)
    run_add_queue();
  if(do_q_run_scan)
    Env::Rgr::post(TaskRunQueueScan(shared_from_this()));
  return true;
}

bool Range::compact_possible(bool minor) {
  Core::ScopedLock lock(m_mutex);
  if(m_state != State::LOADED || m_compacting != COMPACT_NONE ||
     (!minor && !m_require_compact && blocks.processing()))
    return false;
  m_compacting = COMPACT_CHECKING;
  return true;
}

SWC_CAN_INLINE
void Range::compact_require(bool require) {
  m_require_compact.store(require);
}

SWC_CAN_INLINE
bool Range::compact_required() {
  return m_require_compact;
}

void Range::on_change(bool removal,
                      const Query::Update::BaseMeta::Ptr& hdlr,
                      const DB::Cell::Key* old_key_begin) {
  if(cfg->range_type == DB::Types::Range::MASTER) {
    // update manager-root
    // Mngr::RangeUpdated
    hdlr->callback();
    return;
  }

  DB::Cells::Cell cell;
  auto cid_f(std::to_string(cfg->cid));

  if(removal) {
    cell.flag = DB::Cells::DELETE;
    cell.key.copy(m_interval.key_begin);
    cell.key.insert(0, cid_f);
    hdlr->column.add(cell);

  } else {
    cell.flag = DB::Cells::INSERT;

    cell.key.copy(m_interval.key_begin);
    cell.key.insert(0, cid_f);

    DB::Cell::Key key_end(m_interval.key_end);
    key_end.insert(0, cid_f);

    DB::Cell::Key aligned_min;
    DB::Cell::Key aligned_max;
    if(cfg->range_type == DB::Types::Range::DATA) {
      aligned_min.add(cid_f);
      aligned_max.add(cid_f);
      // only DATA until MASTER/META aligned on cells value min/max
      Core::MutexAtomic::scope lock_align(m_mutex_intval_alignment);
      aligned_min.add(m_interval.aligned_min);
      aligned_max.add(m_interval.aligned_max);
    }

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(
      key_end.encoded_length()
       + Serialization::encoded_length_vi64(rid)
       + aligned_min.encoded_length()
       + aligned_max.encoded_length()
      + 8);
    uint24_t fid = 0;
    wfields.add(fid, key_end);
    wfields.add(fid, int64_t(rid));
    wfields.add(++fid, aligned_min);
    wfields.add(++fid, aligned_max);
    cell.set_value(wfields.base, wfields.fill(), false);

    cell.set_time_order_desc(true);
    hdlr->column.add(cell);

    if(old_key_begin && !old_key_begin->equal(m_interval.key_begin)) {
      SWC_ASSERT(!old_key_begin->empty());
      // remove begin-any should not happen

      cell.free();
      cell.flag = DB::Cells::DELETE;
      cell.key.copy(*old_key_begin);
      cell.key.insert(0, std::to_string(cfg->cid));
      hdlr->column.add(cell);
    }
  }

  hdlr->commit(&hdlr->column);
  /* INSERT master-range(
      col-{1,4}), key[cid+m_interval(data(cid)+key)], value[end, rid, min, max]
     INSERT meta-range(
       col-{5,8}), key[cid+m_interval(key)], value[end, rid, min, max]
  */
}

void Range::apply_new(int &err,
                      CellStore::Writers& w_cellstores,
                      CommitLog::Fragments::Vec& fragments_old,
                      const Query::Update::BaseMeta::Ptr& hdlr) {
  {
    Core::ScopedLock lock(m_mutex);
    blocks.apply_new(err, w_cellstores, fragments_old);
    if(err)
      return;
  }
  if(hdlr)
    expand_and_align(true, hdlr);
}

void Range::expand_and_align(bool w_chg_chk,
                             const Query::Update::BaseMeta::Ptr& hdlr) {
  DB::Cell::Key     old_key_begin;
  DB::Cell::Key     key_end;
  DB::Cell::KeyVec  aligned_min;
  DB::Cell::KeyVec  aligned_max;

  if(w_chg_chk) {
    old_key_begin.copy(m_interval.key_begin);
    key_end.copy(m_interval.key_end);
    aligned_min.copy(m_interval.aligned_min);
    aligned_max.copy(m_interval.aligned_max);
  }

  {
    Core::MutexAtomic::scope lock(m_mutex_intval);
    Core::MutexAtomic::scope lock_align(m_mutex_intval_alignment);
    m_interval.free();
    if(cfg->range_type == DB::Types::Range::DATA)
      blocks.expand_and_align(m_interval);
    else
      blocks.expand(m_interval);
  }

  if(!w_chg_chk ||
     !m_interval.key_begin.equal(old_key_begin) ||
     !m_interval.key_end.equal(key_end) ||
     !m_interval.aligned_min.equal(aligned_min) ||
     !m_interval.aligned_max.equal(aligned_max)) {
    on_change(false, hdlr, w_chg_chk ? &old_key_begin : nullptr);
    reset_load_revision();
  } else {
    hdlr->callback();
  }
}

SWC_CAN_INLINE
void Range::internal_create_folders(int& err) {
  Env::FsInterface::interface()->mkdirs(
    err, get_path(DB::RangeBase::LOG_DIR));
  Env::FsInterface::interface()->mkdirs(
    err, get_path(LOG_TMP_DIR));
  Env::FsInterface::interface()->mkdirs(
    err, get_path(DB::RangeBase::CELLSTORES_DIR));
}

void Range::internal_create(int &err, const CellStore::Writers& w_cellstores) {
  set_rgr(err);
  if(err)
    return;

  const auto& fs_if = Env::FsInterface::interface();
  blocks.cellstores.reserve(w_cellstores.size());
  for(auto& cs : w_cellstores) {
    fs_if->rename(
      err,
      cs->smartfd->filepath(),
      get_path_cs(cs->csid)
    );
    if(err)
      return;

    blocks.cellstores.add(
      CellStore::Read::make(err, cs->csid, shared_from_this(), true)
    );
    if(err)
      return;
  }

  #ifdef SWC_RANGER_WITH_RANGEDATA
  RangeData::save(err, blocks.cellstores);
  #endif
  remove_rgr(err);
  err = Error::OK;
}

void Range::internal_create(int &err, CellStore::Readers::Vec& mv_css) {
  set_rgr(err);
  if(err)
    return;

  blocks.cellstores.move_from(err, mv_css);
  if(mv_css.empty())
    err = Error::CANCELLED;
  if(!err) {
    #ifdef SWC_RANGER_WITH_RANGEDATA
    RangeData::save(err, blocks.cellstores);
    #endif
    remove_rgr(err);
    err = Error::OK;
  }
}

void Range::print(std::ostream& out, bool minimal) {
  cfg->print(out << '(');
  out << " rid=" << rid << " state=" << to_string(m_state);
  blocks.print(out << ' ', minimal);
  prev_range_end.print(out << " prev=");
  {
    Core::MutexAtomic::scope lock(m_mutex_intval);
    Core::MutexAtomic::scope lock_align(m_mutex_intval_alignment);
    m_interval.print(out << ' ');
    out << " revision=" << m_load_revision;
  }
  out << ')';
}

void Range::last_rgr_chk(int &err, const Callback::RangeLoad::Ptr& req) {
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-CHECK LAST RGR", cfg->cid, rid);

  if(DB::Types::SystemColumn::is_rgr_data_on_fs(cfg->cid)) {
    DB::RgrData rgr_last;
    Common::Files::RgrData::get_rgr(
      rgr_last, DB::RangeBase::get_path_ranger(m_path));
    if(!rgr_last.endpoints.empty()) {
      if(Comm::equal_endpoints(
          Env::Rgr::rgr_data()->endpoints, rgr_last.endpoints)) {
        return load(err, req);
      }
      if(!Comm::has_endpoint(
          Env::Rgr::rgr_data()->endpoints, rgr_last.endpoints)) {
        SWC_LOG_OUT(LOG_DEBUG,
          rgr_last.print(SWC_LOG_OSTREAM << "RANGER LAST=");
          Env::Rgr::rgr_data()->print(SWC_LOG_OSTREAM << " NEW=");
        );
        Env::Clients::get()->get_rgr_queue(rgr_last.endpoints)->put(
          Comm::Protocol::Rgr::Req::RangeUnload::Ptr(
            new Comm::Protocol::Rgr::Req::RangeUnload(shared_from_this(), req)
          )
        );
        return;
      }
    }
    return internal_take_ownership(err, req);
  }

  class GetRgr : public DB::RgrData::BaseSelector {
    public:
    typedef std::shared_ptr<GetRgr>   Ptr;
    RangePtr                          range;
    Callback::RangeLoad::Ptr          req;
    SWC_CAN_INLINE
    GetRgr(RangePtr&& a_range, const Callback::RangeLoad::Ptr& a_req)
           noexcept : range(std::move(a_range)), req(a_req) { }
    void callback() override {
      struct Task {
        GetRgr::Ptr ptr;
        SWC_CAN_INLINE
        Task(GetRgr::Ptr&& a_ptr) noexcept : ptr(std::move(a_ptr)) { }
        void operator()() {
          int err = Error::OK;
          DB::RgrData rgr_last;
          ptr->get_rgr(err, rgr_last);
          if(err && err != ENOKEY)
            return ptr->range->loaded(err, ptr->req);
          if(!rgr_last.endpoints.empty()) {
            if(Comm::equal_endpoints(
              Env::Rgr::rgr_data()->endpoints, rgr_last.endpoints)) {
              return ptr->range->load(err, ptr->req);
            }
            if(!Comm::has_endpoint(
                Env::Rgr::rgr_data()->endpoints, rgr_last.endpoints)) {
              SWC_LOG_OUT(LOG_DEBUG,
                rgr_last.print(SWC_LOG_OSTREAM << "RANGER LAST=");
                Env::Rgr::rgr_data()->print(SWC_LOG_OSTREAM << " NEW=");
              );
              Env::Clients::get()->get_rgr_queue(rgr_last.endpoints)->put(
                Comm::Protocol::Rgr::Req::RangeUnload::Ptr(
                  new Comm::Protocol::Rgr::Req::RangeUnload(
                    ptr->range, ptr->req)));
              return;
            }
          }
          return ptr->range->internal_take_ownership(err, ptr->req);
        }
      };
      Env::Rgr::post(Task(
        std::dynamic_pointer_cast<GetRgr>(shared_from_this())));
    }
  };
  GetRgr::Ptr(new GetRgr(shared_from_this(), req))->scan(cfg->cid, rid);
}

void Range::load(int &err, const Callback::RangeLoad::Ptr& req) {
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-DATA", cfg->cid, rid);

  if(state_unloading())
    return loaded(Error::SERVER_SHUTTING_DOWN, req);

  bool is_initial_column_range = false;
  #ifdef SWC_RANGER_WITH_RANGEDATA
  RangeData::load(err, blocks.cellstores);
  #else
  blocks.cellstores.load_from_path(err);
  #endif
  if(err) {
    (void)err;
    //err = Error::OK; // ranger-to determine range-removal (+ Notify Mngr)

  } else if(blocks.cellstores.empty()) {
    // init 1st cs(for log_cells)
    auto cs = CellStore::create_initial(err, shared_from_this());
    if(!err) {
      blocks.cellstores.add(cs);
      is_initial_column_range = true;
    }
  }

  if(!err)
    blocks.load(err);

  if(err)
    return loaded(err, req);

  blocks.cellstores.get_prev_key_end(0, prev_range_end);
  {
    Core::MutexAtomic::scope lock(m_mutex_intval);
    Core::MutexAtomic::scope lock_align(m_mutex_intval_alignment);
    m_interval.free();
    if(cfg->range_type == DB::Types::Range::DATA)
      blocks.expand_and_align(m_interval);
    else
      blocks.expand(m_interval);
  }

  if(is_initial_column_range) {
    #ifdef SWC_RANGER_WITH_RANGEDATA
    RangeData::save(err, blocks.cellstores);
    #endif
    return on_change(false, MetaRegOnLoadReq::make(shared_from_this(), req));
  }

  if(cfg->range_type == DB::Types::Range::MASTER)
    return loaded(err, req);

  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-CHECK META", cfg->cid, rid);
  Query::Select::CheckMeta::run(shared_from_this(), req);
}

void Range::check_meta(const Query::Select::CheckMeta::Ptr& hdlr) {
  int err = hdlr->state_error;
  SWC_LOGF(LOG_DEBUG, "LOADING RANGE(%lu/%lu)-CHECK META err=%d(%s)",
                      cfg->cid, rid, err, Error::get_text(err));
  if(err)
    return loaded(err, hdlr->req);
  if(state_unloading())
    return loaded(Error::SERVER_SHUTTING_DOWN, hdlr->req);

  if(hdlr->empty()) {
    SWC_LOG_OUT(LOG_ERROR,
      SWC_LOG_OSTREAM
        << "Range MetaData missing cid=" << cfg->cid << " rid=" << rid;
      m_interval.print(
        SWC_LOG_OSTREAM << "\n\tauto-registering=");
      hdlr->spec.print(
        SWC_LOG_OSTREAM << "\n\tmeta-cid(" << hdlr->cid << ")=");
    );
    return on_change(
      false, MetaRegOnLoadReq::make(shared_from_this(), hdlr->req));
  }

  DB::Cells::Result cells;
  hdlr->get_cells(cells);

  rid_t _rid = 0;
  bool synced = cells.size() == 1;
  if(!synced) {
    SWC_LOG_OUT(LOG_ERROR,
      SWC_LOG_OSTREAM
        << "Range MetaData DUPLICATE-RID cid=" << cfg->cid << " rid=" << rid;
      m_interval.print(SWC_LOG_OSTREAM << "\n\tauto-del-registering=");
      hdlr->spec.print(
        SWC_LOG_OSTREAM << "\n\tmeta-cid(" << hdlr->cid << ")=");
      cells.print(SWC_LOG_OSTREAM << "\n\t", DB::Types::Column::SERIAL, true);
    );
  } else { try {

    auto& cell = *cells[0];

    DB::Cells::Interval interval(cfg->key_seq);
    interval.was_set = true;
    /* Range MetaData does not include timestamp
        for the comparison use current interval ts */
    interval.ts_earliest.copy(m_interval.ts_earliest);
    interval.ts_latest.copy(m_interval.ts_latest);

    interval.key_begin.copy(cell.key);
    interval.key_begin.remove(0);

    StaticBuffer v;
    cell.get_value(v);
    const uint8_t* ptr = v.base;
    size_t remain = v.size;

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    interval.key_end.decode(&ptr, &remain, false);
    interval.key_end.remove(0);

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    _rid = Serialization::decode_vi64(&ptr, &remain);

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    interval.aligned_min.decode(&ptr, &remain);
    interval.aligned_min.remove(0);

    DB::Cell::Serial::Value::skip_type_and_id(&ptr, &remain);
    interval.aligned_max.decode(&ptr, &remain);
    interval.aligned_max.remove(0);

    synced = !remain && rid == _rid && m_interval.equal(interval);
    if(!synced) {
      SWC_LOG_OUT(LOG_ERROR,
        SWC_LOG_OSTREAM << "Range MetaData NOT-SYNCED cid=" << cfg->cid;
        SWC_LOG_OSTREAM << "\n\t    loaded-rid=" << rid;
        m_interval.print(SWC_LOG_OSTREAM << ' ');
        SWC_LOG_OSTREAM << "\n\tregistered-rid=" << _rid;
        interval.print(SWC_LOG_OSTREAM << ' ');
      );
    }
  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
    synced = false;
  } }

  if(synced)
    return loaded(err, hdlr->req);

  auto updater = MetaRegOnLoadReq::make(shared_from_this(), hdlr->req);
  for(auto cell : cells) {
    cell->flag = DB::Cells::DELETE;
    cell->free();
    updater->column.add(*cell);
  }
  on_change(false, updater);
}

void Range::loaded(int err, const Callback::RangeLoad::Ptr& req) {
  if(!err && state_unloading())
    err = Error::SERVER_SHUTTING_DOWN;
  bool tried;
  {
    Core::ScopedLock lock(m_mutex);
    tried = m_state == State::LOADING;
    if(tried)
      m_state.store(err ? State::NOTLOADED : State::LOADED);
  }
  if(tried && !err)
    reset_load_revision();

  SWC_LOG_OUT((is_loaded() ? LOG_INFO : LOG_WARN),
    if(!tried)
      SWC_LOG_OSTREAM << "CHECK ";
    SWC_LOG_OSTREAM << "LOAD RANGE ";
    if(_log_pr == LOG_INFO)
      SWC_LOG_OSTREAM << "SUCCEED";
    else
      Error::print(SWC_LOG_OSTREAM << "FAILED ", err);
    print(SWC_LOG_OSTREAM << ' ', _log_pr == LOG_INFO &&
                                  err != Error::SERVER_SHUTTING_DOWN &&
                                  err != Error::RGR_NOT_LOADED_RANGE);
  );

  blocks.processing_decrement();
  req->loaded(err);
}

bool Range::wait(uint8_t from_state, bool incr) {
  bool waited;
  Core::UniqueLock lock_wait(m_mutex);
  if((waited = (m_compacting >= from_state))) {
    m_cv.wait(
      lock_wait,
      [this, from_state]() {
        return m_compacting < from_state;
      }
    );
  }
  if(incr)
    blocks.processing_increment();
  return waited;
}

void Range::run_add_queue() {
  if(m_adding.fetch_add(1) < Env::Rgr::get()->cfg_req_add_concurrency->get())
    Env::Rgr::post(TaskRunQueueAdd(shared_from_this()));
  else
    m_adding.fetch_sub(1);
}



class Range::MetaRegOnAddReq : public Query::Update::BaseMeta {
  public:
  typedef std::shared_ptr<MetaRegOnAddReq>     Ptr;

  SWC_CAN_INLINE
  static Ptr make(const RangePtr& range, Callback::RangeQueryUpdate* req) {
    return Ptr(new MetaRegOnAddReq(range, req));
  }

  Callback::RangeQueryUpdate* req;

  SWC_CAN_INLINE
  MetaRegOnAddReq(const RangePtr& a_range, Callback::RangeQueryUpdate* a_req)
                : Query::Update::BaseMeta(a_range), req(a_req) {
  }

  virtual ~MetaRegOnAddReq() {
    bool added = req->rsp.cells_added;
    delete req;
    if(added)
      range->blocks.commitlog.commit();
  }

  virtual void response(int err=Error::OK) override {
    if(is_last_rsp(err))
      callback();
  }

  virtual void callback() override {
    range->blocks.processing_decrement();
    Env::Rgr::post(TaskRunQueueAdd(range));
    if(!req->rsp.err && error() != Error::SERVER_SHUTTING_DOWN)
      req->rsp.err = error();
    req->response();
  }

};


void Range::_run_add_queue() {
  const uint64_t ttl = cfg->cell_ttl();

  DB::Cell::KeyVec align_min;
  DB::Cell::KeyVec align_max;

  while(!m_q_add.empty()) {
    {
      Core::ScopedLock lock(m_mutex);
      if(m_compacting >= COMPACT_PREPARING) {
        m_q_run_add = true;
        m_adding.fetch_sub(1);
        return;
      }
      blocks.processing_increment();
      // Compaction won't change m_interval while processing positive
      // _run_add_queue is not running at COMPACT_PREPARING+
    }

    Callback::RangeQueryUpdate* req = m_q_add.next();
    if(!req) {
      blocks.processing_decrement();
      break;
    }

    if(req->expired()) {
      req->rsp.err = Error::REQUEST_TIMEOUT;
      goto _response;
    }

    if(m_state != State::LOADED && m_state != State::UNLOADING) {
       req->rsp.err = m_state == State::DELETED
        ? Error::COLUMN_MARKED_REMOVED : Error::RGR_NOT_LOADED_RANGE;
      goto _response;
    }

    {
    const uint8_t* buf = req->ev->data_ext.base;
    size_t remain = req->ev->data_ext.size;
    bool aligned_chg = false;

    try { for(DB::Cells::Cell cell; remain; ) {

      cell.read(&buf, &remain);

      if(!m_interval.key_end.empty() &&
          DB::KeySeq::compare(cfg->key_seq, m_interval.key_end, cell.key)
           == Condition::GT) {
        if(req->rsp.range_end.empty()) {
          req->rsp.range_end.copy(m_interval.key_end);
          req->rsp.err = Error::RANGE_BAD_INTERVAL;
        }
        continue;
      }

      if(!prev_range_end.empty() &&
          DB::KeySeq::compare(cfg->key_seq, prev_range_end, cell.key)
           != Condition::GT) {
        if(req->rsp.range_prev_end.empty()) {
          req->rsp.range_prev_end.copy(prev_range_end);
          req->rsp.err = Error::RANGE_BAD_INTERVAL;
        }
        continue;
      }

      ++req->rsp.cells_added;
      if(cell.has_expired(ttl))
        continue;

      if(!(cell.control & DB::Cells::HAVE_TIMESTAMP)) {
        cell.set_timestamp(Time::now_ns());
        if(cell.control & DB::Cells::AUTO_TIMESTAMP)
          cell.control ^= DB::Cells::AUTO_TIMESTAMP;
        cell.control |= DB::Cells::REV_IS_TS;
      } else {
        cell.set_revision(Time::now_ns());
      }

      blocks.add_logged(cell);

      switch(cfg->range_type) {
        case DB::Types::Range::DATA: {
          if(DB::KeySeq::align(cfg->key_seq, cell.key, align_min, align_max))
            aligned_chg = true;
          break;
        }
        default: {
          /* MASTER/META need aligned interval
               over cells value +plus (cs+logs) at compact
          if(cell.flag == DB::Cells::INSERT) {
            size_t remain = cell.vlen;
            const uint8_t * ptr = cell.value;
            DB::Cell::Key key_end;
            key_end.decode(&ptr, &remain);
            Serialization::decode_vi64(&ptr, &remain);//rid
            DB::Cell::Key aligned_min;
            aligned_min.decode(&ptr, &remain);
            DB::Cell::Key aligned_max;
            aligned_max.decode(&ptr, &remain);
            aligned_chg = align(aligned_min);
            if(align(aligned_max))
              aligned_chg = true;
          }
          */
          break;
        }
      }
    } } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      req->rsp.err = Error::RANGE_BAD_CELLS_INPUT;
    }

    if(aligned_chg) {
      Core::MutexAtomic::scope lock(m_mutex_intval_alignment);
      aligned_chg = m_interval.align(align_min, align_max);
    }
    if(aligned_chg)
      return on_change(false, MetaRegOnAddReq::make(shared_from_this(), req));

    }

    _response:
      blocks.processing_decrement();
      req->response();
      bool added = req->rsp.cells_added;
      delete req;
      if(added)
        blocks.commitlog.commit();
  }

  if(m_adding.fetch_sub(1) == 1 && !m_q_add.empty())
    run_add_queue();
}


}}


#include "swcdb/ranger/Protocol/Rgr/req/RangeUnload.cc"
#include "swcdb/ranger/queries/select/CheckMeta.cc"
#include "swcdb/ranger/queries/update/BaseMeta.cc"
