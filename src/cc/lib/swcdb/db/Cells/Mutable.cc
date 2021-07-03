/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/Mutable.h"


namespace SWC { namespace DB { namespace Cells {



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
  Iterator it_start = get<Iterator>();
  for(auto it=it_start; it && (!threshold || threshold > cells.fill()) &&
                              (!max_cells || max_cells > cell_count); ++it) {
    ++count;
    if((cell=it.item())->has_expired(ttl))
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
  Iterator it_start = get<Iterator>();
  {
  Iterator it = get<Iterator>(key_start);
  for(Cell* cell; it && (!threshold || threshold > cells.fill()); ++it) {
    cell = it.item();

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
  Iterator it_start = get<Iterator>();
  for(auto it=it_start; it && (!threshold || threshold>cells.fill()); ++it) {
    ++count;
    if(!it.item()->has_expired(ttl))
      it.item()->write(cells);
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
  out << "Cells(size=" << _size
      << " bytes=" << _bytes
      << " type=" << Types::to_string(type)
      << " max_revs=" << max_revs
      << " ttl=" << ttl
      << " sizeof-container=" << _container.size_of_internal();
  if(with_cells) {
    size_t count = 0;
    out << " [\n";
    for(auto it = get<ConstIterator>(); it; ++it) {
      it.item()->print(out << '\n', type);
      ++count;
    }
    out << "] counted=" << count;
    _container.print(out << "\n container=");
  }

  out << ')';
}


void Mutable::scan_version_single(ReqScan* req) const {
  bool stop = false;
  const bool only_deletes = req->spec.flags.is_only_deletes();
  const Cell* cell;
  for(auto it = get<ConstIterator>(req->spec); !stop && it; ++it) {
    cell = it.item();

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
  for(auto it = get<ConstIterator>(req->spec); !stop && it; ++it) {
    cell = it.item();

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
  //std::cout << "## scan_test_use" << std::endl;
  bool stop = false;
  uint32_t cell_offset = specs.flags.offset;
  const bool only_deletes = specs.flags.is_only_deletes();
  const Cell* cell;
  for(auto it = get<ConstIterator>(); !stop && it; ++it) {
    cell = it.item();
    //cell->print(std::cout << "\n scan_test_use ", DB::Types::Column::PLAIN);

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


bool Mutable::split(Mutable& cells, bool loaded) {
  auto it = get<ConstIterator>(_size / 2);
  if(!it)
    return false;
  const Cell* from_cell = it.item();
  size_t count = _size;
  bool from_set = false;
  Iterator it_start = get<Iterator>();
  Cell* cell;
  for(auto it = get<Iterator>(from_cell->key); it; ++it) {
    cell = it.item();

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

void Mutable::_add_remove(const Cell& e_cell, Mutable::Iterator& it,
                          size_t& offset) {
  int64_t ts = e_cell.get_timestamp();
  int64_t rev = e_cell.get_revision();
  bool chk_rev = rev != AUTO_ASSIGN;
  Condition::Comp cond;
  int64_t e_ts;
  for(Cell* cell; it; ) {

    if((cell=it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }

    if((cond = DB::KeySeq::compare(key_seq, cell->key, e_cell.key))
                == Condition::GT) {
      ++it;
      ++offset;
      continue;
    }

    if(cond == Condition::LT) {
      _insert(it, e_cell);
      return;
    }

    if(cell->is_removing(ts) ||
       (chk_rev &&
        (e_ts = cell->get_revision()) != AUTO_ASSIGN &&
        e_ts >= rev))
      return;

    if(e_cell.is_removing(cell->get_timestamp()))
      _remove(it);
    else
      ++it;
  }

  add_sorted(e_cell);
}

void Mutable::_add_plain(const Cell& e_cell, Mutable::Iterator& it,
                         size_t& offset) {
  int64_t ts = e_cell.get_timestamp();
  int64_t rev = e_cell.get_revision();
  bool chk_ts = ts != AUTO_ASSIGN;
  bool chk_rev = rev != AUTO_ASSIGN;

  uint32_t revs = 0;
  Condition::Comp cond;
  for(Cell* cell; it; ) {

    if((cell=it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }

    if((cond = DB::KeySeq::compare(key_seq, cell->key, e_cell.key))
               == Condition::GT) {
      ++it;
      ++offset;
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

void Mutable::_add_counter(const Cell& e_cell, Mutable::Iterator& it,
                           size_t& offset) {
  int64_t ts = e_cell.get_timestamp();
  int64_t rev = e_cell.get_revision();
  bool chk_rev = rev != AUTO_ASSIGN;

  Condition::Comp cond;
  for(Cell* cell; it; ) {

    if((cell=it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }

    if((cond = DB::KeySeq::compare(key_seq, cell->key, e_cell.key))
                == Condition::GT) {
      ++it;
      ++offset;
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
    Cell* cell = _insert(it, e_cell);
    if(type != Types::Column::COUNTER_I64) {
      uint8_t op_1;
      int64_t eq_rev_1;
      int64_t value_1 = cell->get_counter(op_1, eq_rev_1);
      cell->set_counter(op_1, value_1, type, eq_rev_1);
    }
}






}}}
