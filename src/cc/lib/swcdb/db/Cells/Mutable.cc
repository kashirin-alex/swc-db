/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/Mutable.h"


namespace SWC { namespace DB { namespace Cells {


void Mutable::configure(const uint32_t revs, const uint64_t ttl_ns,
                        const Types::Column typ, bool finalized) {
  bool chk_revs = max_revs != revs && (!Types::is_counter(type) || finalized);
  bool chk_ttl = ttl != ttl_ns;
  type = typ;
  ttl = ttl_ns;
  max_revs = revs;
  if(empty() || (!chk_revs && !chk_ttl))
    return;
  Cell* cell;
  for(auto it = get<Iterator>(); it; ) {
    if((cell = it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }
    ++it;
    if(chk_revs)
      _remove_overhead(it, cell->key, 1);
  }
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
  Iterator it_start = get<Iterator>();
  for(auto it=it_start; it && (!threshold || threshold > cells.fill()) &&
                              (!max_cells || max_cells > cell_count); ++it) {
    ++count;
    if((cell=it.item())->has_expired(ttl))
      continue;

    cell->write(cells);
    intval.expand(cell->get_timestamp());
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
         !req->spec.is_matching(
           cell->get_timestamp(), cell->is_time_order_desc())) {
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


void Mutable::_add_remove(const Cell& e_cell, Mutable::Iterator& it,
                          size_t& offset) {
  for(Cell* cell; it; ) {
    if((cell = it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }
    switch(DB::KeySeq::compare(key_seq, cell->key, e_cell.key)) {
      case Condition::GT: {
        ++it;
        ++offset;
        break;
      }
      case Condition::LT: {
        _insert(it, e_cell);
        return;
      }
      default: {
        bool chk_rev = e_cell.get_revision() != TIMESTAMP_AUTO;
        bool rev_set = cell->get_revision() != TIMESTAMP_AUTO;
        if(chk_rev && (
            cell->get_revision() == e_cell.get_revision()
              ||
            (cell->removal() &&
             (!rev_set || cell->get_revision() > e_cell.get_revision()) &&
             cell->is_removing(e_cell.get_timestamp())) ) ) {
          return;
        }
        if((!chk_rev ||
            (rev_set && e_cell.get_revision() > cell->get_revision())
           ) && e_cell.is_removing(cell->get_timestamp()) ) {
          _remove(it);
        } else {
          ++it;
        }
        break;
      }
    }
  }
  add_sorted(e_cell);
}

void Mutable::_add_unfinalized(const Cell& e_cell,
                               Mutable::Iterator& it,
                               size_t& offset) {
  for(Cell* cell; it; ) {
    if((cell = it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }
    switch(DB::KeySeq::compare(key_seq, cell->key, e_cell.key)) {
      case Condition::GT: {
        ++it;
        ++offset;
        break;
      }
      case Condition::LT: {
        _insert(it, e_cell);
        return;
      }
      default: {
        if(e_cell.get_revision() == TIMESTAMP_AUTO) {
          ++it;
          break;
        }
        if(cell->get_revision() != TIMESTAMP_AUTO) {
          if(cell->get_revision() == e_cell.get_revision()) {
            return;
          }
          if(e_cell.get_revision() > cell->get_revision()) {
            ++it;
            break;
          }
          if(cell->removal() && cell->is_removing(e_cell.get_timestamp())) {
            return;
          }
        }
        _insert(it, e_cell);
        return;
      }
    }
  }

  add_sorted(e_cell);
}


void Mutable::_add_plain_version_single(const Cell& e_cell,
                                        Mutable::Iterator& it,
                                        size_t& offset) {
  for(Cell* cell; it; ) {
    if((cell = it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }
    switch(DB::KeySeq::compare(key_seq, cell->key, e_cell.key)) {
      case Condition::GT: {
        ++it;
        ++offset;
        break;;
      }
      case Condition::LT: {
        _insert(it, e_cell);
        return;
      }
      default: {
        bool chk_rev = e_cell.get_revision() != TIMESTAMP_AUTO;
        if(cell->removal()) {
          if(chk_rev &&
             cell->get_revision() > e_cell.get_revision() &&
             cell->is_removing(e_cell.get_timestamp()) )
            return;
          ++it;
          break;
        }
        if(!chk_rev || e_cell.get_revision() > cell->get_revision()) {
          _adjust_copy(*cell, e_cell);
        }
        return;
      }
    }
  }
  add_sorted(e_cell);
}

void Mutable::_add_plain_version_multi(const Cell& e_cell,
                                       Mutable::Iterator& it,
                                       size_t& offset) {
  bool chk_ts = e_cell.get_timestamp() != TIMESTAMP_AUTO;
  bool chk_rev = e_cell.get_revision() != TIMESTAMP_AUTO;
  uint32_t revs = 0;

  for(Cell* cell; it; ) {
    if((cell = it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }
    switch(DB::KeySeq::compare(key_seq, cell->key, e_cell.key)) {
      case Condition::GT: {
        ++it;
        ++offset;
        continue;
      }
      case Condition::LT: {
        _insert(it, e_cell);
        return;
      }
      default: {
        break;
      }
    }

    if(chk_rev && cell->get_revision() == e_cell.get_revision())
      return;

    if(cell->removal()) {
      if(chk_rev &&
         cell->get_revision() > e_cell.get_revision() &&
         cell->is_removing(e_cell.get_timestamp()) )
        return;
      ++it;
      continue;
    }

    if(chk_ts && cell->get_timestamp() == e_cell.get_timestamp()) {
      if(!chk_rev || e_cell.get_revision() > cell->get_revision())
        _adjust_copy(*cell, e_cell);
      return;
    }

    ++revs;
    if(e_cell.is_time_order_desc()
        ? (chk_ts && e_cell.get_timestamp() < cell->get_timestamp())
        : (!chk_ts || e_cell.get_timestamp() > cell->get_timestamp()) ) {
      if(max_revs == revs)
        return;
      ++it;
      continue;
    }

    if(max_revs == revs) {
      if(!chk_rev || e_cell.get_revision() > cell->get_revision())
        _adjust_copy(*cell, e_cell);
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
  for(Cell* cell; it; ) {
    if((cell = it.item())->has_expired(ttl)) {
      _remove(it);
      continue;
    }
    switch(DB::KeySeq::compare(key_seq, cell->key, e_cell.key)) {
      case Condition::GT: {
        ++it;
        ++offset;
        break;
      }
      case Condition::LT: {
        goto add_counter;
      }
      default: {
        if(cell->removal()) {
          if(e_cell.get_revision() != TIMESTAMP_AUTO &&
             cell->get_revision() > e_cell.get_revision() &&
             cell->is_removing(e_cell.get_timestamp()) )
            return;
          ++it;
          break;
        }

        uint8_t op;
        int64_t eq_rev;
        int64_t value = cell->get_counter(op, eq_rev);
        if(op & OP_EQUAL) {
          if(eq_rev == TIMESTAMP_NULL &&
             cell->get_timestamp() != TIMESTAMP_AUTO) {
            eq_rev = cell->get_timestamp();
          }
          if(eq_rev > e_cell.get_timestamp())
            return;
        }

        int64_t value_2;
        if(e_cell.get_counter(value_2) & OP_EQUAL) {
          _adjust_copy(*cell, e_cell);
        } else {
          _bytes -= cell->encoded_length();
          value += value_2;
          cell->set_counter(op, value, type, eq_rev);
          int64_t ts = (e_cell.get_timestamp() == TIMESTAMP_AUTO ||
                        e_cell.get_timestamp() > cell->get_timestamp())
                      ? e_cell.get_timestamp() : cell->get_timestamp();
          int64_t rev =(e_cell.get_revision() == TIMESTAMP_AUTO ||
                        e_cell.get_revision() > cell->get_revision())
                      ? e_cell.get_revision() : cell->get_revision();
          if(ts == TIMESTAMP_AUTO) {
            cell->set_timestamp_auto();
            if(rev != TIMESTAMP_AUTO)
              cell->set_revision(rev);
          } else if(ts == rev) {
            cell->set_timestamp_with_rev_is_ts(ts);
          } else {
            cell->set_timestamp(ts);
            cell->set_revision(rev);
          }
          _bytes += cell->encoded_length();
        }
        return;
      }
    }
  }

  add_counter: {
    bool no_value = type != Types::Column::COUNTER_I64;
    Cell* cell = new Cell(e_cell, no_value);
    if(no_value) {
      uint8_t op;
      int64_t eq_rev;
      int64_t value = e_cell.get_counter(op, eq_rev);
      cell->set_counter(op, value, type, eq_rev);
    }
    _add(*cell);
    it ? it.insert(cell) : _container.push_back(cell);
  }
}


void Mutable::_finalize_counter() {
  Mutable  finalized_cells(key_seq, max_revs, ttl, type);
  Iterator it = get<Iterator>();
  Iterator agg_it(it);
  Cell*    agg = nullptr;
  bool     agg_state = false;
  int64_t  agg_rev = 0;
  int64_t  agg_ts = 0;
  uint8_t  agg_op = 0;
  int64_t  agg_eq_rev = 0;
  int64_t  agg_value = 0;
  for(Cell* cell; ;) {

    if(!agg || agg_it == it) {
      for(; agg_it &&
            ((agg = agg_it.item())->has_expired(ttl) || agg->removal());
          ++agg_it);
      if(!agg_it)
        break;
      agg_state = false;
      ++(it = agg_it);
    }

    if(!it || !agg->key.equal((cell = it.item())->key)) {
      Cell* agged = new Cell(*agg, agg_state);
      if(agg_state) {
        if(agg_ts == agg_rev) {
          agged->set_timestamp_with_rev_is_ts(agg_ts);
        } else {
          agged->set_timestamp(agg_ts);
          agged->set_revision(agg_rev);
        }
        agged->set_counter(OP_EQUAL, agg_value, type, TIMESTAMP_NULL);
      }
      finalized_cells.add_sorted(agged);

      if(!it)
        break;
      agg_it = it;
      continue;
    }

    if(!cell->has_expired(ttl) && !cell->removal()) {
      if(!agg_state) {
        agg_rev = agg->get_revision();
        agg_ts = agg->get_timestamp();
        agg_value = agg->get_counter(agg_op, agg_eq_rev);
        if(agg_eq_rev == TIMESTAMP_NULL)
          agg_eq_rev = agg_ts;
        agg_state = true;
      }
      uint8_t op;
      int64_t eq_rev;
      int64_t value = cell->get_counter(op, eq_rev);
      if(eq_rev == TIMESTAMP_NULL)
        eq_rev = cell->get_timestamp();

      if(!(agg_op & OP_EQUAL) || eq_rev > agg_eq_rev) {
        if(op & OP_EQUAL) {
          agg_op = op;
          agg_value = value;
          agg_eq_rev = eq_rev;
        } else {
          agg_value += value;
        }
        if(agg_ts < cell->get_timestamp())
          agg_ts = cell->get_timestamp();
        agg_rev = cell->get_revision();
      }
    }
    ++it;
  }

  *this = std::move(finalized_cells);
}


/* without temp "finalized_cells", remove from and keep the _container
void Mutable::_finalize_counter() {
  Iterator it = get<Iterator>();
  Iterator agg_it(it);
  Cell* agg = nullptr;
  int64_t agg_rev;
  int64_t agg_ts;
  uint8_t agg_op;
  int64_t agg_eq_rev;
  int64_t agg_value;
  Cell* cell;
  for(size_t revs = 1; ;) {

    if(!agg || agg_it == it) {
      while((agg = agg_it.item())->has_expired(ttl) || agg->removal()) {
        _remove(agg_it);
        if(!agg_it)
          return;
      }
      agg_rev = agg->get_revision();
      agg_ts = agg->get_timestamp();
      agg_value = agg->get_counter(agg_op, agg_eq_rev);
      if(agg_eq_rev == TIMESTAMP_NULL)
        agg_eq_rev = agg_ts;
      ++(it = agg_it);
    }

    if(!it || !agg->key.equal((cell = it.item())->key)) {
      ++agg_it;
      if(revs > 1) {
        if(agg_ts == agg_rev) {
          agg->set_timestamp_with_rev_is_ts(agg_ts);
        } else {
          agged->set_timestamp(agg_ts);
          agged->set_revision(agg_rev);
        }
        _remove(agg_it, --revs);
        revs = 1;
      }
      agged->set_counter(OP_EQUAL, agg_value, type, TIMESTAMP_NULL);
      if(!agg_it)
        return;
      it = agg_it;
      continue;
    }

    if(!cell->has_expired(ttl) && !cell->removal()) {
      uint8_t op;
      int64_t eq_rev;
      int64_t value = cell->get_counter(op, eq_rev);
      if(eq_rev == TIMESTAMP_NULL)
        eq_rev = cell->get_timestamp();

      if(!(agg_op & OP_EQUAL) || eq_rev > agg_eq_rev) {
        if(op & OP_EQUAL) {
          agg_op = op;
          agg_value = value;
          agg_eq_rev = eq_rev;
        } else {
          agg_value += value;
        }
        agg_ts = cell->get_timestamp();
        agg_rev = cell->get_revision();
      }
    }
    ++revs;
    ++it;
  }
}
*/




}}}
