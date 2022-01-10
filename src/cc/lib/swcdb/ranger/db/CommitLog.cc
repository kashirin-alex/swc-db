/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CommitLog.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Semaphore.h"


namespace SWC { namespace Ranger { namespace CommitLog {

static const uint8_t MAX_FRAGMENTS_NARROW = 20;


SWC_CAN_INLINE
Fragments::Fragments(const DB::Types::KeySeq key_seq)
                    : stopping(false), m_cells(key_seq),
                      m_compacting(false), m_deleting(false),
                      m_sem(5), m_last_id(0),
                      m_releasable_bytes(0), m_modification_ts(0) {
}

Fragments::~Fragments() noexcept {
  Env::Rgr::res().less_mem_releasable(m_releasable_bytes);
}

SWC_CAN_INLINE
void Fragments::init(const RangePtr& for_range) {
  range = for_range;
  schema_update();
}

SWC_CAN_INLINE
void Fragments::schema_update() {
  Core::ScopedLock lock(m_mutex_cells);
  m_cells.configure(
    range->cfg->block_cells() * 2,
    range->cfg->cell_versions(),
    range->cfg->cell_ttl(),
    range->cfg->column_type(),
    false
  );
}


SWC_CAN_INLINE
void Fragments::add(const DB::Cells::Cell& cell) {
  Core::ScopedLock lock(m_mutex_cells);
  m_cells.add_raw(cell, false);
}

void Fragments::commit() noexcept {
  m_modification_ts.store(Time::now_ns());
  if(m_commit.running())
    return;
  bool run = m_mutex_cells.try_lock_shared();
  if(run) {
    ssize_t sz = m_cells.size_of_internal();
    Env::Rgr::res().adj_mem_releasable(sz - m_releasable_bytes.exchange(sz));
    run = _need_roll();
    m_mutex_cells.unlock_shared();
  }
  if(!run)
    return m_commit.stop();

  struct Task {
    Fragments* ptr;
    SWC_CAN_INLINE
    Task(Fragments* a_ptr) noexcept : ptr(a_ptr) { }
    void operator()() { ptr->_commit(false); }
  };
  Env::Rgr::post(Task(this));
}

size_t Fragments::commit_release() {
  if(!m_releasable_bytes || !m_mutex.try_lock_shared())
    return 0;
  bool ok = !m_compacting && !m_commit.running();
  m_mutex.unlock_shared();
  return ok ? _commit(true) : 0;
}

void Fragments::commit_finalize() {
  {
    Core::UniqueLock lock_wait(m_mutex);
    if(m_compacting || m_commit.running()) {
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commit.running(); });
    }
  }
  _commit(true);
}

size_t Fragments::_commit(bool finalize) {
  Fragment::Ptr frag;
  size_t released_bytes = 0;
  ssize_t releasable;
  uint32_t block_size;
  for(int err; ;) {
    {
      Core::SharedLock lock2(m_mutex_cells);
      if(m_cells.empty() || (!finalize && !_need_roll()))
        break;
    }

    block_size = range->cfg->block_size();
    Env::Rgr::res().more_mem_future(block_size);

    StaticBuffer::Ptr buff_write(new StaticBuffer());
    {
      DynamicBuffer cells;
      uint32_t cells_count = 0;
      DB::Cells::Interval interval(range->cfg->key_seq);
      Core::ScopedLock lock(m_mutex);
      size_t nxt_id = _next_id();
      {
        Core::ScopedLock lock2(m_mutex_cells);
        m_cells.write_and_free(
          cells, cells_count, interval,
          block_size, range->cfg->block_cells());
        if(m_deleting || !cells.fill())
          break;
        releasable = m_cells.size_of_internal();
      }

      frag = Fragment::make_write(
        err = Error::OK,
        get_log_fragment(nxt_id),
        std::move(interval),
        range->cfg->block_enc(), range->cfg->cell_versions(),
        cells_count, cells,
        buff_write
      );
      if(!frag) {
        // if can happen checkup (fallbacks to Plain at Encoder err)
        // put cells back tp m_cells
        SWC_LOG_OUT(LOG_WARN,
          Error::print(SWC_LOG_OSTREAM << "Bad Fragment Write "
            << range->cfg->cid << '/' << range->rid << ' ', err);
        );
        break;
      }
      _add(frag);
    }

    buff_write->own = false;
    m_sem.acquire();
    frag->write(
      Error::UNPOSSIBLE,
      range->cfg->file_replication(),
      buff_write,
      &m_sem
    );

    ssize_t tmp = m_releasable_bytes.exchange(releasable);
    Env::Rgr::res().adj_mem_releasable(releasable - tmp);
    if(tmp > releasable)
      released_bytes += tmp - releasable;

    m_sem.wait_available();
    Env::Rgr::res().less_mem_future(block_size);
  }

  if(finalize)
    m_sem.wait_all();

  m_commit.stop();
  {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  }

  if(!finalize)
    try_compact();

  return released_bytes;
}

SWC_CAN_INLINE
void Fragments::add(Fragment::Ptr& frag) {
  Core::ScopedLock lock(m_mutex);
  _add(frag);
}

void Fragments::_add(Fragment::Ptr& frag) {
  for(auto it = cbegin() + _narrow(frag->interval.key_begin);
      it != cend(); ++it) {
    if(DB::KeySeq::compare(m_cells.key_seq,
        (*it)->interval.key_begin, frag->interval.key_begin)
         != Condition::GT) {
      insert(it, frag);
      return;
    }
  }
  push_back(frag);
}

SWC_CAN_INLINE
bool Fragments::is_compacting() const {
  return m_compacting;
}

size_t Fragments::need_compact(CompactGroups& groups,
                               const Fragments::Vec& without,
                               size_t vol) {
  Core::SharedLock lock(m_mutex);
  return _need_compact(groups, without, vol);
}

bool Fragments::try_compact(uint32_t tnum) {
  if(stopping || Env::Rgr::res().is_low_mem_state())
    return false;

  bool at = false;
  if(!m_compacting.compare_exchange_weak(at, true))
    return false;

  at = Env::Rgr::log_compact_possible();
  if(at && !(at=range->compact_possible(true)))
    Env::Rgr::log_compact_finished();
  if(!at) {
    m_compacting.store(false);
    {
      Core::ScopedLock lock(m_mutex);
      m_cv.notify_all();
    }
    return false;
  }

  CompactGroups groups;
  uint8_t cointervaling = range->cfg->log_compact_cointervaling();
  size_t need = 0;
  bool need_major;
  {
    Core::ScopedLock lock(m_mutex);
    if(!(need_major = _need_compact_major())) {
      need = _need_compact(groups, {}, cointervaling);
    }
  }
  if(need) {
    range->compacting(need / groups.size() > range->cfg->log_rollout_ratio()
      ? Range::COMPACT_PREPARING  // mitigate add
      : Range::COMPACT_COMPACTING // continue scan & add
    );
    new Compact(this, tnum, groups, cointervaling);
    return true;
  }

  finish_compact(nullptr);
  if(need_major) {
    range->compact_require(true);
    Env::Rgr::compaction_schedule(1000);
  }
  return false;
}

void Fragments::finish_compact(const Compact* compact) {
  m_compacting.store(false);
  {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  }
  range->compacting(Range::COMPACT_NONE);
  Env::Rgr::log_compact_finished();

  if(compact) {
    if(!stopping)
      try_compact(compact->repetition+1);
    delete compact;
  }
}

std::string Fragments::get_log_fragment(const int64_t frag) const {
  std::string s(range->get_path(DB::RangeBase::LOG_DIR));
  std::string tmp(std::to_string(frag));
  s.reserve(s.length() + 6 + tmp.length());
  s.append("/");
  s.append(tmp);
  s.append(".frag");
  return s;
}

std::string Fragments::get_log_fragment(const std::string& frag) const {
  std::string s(range->get_path(DB::RangeBase::LOG_DIR));
  s.reserve(s.length() + 1 + frag.length());
  s.append("/");
  s.append(frag);
  return s;
}

void Fragments::load(int &err) {
  //Core::ScopedLock lock(m_mutex);
  // fragments header OR log.data >> file.frag(intervals)

  err = Error::OK;
  FS::DirentList fragments;
  Env::FsInterface::interface()->readdir(
    err, range->get_path(DB::RangeBase::LOG_DIR), fragments);
  if(err)
    return;

  Fragment::Ptr frag;
  for(auto& entry : fragments) {
    frag = Fragment::make_read(
      err, get_log_fragment(entry.name), range->cfg->key_seq);
    if(err == Error::FS_PATH_NOT_FOUND) {
      err = Error::OK;
      continue;
    }
    if(!frag)
      return;
    add(frag);
  }
}

void Fragments::expand(DB::Cells::Interval& intval) {
  Core::SharedLock lock(m_mutex);
  for(auto& frag : *this)
    intval.expand(frag->interval);
}

void Fragments::expand_and_align(DB::Cells::Interval& intval) {
  Core::SharedLock lock(m_mutex);
  for(auto& frag : *this) {
    intval.expand(frag->interval);
    intval.align(frag->interval);
  }
}

SWC_CAN_INLINE
void Fragments::load_cells(BlockLoader* loader, bool& is_final,
                           Fragments::Vec& frags, uint8_t vol) {
  uint8_t base = vol;
  Core::SharedLock lock(m_mutex);
  _load_cells(loader, frags, vol);
  if(is_final && (is_final = base == vol)) {
    Core::SharedLock lock_cells(m_mutex_cells);
    loader->block->load_final(m_cells);
  }
}

SWC_CAN_INLINE
void Fragments::_load_cells(BlockLoader* loader, Fragments::Vec& frags,
                            uint8_t& vol) {
  for(auto& frag : *this) {
    if(std::find(frags.cbegin(), frags.cend(), frag) == frags.cend() &&
       loader->block->is_consist(frag->interval)) {
      frag->processing_increment();
      frags.push_back(frag);
      if(!--vol)
        return;
    }
  }
}

void Fragments::get(Fragments::Vec& fragments) {
  Core::SharedLock lock(m_mutex);
  fragments.assign(cbegin(), cend());
}

size_t Fragments::release(size_t bytes) {
  size_t released = 0;
  if(m_mutex.try_lock_shared()) {
    for(auto& frag : *this) {
      released += frag->release();
      if(released >= bytes)
        break;
    }
    m_mutex.unlock_shared();
  }
  return released;
}

void Fragments::remove(int &err, Fragments::Vec& fragments_old) {
  Core::Semaphore sem(10);
  {
    Core::ScopedLock lock(m_mutex);
    _remove(err, fragments_old, &sem);
  }
  sem.wait_all();
}

void Fragments::_remove(int &err, Fragments::Vec& fragments_old,
                        Core::Semaphore* semp) {
  for(auto& frag : fragments_old) {
    semp->acquire();
    frag->remove(err, semp);
    auto it = std::find(cbegin(), cend(), frag);
    if(it != cend())
      erase(it);
  }
}

void Fragments::remove(int &err, Fragment::Ptr& frag, bool remove_file) {
  Core::ScopedLock lock(m_mutex);
  if(remove_file)
    frag->remove(err);
  auto it = std::find(cbegin(), cend(), frag);
  if(it != cend())
    erase(it);
}

void Fragments::remove() {
  stopping.store(true);
  {
    Core::UniqueLock lock_wait(m_mutex);
    m_deleting = true;
    if(m_compacting || m_commit.running())
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commit.running(); });
  }
  m_sem.wait_all();
  Core::ScopedLock lock(m_mutex);
  for(auto& frag : *this)
    frag->mark_removed();
  clear();
  range = nullptr;
  m_commit.stop();
}

