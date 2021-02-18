/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CommitLog.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Semaphore.h"


namespace SWC { namespace Ranger { namespace CommitLog {

static const uint8_t MAX_FRAGMENTS_NARROW = 20;


Fragments::Fragments(const DB::Types::KeySeq key_seq)
                    : stopping(false), m_cells(key_seq), m_roll_chk(0),
                      m_compacting(false), m_deleting(false),
                      m_sem(5), m_last_id(0) {
}

void Fragments::init(const RangePtr& for_range) {

  range = for_range;

  m_cells.configure(
    range->cfg->block_cells()*2,
    range->cfg->cell_versions(),
    range->cfg->cell_ttl(),
    range->cfg->column_type()
  );
}

Fragments::~Fragments() {
  if(!m_cells.empty())
    Env::Rgr::res().less_mem_usage(m_cells.size_of_internal());
}

void Fragments::schema_update() {
  std::scoped_lock lock(m_mutex_cells);
  m_cells.configure(
    range->cfg->block_cells()*2,
    range->cfg->cell_versions(),
    range->cfg->cell_ttl(),
    range->cfg->column_type()
  );
}

void Fragments::add(const DB::Cells::Cell& cell) {
  {
    std::scoped_lock lock(m_mutex_cells);
    ssize_t sz = m_cells.size_of_internal();
    m_cells.add_raw(cell);
    Env::Rgr::res().adj_mem_usage(ssize_t(m_cells.size_of_internal()) - sz);
    if(++m_roll_chk ? !Env::Rgr::res().is_low_mem_state() : !_need_roll())
      return;
  }
  if(!m_commit.running())
    Env::Rgr::post([this](){ commit_new_fragment(); });
}

void Fragments::commit_new_fragment(bool finalize) {
  if(finalize) {
    std::unique_lock lock_wait(m_mutex);
    if(m_compacting || m_commit.running())
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commit.running(); });
  }

