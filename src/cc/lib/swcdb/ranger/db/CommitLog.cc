/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/CommitLog.h"
#include "swcdb/core/Encoder.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Semaphore.h"


namespace SWC { namespace Ranger { namespace CommitLog {



Fragments::Fragments(const Types::KeySeq key_seq)  
                    : m_cells(key_seq), stopping(false), 
                      m_commiting(false), m_deleting(false), 
                      m_compacting(false), m_sem(5) { 
}

void Fragments::init(RangePtr for_range) {
  SWC_ASSERT(for_range != nullptr);
  
  range = for_range;
  
  m_cells.configure(
    range->cfg->block_cells()*2,
    range->cfg->cell_versions(), 
    range->cfg->cell_ttl(), 
    range->cfg->column_type()
  );
}

Fragments::~Fragments() { }

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
  bool roll;
  {
    std::scoped_lock lock(m_mutex_cells);
    m_cells.add_raw(cell);
    roll = _need_roll();
  }

  if(roll && m_mutex.try_lock()) {
    if(!m_deleting && !m_commiting) {
      m_commiting = true;
      asio::post(*Env::IoCtx::io()->ptr(), [this](){ commit_new_fragment(); });
    }
    m_mutex.unlock();
  }
}

void Fragments::commit_new_fragment(bool finalize) {
  if(finalize) {
    std::unique_lock lock_wait(m_mutex);
    if(m_commiting || m_compacting)
      m_cv.wait(lock_wait, [this] {
        return !m_compacting && !m_commiting && (m_commiting = true); });
  }
  
  Fragment::Ptr frag;
  for(int err = Error::OK; ;err = Error::OK) {
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
    size_t nxt_id = next_id();
    {
      std::scoped_lock lock(m_mutex);
      {
        std::scoped_lock lock2(m_mutex_cells);
        m_cells.write_and_free(
          cells, cells_count, interval,
          range->cfg->block_size(), range->cfg->block_cells());
        if(m_deleting || !cells.fill())
          break;

        frag = Fragment::make_write(
          err, get_log_fragment(nxt_id), 
          interval, 
          range->cfg->block_enc(), range->cfg->cell_versions(),
          cells_count, cells, 
          buff_write
        );
        if(!frag) 
          // put cells back tp m_cells
          break;
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

    m_sem.wait_until_under(5);
  }

  if(finalize)
    m_sem.wait_all();
  {
    std::unique_lock lock_wait(m_mutex);
    m_commiting = false;
  }
  m_cv.notify_all();

  if(!finalize)
    try_compact();
}

void Fragments::add(Fragment::Ptr frag) {
  std::scoped_lock lock(m_mutex);
  _add(frag);
}

void Fragments::_add(Fragment::Ptr frag) {
  push_back(frag);
  
  std::sort(begin(), end(),
    [seq=m_cells.key_seq] (const Fragment::Ptr& f1, const Fragment::Ptr& f2) {
      return DB::KeySeq::compare(seq, 
        f1->interval.key_begin, f2->interval.key_begin) == Condition::GT; 
    }
  );
}

size_t Fragments::need_compact(std::vector<Fragments::Vec>& groups,
                               const Fragments::Vec& without,
                               size_t vol) {
  std::shared_lock lock(m_mutex);
  return _need_compact(groups, without, vol);
}

bool Fragments::try_compact(int tnum) {
  if(stopping || !range->compact_possible())
    return false;

  std::vector<Fragments::Vec> groups;
  size_t need;
  {
    std::scoped_lock lock(m_mutex);
    if(_need_compact_major() && RangerEnv::compaction_available()) {
      range->compacting(Range::COMPACT_NONE);
      return false;
    }
    need = _need_compact(groups, {}, MIN_COMPACT);
    m_compacting = true;
  }

  if(need) {
    range->compacting(need / groups.size() > range->cfg->log_rollout_ratio() 
      ? Range::COMPACT_PREPARING  // mitigate add
      : Range::COMPACT_COMPACTING// continue scan & add 
    ); 
    new Compact(this, tnum, groups);
    return true;
  }
  finish_compact(nullptr);
  return false;
}

void Fragments::finish_compact(const Compact* compact) {
  {
    std::scoped_lock lock(m_mutex);
    m_compacting = false;
  }
  range->compacting(Range::COMPACT_NONE); 
  m_cv.notify_all();

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
  for(auto entry : fragments) {
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
  for(auto frag : *this)
    intval.expand(frag->interval);
}

void Fragments::expand_and_align(DB::Cells::Interval& intval) {
  std::shared_lock lock(m_mutex);
  for(auto frag : *this) {
    intval.expand(frag->interval);
    intval.align(frag->interval);
  }
}

void Fragments::load_cells(BlockLoader* loader, bool& is_final,
                           Fragments::Vec& frags, uint8_t vol) {  
  if(is_final) {
    std::unique_lock lock_wait(m_mutex);
    if(m_commiting)
      m_cv.wait(lock_wait, [this]{ return !m_commiting; });

    uint8_t base = vol;
    _load_cells(loader, frags, vol);
    if(is_final = base == vol) {
      std::shared_lock lock(m_mutex_cells);
      loader->block->load_cells(m_cells);
    }

  } else {
    std::shared_lock lock(m_mutex);
    _load_cells(loader, frags, vol);
  }
}

void Fragments::_load_cells(BlockLoader* loader, Fragments::Vec& frags, 
                            uint8_t& vol) { 
  for(auto frag : *this) {
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

  for(auto frag : *this) {
    released += frag->release();
    if(bytes && released >= bytes)
      break;
  }
  return released;
}

void Fragments::remove(int &err, Fragments::Vec& fragments_old) {
  std::scoped_lock lock(m_mutex);
  for(auto frag : fragments_old) {
    auto it = std::find(begin(), end(), frag);
    if(it != end()) {
      erase(it);
      frag->remove(err);
      delete frag;
    } else {
      SWC_ASSERT(it != end());
    }
  }
}

void Fragments::remove(int &err, Fragment::Ptr frag, bool remove_file) {
  std::scoped_lock lock(m_mutex);
  auto it = std::find(begin(), end(), frag);
  if(it != end()) {
    erase(it);
    if(remove_file)
      frag->remove(err);
    delete frag;
  } else {
    SWC_ASSERT(it != end());
  }
}

void Fragments::remove(int &err) {
  stopping = true;
  {
    std::unique_lock lock_wait(m_mutex);
    m_deleting = true;
    if(m_commiting || m_compacting) {
      m_cv.wait(lock_wait, [this]{ return !m_commiting && !m_compacting; });
    }
  }
  std::scoped_lock lock(m_mutex);
  for(auto frag : *this) {
    frag->remove(err);
    delete frag;
  }
  clear();
  range = nullptr;
}

void Fragments::unload() {
  {
    std::unique_lock lock_wait(m_mutex);
    if(m_commiting || m_compacting)
      m_cv.wait(lock_wait, [this]{ return !m_commiting && !m_compacting; });
  }
  stopping = true;
  std::scoped_lock lock(m_mutex);
  for(auto frag : *this)
    delete frag;
  clear();
  range = nullptr;
}

Fragment::Ptr Fragments::take_ownership(int &err, Fragment::Ptr take_frag) {
  const std::string filepath(get_log_fragment(next_id()));
  Env::FsInterface::interface()->rename(
    err, take_frag->get_filepath(), filepath);

  if(!err) {
    auto frag = Fragment::make_read(err, filepath, range->cfg->key_seq);
    if(frag) {
      add(frag);
      return frag;
    }
    err = Error::OK;
    // ? log compact
    // Env::FsInterface::interface()->remove(tmperr, filepath); 
    Env::FsInterface::interface()->rename(
      err, filepath, take_frag->get_filepath());
  }
  return nullptr;
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
    for(auto frag : *this)
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
  for(auto frag : *this)
    size += frag->size_bytes_encoded();
  return size;
}

bool Fragments::processing() {
  bool busy;
  if(!(busy = !m_mutex.try_lock())) {
    busy = _processing();
    m_mutex.unlock();
  }
  return busy;
}

uint64_t Fragments::next_id() {
  std::scoped_lock lock(m_mutex);
  return Time::now_ns();
}

std::string Fragments::to_string() {
  size_t count = cells_count();
  std::shared_lock lock(m_mutex);

  std::string s("CommitLog(count=");
  s.append(std::to_string(count));
  
  s.append(" cells=");
  {
    std::shared_lock lock(m_mutex_cells);
  s.append(std::to_string(m_cells.size()));
  }

  s.append(" fragments=");
  s.append(std::to_string(Vec::size()));

  s.append(" [");
  for(auto frag : *this){
    s.append(frag->to_string());
    s.append(", ");
  }
  s.append("]");

  s.append(" processing=");
  s.append(std::to_string(_processing()));

  s.append(" used/actual=");
  s.append(std::to_string(_size_bytes(true)));
  s.append("/");
  s.append(std::to_string(_size_bytes()));

  s.append(")");
  return s;
}


bool Fragments::_need_roll() const {
  auto ratio = range->cfg->log_rollout_ratio();
  auto bytes = range->cfg->block_size();
  auto cells = range->cfg->block_cells();
  return (m_cells.size() >= cells || m_cells.size_bytes() >= bytes) && 
         (m_cells.size_bytes() >= bytes * ratio ||
          m_cells.size() >= cells * ratio ||
          Env::Resources.need_ram(bytes) );
}

size_t Fragments::_need_compact(std::vector<Fragments::Vec>& groups,
                                const Fragments::Vec& without,
                                size_t vol) {
  size_t need = 0;
  if(Vec::size() < vol)
    return need;
  
  Fragments::Vec fragments;
  for(auto frag : *this) {
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
  for(auto it=groups.begin(); it < groups.end(); ) {
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
    for(auto frag : *this) {
      if(need = (sz_bytes += frag->size_bytes_encoded()) > ok)
        break;
    }
  }
  if(need) {
    range->compact_require(true);
    RangerEnv::compaction_schedule(1000);
  }
  return need;
}

bool Fragments::_processing() const {
  if(m_commiting)
    return true;
  for(auto frag : *this)
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
  for(auto frag : *this)
    size += frag->size_bytes(only_loaded);
  return size;
}



}}} // namespace SWC::Ranger::CommitLog