void Fragments::unload() {
  stopping.store(true);
  {
    Core::UniqueLock lock_wait(m_mutex);
    if(m_compacting || m_commit.running())
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commit.running(); });
  }
  Core::ScopedLock lock(m_mutex);
  clear();
  range = nullptr;
  m_commit.stop();
}

Fragment::Ptr Fragments::take_ownership(int &err, Fragment::Ptr& take_frag) {
  auto smartfd = FS::SmartFd::make_ptr(get_log_fragment(next_id()), 0);
  Env::FsInterface::interface()->rename(
    err, take_frag->get_filepath(), smartfd->filepath());

  if(!err) {
    auto frag = Fragment::make_read(err, smartfd, range->cfg->key_seq);
    if(frag) {
      add(frag);
      return frag;
    }
    Env::FsInterface::interface()->rename(
      err = Error::OK, smartfd->filepath(), take_frag->get_filepath());
  }
  return nullptr;
}

void Fragments::take_ownership(int& err, Fragments::Vec& frags,
                                         Fragments::Vec& removing) {
  const auto& fs_if = Env::FsInterface::interface();
  Fragments::Vec tmp_frags;
  for(auto it = frags.cbegin(); !stopping && it != frags.cend(); ) {
    auto smartfd = FS::SmartFd::make_ptr(get_log_fragment(next_id()), 0);
    fs_if->rename(err, (*it)->get_filepath(), smartfd->filepath());
    if(err)
      break;
    frags.erase(it);

    auto frag = Fragment::make_read(err, smartfd, range->cfg->key_seq);
    if(frag) {
      tmp_frags.push_back(frag);
    } else {
      fs_if->remove(err, smartfd->filepath());
      break;
    }
  }

  Core::Semaphore sem(10);
  if(stopping || !frags.empty()) {
    for(auto& frag : tmp_frags) {
      sem.acquire();
      frag->remove(err = Error::OK, &sem);
    }
  } else {
    Core::ScopedLock lock(m_mutex);
    for(auto& frag : tmp_frags)
      _add(frag);
    _remove(err = Error::OK, removing, &sem);
  }
  removing.clear();
  sem.wait_all();
}

