/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/Cells/Vector.h"


namespace SWC { namespace DB { namespace Cells {

Vector::Ptr Vector::make(const uint32_t max_revs, 
                         const uint64_t ttl_ns, 
                         const Types::Column type) {
  return std::make_shared<Vector>(max_revs, ttl_ns, type);
}

Vector::Vector(const uint32_t max_revs, const uint64_t ttl_ns, 
               const Types::Column type)
              : bytes(0), type(type), max_revs(max_revs),  ttl(ttl_ns) {
}

Vector::Vector(Vector& other)
              : std::vector<Cell*>(other), bytes(other.bytes), 
                type(other.type), 
                max_revs(other.max_revs), ttl(other.ttl) {
  other.clear();
  other.bytes = 0;
}

Vector::~Vector() {
  free();
}

void Vector::free() {
  for(auto cell : *this)
    if(cell)
      delete cell;
  bytes = 0;
  clear();
  shrink_to_fit();
}

void Vector::reset(const uint32_t revs, const uint64_t ttl_ns, 
                   const Types::Column typ) {
  free();
  configure(revs, ttl_ns, typ);
}

void Vector::configure(const uint32_t revs, const uint64_t ttl_ns, 
                       const Types::Column typ) {
  type = typ;
  max_revs = revs;
  ttl = ttl_ns;
}


const size_t Vector::size_bytes() const {
  return bytes;
}

void Vector::take_sorted(Vector& other) {
  bytes += other.bytes;
  insert(end(), other.begin(), other.end());
  
  other.clear();
  other.bytes = 0;
}


void Vector::add_sorted(const Cell& cell, bool no_value) {
  Cell* adding;
  _push_back(adding = new Cell(cell, no_value));
  bytes += adding->encoded_length();
}

void Vector::add_sorted_no_cpy(Cell* cell) {
  _push_back(cell);
  bytes += cell->encoded_length();
}

const size_t Vector::add_sorted(const uint8_t* ptr, size_t remain) {
  size_t count = 0;
  bytes += remain;
  while(remain) {
    _push_back(new Cell(&ptr, &remain, true));
    ++count;
  }
  return count;
}


void Vector::add_raw(const DynamicBuffer& cells) {
  Cell cell;
  const uint8_t* ptr = cells.base;
  size_t remain = cells.fill();
  while(remain) {
    cell.read(&ptr, &remain);
    add_raw(cell);
  }
}

void Vector::add_raw(const DynamicBuffer& cells, const DB::Cell::Key& from_key) {
  Cell cell;
  const uint8_t* ptr = cells.base;
  size_t remain = cells.fill();
  while(remain) {
    cell.read(&ptr, &remain);
    if(from_key.compare(cell.key) == Condition::GT)
      add_raw(cell);
  }
}

void Vector::add_raw(const Cell& e_cell) {
  if(e_cell.has_expired(ttl))
    return;

  size_t offset = _narrow(e_cell.key);

  if(e_cell.removal()) {
    _add_remove(e_cell, offset);

  } else {
    if(Types::is_counter(type))
      _add_counter(e_cell, offset);
    else
      _add_plain(e_cell, offset);
  }
}


Cell* Vector::takeout_begin(size_t idx) {
  auto it = begin() + idx;
  Cell* cell = *it;
  erase(it);
  bytes -= cell->encoded_length();
  return cell;
}

Cell* Vector::takeout_end(size_t idx) {
  auto it = end() - idx;
  Cell* cell = *it;
  erase(it);
  bytes -= cell->encoded_length();
  return cell;
}


void Vector::write(DynamicBuffer& cells) const {
  cells.ensure(bytes);
  for(auto cell : *this) {
    if(!cell->has_expired(ttl))
      cell->write(cells);
  }
}

void Vector::write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                            Interval& intval, uint32_t threshold, 
                            uint32_t max_cells) {
  if(empty())
    return;
    
  cells.ensure(bytes < threshold? bytes: threshold);
  Cell* first = nullptr;
  Cell* last = nullptr;
  auto it = begin();
  for(Cell* cell; it < end() && ((!threshold || threshold > cells.fill()) && 
                  (!max_cells || max_cells > cell_count) ); ++it) {
    if((cell = *it)->has_expired(ttl))
      continue;

    cell->write(cells);
    intval.expand(cell->timestamp);
    cell->key.align(intval.aligned_min, intval.aligned_max);
    (first ? last : first) = cell;
    ++cell_count;
  }

  if(first) {
    intval.expand_begin(*first);
    intval.expand_end(*(last ? last : first));
  }
  
  if(it == end())
    free();
  else
    _remove(begin(), it);
}

bool Vector::write_and_free(const DB::Cell::Key& key_start, 
                            const DB::Cell::Key& key_finish,
                            DynamicBuffer& cells, uint32_t threshold) {
  if(empty())
    return false;

  size_t count = 0;
  auto offset_it = end();
  
  cells.ensure(bytes < threshold? bytes: threshold);
  
  for(auto it = begin() + _narrow(key_start); it < end(); ++it) {
    if(!key_start.empty() && 
        key_start.compare((*it)->key, 0) == Condition::LT) 
      continue;
    if(!key_finish.empty() && 
        key_finish.compare((*it)->key, 0) == Condition::GT)
      break;

    ++count;
    if(offset_it == end())
      offset_it = it;

    if((*it)->has_expired(ttl))
      continue;

    (*it)->write(cells);
    if(threshold < cells.fill())
      break;
  }
  
  if(count) {
    if(size() == count) {
      free();
    } else {
      _remove(offset_it, offset_it + count);
      return true;
    }
  }
  return false;
}


const std::string Vector::to_string(bool with_cells) const {
  std::string s("Cells(size=");
  s.append(std::to_string(size()));
  s.append(" bytes=");
  s.append(std::to_string(bytes));
  s.append(" type=");
  s.append(Types::to_string(type));
  s.append(" max_revs=");
  s.append(std::to_string(max_revs));
  s.append(" ttl=");
  s.append(std::to_string(ttl));
  if(with_cells) {
    s.append(" cells=[\n");
    for(auto cell : *this) {
      s.append(cell->to_string(type));
      s.append("\n");
    }
    s.append("]");
  }
  s.append(")");
  return s;
}


const bool Vector::has_one_key() const {
  return front()->key.compare(back()->key) == Condition::EQ;
}

void Vector::get(int32_t idx, DB::Cell::Key& key) const {
  key.copy((*((idx < 0 ? end() : begin()) + idx))->key);
}
 
const bool Vector::get(const DB::Cell::Key& key, Condition::Comp comp, 
                       DB::Cell::Key& res) const {
  Condition::Comp chk;
  for(auto it = begin() + _narrow(key); it < end(); ++it) {
    if((chk = key.compare((*it)->key, 0)) == Condition::GT 
      || (comp == Condition::GE && chk == Condition::EQ)){
      res.copy((*it)->key);
      return true;
    }
  }
  return false;
}


void Vector::scan(const Specs::Interval& specs, Vector& cells, 
                  size_t& cell_offset, 
                  const std::function<bool()>& reached_limits, 
                  size_t& skips, const Selector_t& selector) const {
  if(empty())
    return;
  if(max_revs == 1) 
    scan_version_single(
      specs, cells, cell_offset, reached_limits, skips, selector);
  else
    scan_version_multi(
      specs, cells, cell_offset, reached_limits, skips, selector);
}

void Vector::scan_version_single(const Specs::Interval& specs, Vector& cells, 
                                 size_t& cell_offset, 
                                 const std::function<bool()>& reached_limits, 
                                 size_t& skips, 
                                 const Selector_t& selector) const {
  bool stop = false;
  bool only_deletes = specs.flags.is_only_deletes();
  bool only_keys = specs.flags.is_only_keys();

  size_t offset = specs.offset_key.empty()? 0 : _narrow(specs.offset_key);
                                             // ?specs.key_start
  Cell* cell;
  for(auto it = begin() + offset; !stop && it < end(); ++it){

    if(!(cell = *it)->has_expired(ttl) &&
       (only_deletes ? cell->flag != INSERT : cell->flag == INSERT) &&
       selector(*cell, stop)) {
      
      if(cell_offset) {
        --cell_offset;
        ++skips;  
        continue;
      }

      cells.add_sorted(*cell, only_keys);
      if(reached_limits())
        break;
    } else 
      ++skips;
  }
}

void Vector::scan_version_multi(const Specs::Interval& specs, Vector& cells, 
                                size_t& cell_offset, 
                                const std::function<bool()>& reached_limits, 
                                size_t& skips, 
                                const Selector_t& selector) const {
  bool stop = false;
  bool only_deletes = specs.flags.is_only_deletes();
  bool only_keys = specs.flags.is_only_keys();
  
  bool chk_align;
  uint32_t rev;
  size_t offset;
  if((chk_align = !specs.offset_key.empty())) {
    rev = cells.max_revs;
    offset = _narrow(specs.offset_key);// ?specs.key_start
   } else {
    rev = 0;
    offset = 0;
  }
  Cell* cell;
  for(auto it = begin() + offset; !stop && it < end(); ++it) {
    cell = *it;

    if((only_deletes ? cell->flag == INSERT : cell->flag != INSERT) || 
       cell->has_expired(ttl)) {
      ++skips;
      continue;
    }

    if(chk_align) switch(specs.offset_key.compare(cell->key)) {
      case Condition::LT: {
        ++skips;
        continue;
      }
      case Condition::EQ: {
        if(!rev ||
           !specs.is_matching(cell->timestamp, cell->control & TS_DESC)) {
          if(rev)
            --rev;
          //if(cell_offset && selector(*cell, stop))
          //  --cell_offset;
          ++skips;
          continue;
        }
      }
      default:
        chk_align = false;
        break;
    }

    if(!selector(*cell, stop)) {
      ++skips;
      continue;
    }
    if(!cells.empty() && 
       cells.back()->key.compare(cell->key) == Condition::EQ) {
      if(!rev) {
        ++skips;
        continue;
      }
    } else {
      rev = cells.max_revs;
    }

    if(cell_offset) {
      --cell_offset;
      ++skips;
      continue;
    }

    cells.add_sorted(*cell, only_keys);
    if(reached_limits())
      break;
    --rev;
  }
}

void Vector::scan_test_use(const Specs::Interval& specs, 
                           DynamicBuffer& result, 
                           size_t& count, size_t& skips) const {
  Cell* cell;
  uint cell_offset = specs.flags.offset;
  bool only_deletes = specs.flags.is_only_deletes();

  for(auto it = begin(); it < end(); ++it) {

    if(!(cell = *it)->has_expired(ttl) && 
       (only_deletes ? cell->flag != INSERT : cell->flag == INSERT) &&
       specs.is_matching(*cell, type)) {

      if(cell_offset) {
        --cell_offset;
        ++skips;
        continue;
      }
      
      cell->write(result);
      if(++count == specs.flags.limit) 
        // specs.flags.limit_by && specs.flags.max_versions
          break;
    } else 
      ++skips;
  }
}

void Vector::scan(Interval& interval, Vector& cells) const {
  if(empty())
    return;

  Cell* cell;
  for(auto it = begin() + _narrow(interval.key_begin); it < end(); ++it) {
    if((cell = *it)->has_expired(ttl) || (!interval.key_begin.empty() 
        && interval.key_begin.compare(cell->key) == Condition::LT))
      continue;
    if(!interval.key_end.empty() 
        && interval.key_end.compare(cell->key) == Condition::GT)
      break; 

    cells.add_raw(*cell);
  }
}


void Vector::expand(Interval& interval) const {
  expand_begin(interval);
  if(size() > 1)
    expand_end(interval);
}

void Vector::expand_begin(Interval& interval) const {
  interval.expand_begin(*front());
}

void Vector::expand_end(Interval& interval) const {
  interval.expand_end(*back());
}


void Vector::split(size_t from, Vector& cells, 
                   Interval& intval_1st, Interval& intval_2nd, 
                   bool loaded) {
  Cell* from_cell = *(begin() + from);
  auto it_start = end();

  for(auto it = begin() + _narrow(from_cell->key); it < end(); ++it) {

    if(it_start == end()) {
      if((*it)->key.compare(from_cell->key, 0) == Condition::GT)
        continue;

      intval_2nd.expand_begin(**it);
      it_start = it;
      if(!loaded)
        break;
    }

    bytes -= (*it)->encoded_length();
    if((*it)->has_expired(ttl))
      delete *it; 
    else
      cells.add_sorted_no_cpy(*it);
  }

  if(loaded)
    erase(it_start, end());
  else
    _remove(it_start, end());

  intval_2nd.set_key_end(intval_1st.key_end);      
  intval_1st.key_end.free();
  expand_end(intval_1st);
}


void Vector::_add_remove(const Cell& e_cell, size_t offset) {
  int64_t ts = e_cell.get_timestamp();
  int64_t rev;
  bool chk_rev = (rev = e_cell.get_revision()) != AUTO_ASSIGN;
  Condition::Comp cond;
  Cell* cell;
  for(auto it = begin() + offset; it < end(); ++it) {

    if((cell = *it)->has_expired(ttl)) {
      _remove(it--);
      continue;
    }

    if((cond = cell->key.compare(e_cell.key, 0)) == Condition::GT)
      continue;

    if(cond == Condition::LT) 
      return _insert(it, e_cell);

    if((chk_rev && cell->get_revision() >= rev) ||
       (cell->removal() && cell->is_removing(ts)) )
      return;
    
    if(e_cell.is_removing(cell->get_timestamp()))
      _remove(it--);
  }
  
  add_sorted(e_cell);
}

void Vector::_add_plain(const Cell& e_cell, size_t offset) {
  int64_t ts = e_cell.get_timestamp();
  int64_t rev;
  bool chk_rev = (rev = e_cell.get_revision()) != AUTO_ASSIGN;

  uint32_t revs = 0;
  Condition::Comp cond;
  Cell* cell;
  for(auto it = begin() + offset; it < end(); ++it) {

    if((cell = *it)->has_expired(ttl)) {
      _remove(it--);
      continue;
    }

    if((cond = cell->key.compare(e_cell.key, 0)) == Condition::GT)
      continue;

    if(cond == Condition::LT)
      return _insert(it, e_cell);

    if(chk_rev && cell->get_revision() >= rev)
      return;

    if(cell->removal()) {
      if(cell->is_removing(ts))
        return;
      continue;
    }

    if(ts != AUTO_ASSIGN && cell->get_timestamp() == ts)
      return cell->copy(e_cell);
    
    ++revs;
    if(e_cell.control & TS_DESC 
        ? e_cell.timestamp < cell->timestamp
          : e_cell.timestamp > cell->timestamp) {
      if(max_revs == revs)
        return;
      continue;
    }
    
    if(max_revs == revs) {
      cell->copy(e_cell);
    } else {
      size_t offset = it - begin();
      _insert(it, e_cell);
      _remove_overhead(begin() + offset + 1, e_cell.key, revs);
    }
    return;
  }

  add_sorted(e_cell);
}

void Vector::_add_counter(const Cell& e_cell, size_t offset) {
  Condition::Comp cond;

  int64_t ts = e_cell.get_timestamp();
  int64_t rev;
  bool chk_rev = (rev = e_cell.get_revision()) != AUTO_ASSIGN;

  auto add_at = end();  
  Cell* cell;
  for(auto it = begin() + offset; it < end(); ++it) {

    if((cell = *it)->has_expired(ttl)) {
      _remove(it--);
      add_at = end();
      continue;
    }

    if((cond = cell->key.compare(e_cell.key, 0)) == Condition::GT)
      continue;

    if(cond == Condition::LT) { //without aggregate|| ts == AUTO_ASSIGN
        add_at = it;
      goto add_counter;
    }

    if(chk_rev && cell->get_revision() >= rev)
      return;

    if(cell->removal()) {
      if(cell->is_removing(ts))
        return;
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
    
    if(e_cell.get_counter_op() & OP_EQUAL)
      cell->copy(e_cell);
    else {
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
    _insert(add_at, e_cell);
    if(type != Types::Column::COUNTER_I64) {
      cell = back();
      uint8_t op_1;
      int64_t eq_rev_1;
      int64_t value_1 = cell->get_counter(op_1, eq_rev_1);
      cell->set_counter(op_1, value_1, type, eq_rev_1);
    }
}


size_t Vector::_narrow(const DB::Cell::Key& key) const {
  if(key.empty() || size() <= narrow_sz) 
    return 0;
  size_t sz;
  size_t offset = sz = size() >> 1;

  try_narrow:
    if((*(begin() + offset))->key.compare(key, 0) == Condition::GT) {
      if(sz < narrow_sz)
        return offset;
      offset += sz >>= 1; 
      goto try_narrow;
    }
    if((sz >>= 1) == 0)
      ++sz;  

    if(offset < sz)
      return 0;
    offset -= sz;
  goto try_narrow;
}


void Vector::_push_back(Cell* cell) {
  if(capacity() == size())
    reserve(capacity() + (capacity() >> 1) + 1000);
  push_back(cell);
}

void Vector::_insert(iterator at, const Cell& cell) {
  if(capacity() == size()) {
    size_t offset = at - begin();
    reserve(capacity() + (capacity() >> 1) + 1000);
    at = begin() + offset;
  }
  push_back(nullptr);
  auto it = end();
  while(--it > at)
    *it = *(it-1);
  bytes += (*it = new Cell(cell))->encoded_length();
}

void Vector::_remove(iterator it) {
  bytes -= (*it)->encoded_length();
  delete *it; 
  erase(it);
}

void Vector::_remove(iterator it_begin, iterator it_end) {
  auto it = it_end;
  do {
    --it;
    bytes -= (*it)->encoded_length();
    delete *it; 
  } while(it > it_begin);
  erase(it_begin, it_end);
}

void Vector::_remove_overhead(iterator it, const DB::Cell::Key& key, 
                              uint32_t revs) {
  for(; it < end(); ++it) {
    if((*it)->key.compare(key, 0) != Condition::EQ)
      return;

    if((*it)->flag != INSERT)
      continue;

    if(++revs > max_revs)
      _remove(it--);
  }
}

}}}