  Fragment::Ptr frag;
  for(int err; ;) {
    if(finalize) {
      std::shared_lock lock2(m_mutex_cells);
      if(m_cells.empty())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
      std::shared_lock lock2(m_mutex_cells);
      if(m_cells.empty() || (!finalize && !_need_roll()))
        break;
    }

    DynamicBuffer cells;
    uint32_t cells_count = 0;
    DB::Cells::Interval interval(range->cfg->key_seq);
    auto buff_write = std::make_shared<StaticBuffer>();
    ssize_t sz;
    {
      std::scoped_lock lock(m_mutex);
      size_t nxt_id = _next_id();
      {
        std::scoped_lock lock2(m_mutex_cells);
        sz = m_cells.size_of_internal();
        m_cells.write_and_free(
          cells, cells_count, interval,
          range->cfg->block_size(), range->cfg->block_cells());
        Env::Rgr::res().adj_mem_usage(ssize_t(m_cells.size_of_internal())-sz);
        if(m_deleting || !cells.fill())
          break;

        frag = Fragment::make_write(
          err = Error::OK,
          get_log_fragment(nxt_id),
          interval,
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
    }

    buff_write->own = false;
    m_sem.acquire();
    frag->write(
      Error::UNPOSSIBLE,
      range->cfg->file_replication(),
      frag->offset_data + frag->size_enc,
      buff_write,
      &m_sem
    );

    m_sem.wait_available();
  }

  if(finalize)
    m_sem.wait_all();

  m_commit.stop();
  {
    std::scoped_lock lock(m_mutex);
    m_cv.notify_all();
  }

  if(!finalize)
    try_compact();
}

void Fragments::add(Fragment::Ptr& frag) {
  std::scoped_lock lock(m_mutex);
  _add(frag);
}

void Fragments::_add(Fragment::Ptr& frag) {
  for(auto it = begin() + _narrow(frag->interval.key_begin); it<end(); ++it) {
    if(DB::KeySeq::compare(m_cells.key_seq,
        (*it)->interval.key_begin, frag->interval.key_begin)
         != Condition::GT) {
      insert(it, frag);
      return;
    }
  }
  push_back(frag);
}

SWC_SHOULD_INLINE
bool Fragments::is_compacting() const {
  return m_compacting;
}

size_t Fragments::need_compact(std::vector<Fragments::Vec>& groups,
                               const Fragments::Vec& without,
                               size_t vol) {
  std::shared_lock lock(m_mutex);
  return _need_compact(groups, without, vol);
}

bool Fragments::try_compact(int tnum) {
  if(stopping ||
     Env::Rgr::res().is_low_mem_state() ||
     !range->compact_possible(true))
    return false;

  std::vector<Fragments::Vec> groups;
  size_t need;
  uint8_t cointervaling = range->cfg->log_compact_cointervaling();
  {
    std::scoped_lock lock(m_mutex);
    if(_need_compact_major() && Env::Rgr::compaction_available()) {
      range->compacting(Range::COMPACT_NONE);
      return false;
    }
    need = _need_compact(groups, {}, cointervaling);
    m_compacting.store(true);
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
  return false;
}

void Fragments::finish_compact(const Compact* compact) {
  m_compacting.store(false);
  {
    std::scoped_lock lock(m_mutex);
    m_cv.notify_all();
  }
  range->compacting(Range::COMPACT_NONE);

  if(compact) {
    if(!stopping)
      try_compact(compact->repetition+1);
    delete compact;
  }
}

const std::string Fragments::get_log_fragment(const int64_t frag) const {
  std::string s(range->get_path(Range::LOG_DIR));
  s.append("/");
  s.append(std::to_string(frag));
  s.append(".frag");
  return s;
}

const std::string Fragments::get_log_fragment(const std::string& frag) const {
  std::string s(range->get_path(Range::LOG_DIR));
  s.append("/");
  s.append(frag);
  return s;
}

void Fragments::load(int &err) {
  //std::scoped_lock lock(m_mutex);
  // fragments header OR log.data >> file.frag(intervals)

  err = Error::OK;
  FS::DirentList fragments;
  Env::FsInterface::interface()->readdir(
    err, range->get_path(Range::LOG_DIR), fragments);
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
  std::shared_lock lock(m_mutex);
  for(auto& frag : *this)
    intval.expand(frag->interval);
}

void Fragments::expand_and_align(DB::Cells::Interval& intval) {
  std::shared_lock lock(m_mutex);
  for(auto& frag : *this) {
    intval.expand(frag->interval);
    intval.align(frag->interval);
  }
}

void Fragments::load_cells(BlockLoader* loader, bool& is_final,
                           Fragments::Vec& frags, uint8_t vol) {
  uint8_t base = vol;
  std::shared_lock lock(m_mutex);
  _load_cells(loader, frags, vol);
  if(is_final && (is_final = base == vol)) {
    std::shared_lock lock(m_mutex_cells);
    loader->block->load_final(m_cells);
  }
}

void Fragments::_load_cells(BlockLoader* loader, Fragments::Vec& frags,
                            uint8_t& vol) {
  for(auto& frag : *this) {
    if(std::find(frags.begin(), frags.end(), frag) == frags.end() &&
       loader->block->is_consist(frag->interval)) {
      frag->processing_increment();
      frags.push_back(frag);
      if(!--vol)
        return;
    }
  }
}

void Fragments::get(Fragments::Vec& fragments) {
  fragments.clear();

  std::shared_lock lock(m_mutex);
  fragments.assign(begin(), end());
}

size_t Fragments::release(size_t bytes) {
  size_t released = 0;
  std::shared_lock lock(m_mutex);

  for(auto& frag : *this) {
    released += frag->release();
    if(bytes && released >= bytes)
      break;
  }
  return released;
}

void Fragments::remove(int &err, Fragments::Vec& fragments_old, bool safe) {
  Core::Semaphore sem(10);
  if(!safe)
    m_mutex.lock();
  for(auto& frag : fragments_old) {
    sem.acquire();
    frag->remove(err, &sem);
    auto it = std::find(begin(), end(), frag);
    if(it != end())
      erase(it);
  }
  if(!safe)
    m_mutex.unlock();
  sem.wait_all();
}

void Fragments::remove(int &err, Fragment::Ptr& frag, bool remove_file) {
  std::scoped_lock lock(m_mutex);
  if(remove_file)
    frag->remove(err);
  auto it = std::find(begin(), end(), frag);
  if(it != end())
    erase(it);
}

void Fragments::remove() {
  stopping.store(true);
  {
    std::unique_lock lock_wait(m_mutex);
    m_deleting = true;
    if(m_compacting || m_commit.running())
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commit.running(); });
  }
  std::scoped_lock lock(m_mutex);
  for(auto& frag : *this)
    frag->mark_removed();
  clear();
  range = nullptr;
  m_commit.stop();
}

void Fragments::unload() {
  stopping.store(true);
  {
    std::unique_lock lock_wait(m_mutex);
    if(m_compacting || m_commit.running())
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commit.running(); });
  }
  std::scoped_lock lock(m_mutex);
  clear();
  range = nullptr;
  m_commit.stop();
}

Fragment::Ptr Fragments::take_ownership(int &err, Fragment::Ptr& take_frag) {
  const std::string filepath(get_log_fragment(next_id()));
  Env::FsInterface::interface()->rename(
    err, take_frag->get_filepath(), filepath);

  if(!err) {
    auto frag = Fragment::make_read(err, filepath, range->cfg->key_seq);
    if(frag) {
      add(frag);
      return frag;
    }
    Env::FsInterface::interface()->rename(
      err = Error::OK, filepath, take_frag->get_filepath());
  }
  return nullptr;
}