bool Fragments::deleting() {
  Core::SharedLock lock(m_mutex);
  return m_deleting;
}

size_t Fragments::cells_count(bool only_current) {
  size_t count = 0;
  {
    Core::SharedLock lock(m_mutex_cells);
    count += m_cells.size();
  }
  if(!only_current) {
    Core::SharedLock lock(m_mutex);
    for(auto& frag : *this)
      count += frag->cells_count;
  }
  return count;
}

SWC_CAN_INLINE
bool Fragments::empty() {
  Core::SharedLock lock1(m_mutex);
  if(!Vec::empty())
    return false;
  Core::SharedLock lock2(m_mutex_cells);
  return m_cells.empty();
}

size_t Fragments::size() noexcept {
  Core::SharedLock lock(m_mutex);
  return Vec::size();
}

SWC_CAN_INLINE
size_t Fragments::size_bytes(bool only_loaded) {
  return _size_bytes(only_loaded);
}

size_t Fragments::size_bytes_encoded() {
  Core::SharedLock lock(m_mutex);
  size_t size = 0;
  for(auto& frag : *this)
    size += frag->size_bytes_encoded();
  return size;
}

bool Fragments::processing() noexcept {
  bool busy;
  if(!(busy = !m_mutex.try_lock())) {
    busy = _processing();
    m_mutex.unlock();
  }
  return busy;
}

