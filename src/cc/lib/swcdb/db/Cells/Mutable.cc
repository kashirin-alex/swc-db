/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/Mutable.h"


namespace SWC { namespace DB { namespace Cells {



void Mutable::take_sorted(Mutable& other) {
  if(!other.empty()) {
    if(empty()) {
      delete *buckets.begin();
      buckets.clear();
    }
    _bytes += other.size_bytes();
    _size += other.size();
    buckets.insert(buckets.end(), other.buckets.begin(), other.buckets.end());
    other.buckets.clear();
    other.free();
  }
}

void Mutable::free() {
  for(auto bucket : buckets) {
    for(auto cell : *bucket)
      delete cell;
    delete bucket;
  }
  _bytes = 0;
  _size = 0;
  buckets.clear();
  buckets.shrink_to_fit();
  buckets.emplace_back(make_bucket(0));
}

size_t Mutable::add_sorted(const uint8_t* ptr, size_t remain) {
  size_t count = 0;
  _bytes += remain;
  while(remain) {
    _push_back(new Cell(&ptr, &remain, true));
    ++count;
  }
  _size += count;
  return count;
}


void Mutable::add_raw(const DynamicBuffer& cells) {
  const uint8_t* ptr = cells.base;
  size_t remain = cells.fill();
  while(remain) {
    add_raw(Cell(&ptr, &remain));
  }
}

void Mutable::add_raw(const DynamicBuffer& cells,
                      const DB::Cell::Key& upto_key,
                      const DB::Cell::Key& from_key,
                      uint32_t skip, bool malformed) {
  const uint8_t* ptr = cells.base;
  size_t remain = cells.fill();
  while(remain) {
    Cell cell(&ptr, &remain);
    if(malformed && !skip) {
      add_raw(cell);
    } else if(
        (!upto_key.empty() &&
         DB::KeySeq::compare(key_seq, upto_key, cell.key) != Condition::GT) ||
        DB::KeySeq::compare(key_seq, from_key, cell.key) == Condition::GT) {
      add_raw(cell);
    } else {
      --skip;
    }
  }
}

Cell* Mutable::takeout(size_t idx) {
  auto it = Iterator(&buckets, idx);
  if(!it)
    return nullptr;
  Cell* cell = *it.item;
  _remove(cell);
  it.remove();
  return cell;
}


void Mutable::write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                             Interval& intval, uint32_t threshold,
                             uint32_t max_cells) {
  if(!_size)
    return;

  cells.ensure(_bytes < threshold? _bytes: threshold);
  Cell* first = nullptr;
  Cell* last = nullptr;
  Cell* cell;
  size_t count = 0;
  auto it_start = Iterator(&buckets);
  for(auto it = it_start; it && (!threshold || threshold > cells.fill()) &&
                                (!max_cells || max_cells > cell_count); ++it) {
    ++count;
    if((cell=*it.item)->has_expired(ttl))
      continue;

    cell->write(cells);
    intval.expand(cell->timestamp);
    intval.align(cell->key);
    (first ? last : first) = cell;
    ++cell_count;
  }

  if(first) {
    intval.expand_begin(*first);
    intval.expand_end(*(last ? last : first));
  }

  if(_size == count)
    free();
  else if(count)
    _remove(it_start, count);
}

bool Mutable::write_and_free(const DB::Cell::Key& key_start,
                             const DB::Cell::Key& key_finish,
                             DynamicBuffer& cells, uint32_t threshold) {
  bool more = _size;
  if(!more)
    return more;

  cells.ensure(_bytes < threshold? _bytes: threshold);

  size_t count = 0;
  Iterator it_start;
  {
  auto it = Iterator(&buckets, _narrow(key_start));
  for(Cell* cell; it && (!threshold || threshold > cells.fill()); ++it) {
    cell = *it.item;

    if(!key_start.empty() &&
        DB::KeySeq::compare(key_seq, key_start, cell->key) == Condition::LT)
      continue;
    if(!key_finish.empty() &&
        DB::KeySeq::compare(key_seq, key_finish, cell->key) == Condition::GT) {
      more = false;
      break;
    }

    if(!count)
      it_start = it;
    ++count;

    if(!cell->has_expired(ttl))
      cell->write(cells);
  }
  }

  if(count) {
    if(count == _size) {
      free();
    } else {
      _remove(it_start, count);
      return more && it_start;
    }
  }
  return false;
}

bool Mutable::write_and_free(DynamicBuffer& cells, uint32_t threshold) {
  bool more = _size;
  if(!more)
    return more;

  cells.ensure(_bytes < threshold? _bytes: threshold);

  size_t count = 0;
  Iterator it_start = Iterator(&buckets);
  for(auto it=it_start; it && (!threshold || threshold>cells.fill()); ++it) {
    ++count;
    if(!(*it.item)->has_expired(ttl))
      (*it.item)->write(cells);
  }
  if(count) {
    if(count == _size) {
      free();
    } else {
      _remove(it_start, count);
      return it_start;
    }
  }
  return false;
}

void Mutable::print(std::ostream& out, bool with_cells) const {
  out << "Cells(size=" << size() << '/' << _size
      << " bytes=" << _bytes
      << " type=" << Types::to_string(type)
      << " max_revs=" << max_revs
      << " ttl=" << ttl
      << " buckets=" << buckets.size();
  if(with_cells) {
    size_t count = 0;
    out << " [\n";
    for(auto bucket : buckets) {
      out << " sz=" << bucket->size() << " (";
      for(auto cell : *bucket) {
         cell->print(out << '\n', type);
        ++count;
      }
     out << "\n)\n";
    }
    out << "] counted=" << count;
  }

  out << ')';
}


void Mutable::write(DynamicBuffer& cells) const {
  cells.ensure(_bytes);
  for(auto it = ConstIterator(&buckets); it; ++it) {
    if(!(*it.item)->has_expired(ttl))
      (*it.item)->write(cells);
  }
}


void Mutable::scan_version_single(ReqScan* req) const {
  bool stop = false;
  const bool only_deletes = req->spec.flags.is_only_deletes();
  const Cell* cell;
  for(auto it = ConstIterator(&buckets, _narrow(req->spec));
      !stop && it; ++it) {
    cell = *it.item;

    if((only_deletes ? cell->flag == INSERT : cell->flag != INSERT) ||
       cell->has_expired(ttl) ||
       !req->selector(key_seq, *cell, stop) ||
       req->offset_adjusted()) {
      req->profile.skip_cell();

    } else if(!req->add_cell_and_more(*cell)) {
      break;
    }
  }
}

void Mutable::scan_version_multi(ReqScan* req) const {
  bool stop = false;
  const bool only_deletes = req->spec.flags.is_only_deletes();

  bool chk_align = !req->spec.offset_key.empty();
  uint32_t rev = chk_align ? req->spec.flags.max_versions : 0;
  const DB::Cell::Key* last_key = nullptr;
  const Cell* cell;
  for(auto it = ConstIterator(&buckets, _narrow(req->spec));
      !stop && it; ++it) {
    cell = *it.item;

    if((only_deletes ? cell->flag == INSERT : cell->flag != INSERT) ||
       cell->has_expired(ttl)) {
      req->profile.skip_cell();
      continue;
    }

    if(chk_align) {
      Condition::Comp comp = DB::KeySeq::compare(
        key_seq, req->spec.offset_key, cell->key);
      if(comp == Condition::LT) {
        req->profile.skip_cell();
        continue;
      }
      if(comp == Condition::EQ &&
         !req->spec.is_matching(cell->timestamp, cell->control & TS_DESC)) {
        if(!--rev)
          chk_align = false;
        last_key = &cell->key;
        req->profile.skip_cell();
        continue;
      }
      chk_align = false;
    }

    if(!req->selector(key_seq, *cell, stop)) {
      req->profile.skip_cell();
      continue;
    }
    if(last_key && last_key->equal(cell->key)) {
      if(!rev) {
        req->profile.skip_cell();
        continue;
      }
    } else {
      rev = req->spec.flags.max_versions;
      last_key = &cell->key;
    }

    if(req->offset_adjusted()) {
      --rev;
      req->profile.skip_cell();
      continue;
    }

    if(!req->add_cell_and_more(*cell))
      break;
    --rev;
  }
}

void Mutable::scan_test_use(const Specs::Interval& specs,
                            DynamicBuffer& result,
                            size_t& count, size_t& skips) const {
  bool stop = false;
  uint32_t cell_offset = specs.flags.offset;
  const bool only_deletes = specs.flags.is_only_deletes();
  const Cell* cell;
  for(auto it = ConstIterator(&buckets); !stop && it; ++it) {
    cell = *it.item;

    if((only_deletes ? cell->flag != INSERT : cell->flag == INSERT) &&
       !cell->has_expired(ttl) &&
       specs.is_matching(key_seq, *cell, stop)) {

      if(cell_offset) {
        --cell_offset;
        ++skips;
        continue;
      }

      cell->write(result);
      if(++count == specs.flags.limit)
        // specs.flags.limit_by && specs.flags.max_versions
        break;
    } else {
      ++skips;
    }
  }
}

SWC_SHOULD_INLINE
bool Mutable::scan_after(const DB::Cell::Key& after,
                         const DB::Cell::Key& to, Mutable& cells) const {
  if(!_size)
    return false;

  const Cell* cell;
  for(auto it=ConstIterator(&buckets, _narrow(after)); it; ++it) {
    cell = *it.item;
    if(!to.empty()
        && DB::KeySeq::compare(key_seq, to, cell->key) == Condition::GT)
      return false;
    if(cell->has_expired(ttl) || (!after.empty()
        && DB::KeySeq::compare(key_seq, after, cell->key) != Condition::GT))
      continue;

    cells.add_raw(*cell);
  }
  return true;
}


bool Mutable::split(Mutable& cells, bool loaded) {
  auto it = ConstIterator(&buckets, _size / 2);
  if(!it)
    return false;
  Cell* from_cell = *it.item;
  size_t count = _size;
  bool from_set = false;
  Iterator it_start;
  Cell* cell;
  for(auto it = Iterator(&buckets, _narrow(from_cell->key)); it; ++it) {
    cell = *it.item;

    if(!from_set) {
      if(DB::KeySeq::compare(key_seq, cell->key, from_cell->key)
           == Condition::GT) {
       --count;
        continue;
      }

      it_start = it;
      if(!loaded)
        break;
      from_set = true;
    }
    _remove(cell);
    if(cell->has_expired(ttl))
      delete cell;
    else
      cells.add_sorted_no_cpy(cell);
  }

  _remove(it_start, count, !loaded);
  return true;
}

void Mutable::split(Mutable& cells) {
  size_t split_at = buckets.size() / 2;
  if(!split_at)
    return;

  delete *cells.buckets.begin();
  cells.buckets.clear();
  cells._size = 0;
  cells._bytes = 0;

  auto it = buckets.begin() + split_at;
  cells.buckets.assign(it, buckets.end());
  buckets.erase(it, buckets.end());
  for(auto bucket : cells.buckets) {
    for(auto cell : *bucket) {
      cells._add(cell);
    }
  }
  _size -= cells._size;
  _bytes -= cells._bytes;
}


void Mutable::_add_remove(const Cell& e_cell, size_t* offsetp) {
  int64_t ts = e_cell.get_timestamp();
  int64_t rev;
  bool chk_rev = (rev = e_cell.get_revision()) != AUTO_ASSIGN;
  Condition::Comp cond;
  int64_t e_ts;
  Cell* cell;
  for(auto it = Iterator(&buckets, *offsetp); it; ) {

    if((cell=*it.item)->has_expired(ttl)) {
      _remove(it);
      continue;
    }

    if((cond = DB::KeySeq::compare(key_seq, cell->key, e_cell.key))
                == Condition::GT) {
      ++it;
      ++*offsetp;
      continue;
    }

    if(cond == Condition::LT) {
      _insert(it, e_cell);
      return;
    }

    if(cell->is_removing(ts) ||
      (chk_rev && (e_ts = cell->get_revision()) != AUTO_ASSIGN && e_ts >= rev))
      return;

    if(e_cell.is_removing(cell->get_timestamp()))
      _remove(it);
    else
      ++it;
  }

  add_sorted(e_cell);
}

void Mutable::_add_plain(const Cell& e_cell, size_t* offsetp) {
  int64_t ts;
  bool chk_ts = (ts = e_cell.get_timestamp()) != AUTO_ASSIGN;
  int64_t rev;
  bool chk_rev = (rev = e_cell.get_revision()) != AUTO_ASSIGN;

  uint32_t revs = 0;
  Condition::Comp cond;
  Cell* cell;
  for(auto it = Iterator(&buckets, *offsetp); it;) {

    if((cell=*it.item)->has_expired(ttl)) {
      _remove(it);
      continue;
    }

    if((cond = DB::KeySeq::compare(key_seq, cell->key, e_cell.key))
               == Condition::GT) {
      ++it;
      ++*offsetp;
      continue;
    }

    if(cond == Condition::LT) {
      _insert(it, e_cell);
      return;
    }

    if(chk_rev && cell->get_revision() == rev)
      return;

    if(cell->removal()) {
      if(chk_ts && cell->is_removing(ts))
        return;
      ++it;
      continue;
    }

    if(chk_ts && cell->get_timestamp() == ts) {
      if(!chk_rev || rev > cell->get_revision())
        return cell->copy(e_cell);
      return;
    }

    ++revs;
    if(e_cell.control & TS_DESC
        ? (chk_ts && ts < cell->get_timestamp())
        : (!chk_ts || ts > cell->get_timestamp()) ) {
      if(max_revs == revs)
        return;
      ++it;
      continue;
    }

    if(max_revs == revs) {
      if(!chk_rev || rev > cell->get_revision())
        cell->copy(e_cell);
    } else {
      _insert(it, e_cell);
      ++it;
      _remove_overhead(it, e_cell.key, revs);
    }
    return;
  }

  add_sorted(e_cell);
}

void Mutable::_add_counter(const Cell& e_cell, size_t* offsetp) {
  Condition::Comp cond;

  int64_t ts = e_cell.get_timestamp();
  int64_t rev;
  bool chk_rev = (rev = e_cell.get_revision()) != AUTO_ASSIGN;

  Cell* cell;
  auto it = Iterator(&buckets, *offsetp);
  while(it) {

    if((cell=*it.item)->has_expired(ttl)) {
      _remove(it);
      continue;
    }

    if((cond = DB::KeySeq::compare(key_seq, cell->key, e_cell.key))
                == Condition::GT) {
      ++it;
      ++*offsetp;
      continue;
    }

    if(cond == Condition::LT) //without aggregate|| ts == AUTO_ASSIGN
      goto add_counter;

    if(cell->removal()) {
      if((chk_rev && cell->get_revision() >= rev) ||
         cell->is_removing(ts) )
        return;
      ++it;
      continue;
    }

    uint8_t op_1;
    int64_t eq_rev_1;
    int64_t value_1 = cell->get_counter(op_1, eq_rev_1);
    if(op_1 & OP_EQUAL) {
      if(!(op_1 & HAVE_REVISION))
        eq_rev_1 = cell->get_timestamp();
      if(eq_rev_1 > ts)
        return;
    }

    if(e_cell.get_counter_op() & OP_EQUAL) {
      cell->copy(e_cell);
    } else {
      value_1 += e_cell.get_counter();
      cell->set_counter(op_1, value_1, type, eq_rev_1);
      if(cell->timestamp < e_cell.timestamp) {
        cell->timestamp = e_cell.timestamp;
        cell->control = e_cell.control;
      }
    }
    return;
  }

  add_counter:
    cell = _insert(it, e_cell);
    if(type != Types::Column::COUNTER_I64) {
      uint8_t op_1;
      int64_t eq_rev_1;
      int64_t value_1 = cell->get_counter(op_1, eq_rev_1);
      cell->set_counter(op_1, value_1, type, eq_rev_1);
    }
}


size_t Mutable::_narrow(const DB::Cell::Key& key, size_t offset) const {
  if(key.empty() || _size <= narrow_size)
    return 0;
  size_t step = _size;
  if(1 < offset && offset < _size) {
    if((step -= offset) <= narrow_size)
      step = narrow_size;
  } else {
    offset = step >>= 1;
  }

  try_narrow:
    auto it = ConstIterator(&buckets, offset);
    if(!it)
      return 0;
    if(DB::KeySeq::compare(key_seq, (*it.item)->key, key) == Condition::GT) {
      if(step < narrow_size + max_revs)
        return offset;
      if(offset + (step >>= 1) >= _size)
        offset = _size - (step >>= 1);
      else
        offset += step;
      goto try_narrow;
    }
    if((step >>= 1) <= narrow_size)
      step += max_revs;
    if(offset < step)
      return 0;
    offset -= step;
  goto try_narrow;
}




}}}