void Fragments::take_ownership(int& err, Fragments::Vec& frags,
                                         Fragments::Vec& removing) {
  const auto& fs_if = Env::FsInterface::interface();
  Fragments::Vec tmp_frags;
  for(auto it = frags.begin(); !stopping && it != frags.end(); ) {
    const std::string filepath(get_log_fragment(next_id()));
    fs_if->rename(err, (*it)->get_filepath(), filepath);
    if(err)
      break;
    frags.erase(it);

    auto frag = Fragment::make_read(err, filepath, range->cfg->key_seq);
    if(frag) {
      tmp_frags.push_back(frag);
    } else {
      fs_if->remove(err, filepath);
      break;
    }
  }

  if(stopping || !frags.empty()) {
    if(!tmp_frags.empty()) {
      Core::Semaphore sem(10);
      for(auto& frag : tmp_frags) {
        sem.acquire();
        frag->remove(err = Error::OK, &sem);
      }
      sem.wait_all();
    }
  } else {
    std::scoped_lock lock(m_mutex);
    for(auto& frag : tmp_frags)
      _add(frag);
    remove(err = Error::OK, removing, true);
  }
  removing.clear();
}

bool Fragments::deleting() {
  std::shared_lock lock(m_mutex);
  return m_deleting;
}

size_t Fragments::cells_count(bool only_current) {
  size_t count = 0;
  {
    std::shared_lock lock(m_mutex_cells);
    count += m_cells.size();
  }
  if(!only_current) {
    std::shared_lock lock(m_mutex);
    for(auto& frag : *this)
      count += frag->cells_count;
  }
  return count;
}

size_t Fragments::size() {
  std::shared_lock lock(m_mutex);
  return Vec::size();
}

size_t Fragments::size_bytes(bool only_loaded) {
  return _size_bytes(only_loaded);
}

size_t Fragments::size_bytes_encoded() {
  std::shared_lock lock(m_mutex);
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
  std::scoped_lock lock(m_mutex);
  return _next_id();
}

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
  std::shared_lock lock(m_mutex);

  out << "CommitLog(count=" << count
      << " cells=";
  {
    std::shared_lock lock(m_mutex_cells);
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


bool Fragments::_need_roll() const {
  auto ratio = range->cfg->log_rollout_ratio();
  auto bytes = range->cfg->block_size();
  auto cells = range->cfg->block_cells();
  return (m_cells.size() >= cells || m_cells.size_bytes() >= bytes) &&
         (m_cells.size_bytes() >= bytes * ratio ||
          m_cells.size() >= cells * ratio ||
          Env::Rgr::res().need_ram(bytes) );
}

size_t Fragments::_need_compact(std::vector<Fragments::Vec>& groups,
                                const Fragments::Vec& without,
                                size_t vol) {
  size_t need = 0;
  if(Vec::size() < vol)
    return need;

  Fragments::Vec fragments;
  for(auto& frag : *this) {
    if(without.empty() ||
       std::find(without.begin(), without.end(), frag) == without.end())
      fragments.push_back(frag);
  }
  if(fragments.size() < vol)
    return need;

  auto it = fragments.begin();
  groups.push_back({*it});
  ++need;
  Fragment::Ptr curt;
  Condition::Comp cond;
  for(++it; it<fragments.end(); ++it) {
    auto const& last = *groups.back().back();
    if((cond = DB::KeySeq::compare(m_cells.key_seq, last.interval.key_end,
                        (curt = *it)->interval.key_begin)) == Condition::LT ||
        (cond == Condition::EQ && ( range->cfg->cell_versions() == 1 || (
          curt->cells_count < range->cfg->block_cells() &&
          curt->size_bytes() < range->cfg->block_size() &&
          last.cells_count < range->cfg->block_cells() &&
          last.size_bytes() < range->cfg->block_size() ) )) ) {
      groups.back().push_back(curt);
      ++need;
    } else {
      if(groups.back().size() < vol) {
        need -= groups.back().size();
        groups.back() = {curt};
      } else {
        groups.push_back({curt});
      }
      ++need;
    }
  }
  for(auto it=groups.begin(); it != groups.end(); ) {
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
  if(need) {
    range->compact_require(true);
    Env::Rgr::compaction_schedule(1000);
  }
  return need;
}

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
    std::shared_lock lock(m_mutex_cells);
    size += m_cells.size_bytes();
  }
  for(auto& frag : *this)
    size += frag->size_bytes(only_loaded);
  return size;
}

size_t Fragments::_narrow(const DB::Cell::Key& key) const {
  size_t offset = 0;
  if(key.empty() || Vec::size() <= MAX_FRAGMENTS_NARROW)
    return offset;

  size_t step = offset = Vec::size() >> 1;
  try_narrow:
    if(DB::KeySeq::compare(m_cells.key_seq,
        (*(begin() + offset))->interval.key_begin, key) == Condition::GT) {
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