uint64_t Fragments::next_id() {
  Core::ScopedLock lock(m_mutex);
  return _next_id();
}

SWC_CAN_INLINE
uint64_t Fragments::_next_id() {
  uint64_t new_id = Time::now_ns();
  if(m_last_id == new_id) {
    ++new_id;
    // debug
    SWC_LOG_OUT(LOG_WARN, SWC_LOG_OSTREAM
      << " Fragments::next_id was the same id=" << new_id; );
  }
  return m_last_id = new_id;
}

void Fragments::print(std::ostream& out, bool minimal) {
  size_t count = cells_count();
  Core::SharedLock lock(m_mutex);

  out << "CommitLog(count=" << count
      << " cells=";
  {
    Core::SharedLock lock_cells(m_mutex_cells);
    out << m_cells.size();
  }

  out << " fragments=" << Vec::size();
  if(!minimal) {
    out << " [";
    for(auto& frag : *this){
      frag->print(out);
      out << ", ";
    }
    out << ']';
  }

  out << " processing=" << _processing()
      << " used/actual=" << _size_bytes(true) << '/' << _size_bytes()
      << ')';
}


SWC_CAN_INLINE
bool Fragments::_need_roll() const noexcept {
  if(m_cells.empty())
    return false;
  auto cells = m_cells.size();
  if(!cells)
    return false;
  auto bytes = m_cells.size_bytes();
  auto cfg_cells = range->cfg->block_cells();
  auto cfg_bytes = range->cfg->block_size();
  if(cells < cfg_cells && bytes < cfg_bytes)
    return false;
  auto cfg_ratio = range->cfg->log_rollout_ratio();
  return bytes >= cfg_bytes * cfg_ratio ||
         cells >= cfg_cells * cfg_ratio ||
         Env::Rgr::res().need_ram(bytes);
}

size_t Fragments::_need_compact(CompactGroups& groups,
                                const Fragments::Vec& without,
                                size_t vol) {
  size_t need = 0;
  if(stopping || Vec::size() < vol)
    return need;

  Fragments::Vec fragments;
  if(without.empty()) {
    fragments.assign(Vec::cbegin(), Vec::cend());
  } else {
    if(Vec::size() < without.size() + vol)
      return need;
    fragments.reserve(Vec::size() - without.size());
    for(auto& frag : *this) {
      if(std::find(without.cbegin(), without.cend(), frag) == without.cend())
        fragments.push_back(frag);
    }
    if(fragments.size() < vol)
      return need;
  }

  groups.emplace_back().push_back(fragments.front());
  ++need;
  Fragment::Ptr curt;
  Condition::Comp cond;
  for(auto it = fragments.cbegin() + 1; it != fragments.cend(); ++it) {
    auto const& last = *groups.back().back();
    if(!( (cond = DB::KeySeq::compare(m_cells.key_seq, last.interval.key_end,
                        (curt = *it)->interval.key_begin)) == Condition::LT ||
        (cond == Condition::EQ &&
         curt->cells_count < range->cfg->block_cells() &&
         curt->size_bytes() < range->cfg->block_size() &&
         last.cells_count < range->cfg->block_cells() &&
         last.size_bytes() < range->cfg->block_size() ) ) ) {
      if(groups.back().size() < vol) {
        need -= groups.back().size();
        groups.back().clear();
      } else {
        groups.emplace_back();
      }
    }
    groups.back().push_back(curt);
    ++need;
  }
  for(auto it=groups.cbegin(); it != groups.cend(); ) {
    if(it->size() < vol) {
      need -= it->size();
      groups.erase(it);
    } else {
      ++it;
    }
  }
  return need;
}

bool Fragments::_need_compact_major() {
  size_t ok = range->cfg->cellstore_size();
  ok /= 100;
  ok *= range->cfg->compact_percent();

  bool need = Vec::size() > ok/range->cfg->block_size() &&
    range->blocks.cellstores.blocks_count() < ok/range->cfg->block_size();
  if(!need) {
    size_t sz_bytes = 0;
    for(auto& frag : *this) {
      if((need = (sz_bytes += frag->size_bytes_encoded()) > ok))
        break;
    }
  }
  return need;
}

SWC_CAN_INLINE
bool Fragments::_processing() const noexcept {
  if(m_commit)
    return true;
  for(auto& frag : *this)
    if(frag->processing())
      return true;
  return false;
}

size_t Fragments::_size_bytes(bool only_loaded) {
  size_t size = 0;
  {
    Core::SharedLock lock(m_mutex_cells);
    size += m_cells.size_bytes();
  }
  for(auto& frag : *this)
    size += frag->size_bytes(only_loaded);
  return size;
}

SWC_CAN_INLINE
size_t Fragments::_narrow(const DB::Cell::Key& key) const {
  size_t offset = 0;
  if(key.empty() || Vec::size() <= MAX_FRAGMENTS_NARROW)
    return offset;

  size_t step = offset = Vec::size() >> 1;
  try_narrow:
    if(DB::KeySeq::compare(m_cells.key_seq,
        (*(cbegin() + offset))->interval.key_begin, key) == Condition::GT) {
      if(step < MAX_FRAGMENTS_NARROW)
        return offset;
      offset += step >>= 1;
      goto try_narrow;
    }
    if((step >>= 1) <= MAX_FRAGMENTS_NARROW)
      ++step;
    if(offset < step)
      return 0;
    offset -= step;
  goto try_narrow;
}


}}} // namespace SWC::Ranger::CommitLog
