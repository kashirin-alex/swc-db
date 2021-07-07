/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/Result.h"


namespace SWC { namespace DB { namespace Cells {



void Result::write(DynamicBuffer& cells) const {
  cells.ensure(bytes);
  for(auto cell : *this) {
    if(!cell->has_expired(ttl))
      cell->write(cells);
  }
}

void Result::write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                            Interval& intval, uint32_t threshold,
                            uint32_t max_cells) {
  if(empty())
    return;

  cells.ensure(bytes < threshold? bytes: threshold);
  Cell* first = nullptr;
  Cell* last = nullptr;
  auto it = cbegin();
  for(Cell* cell; it != cend() && (
                  (!threshold || threshold > cells.fill()) &&
                  (!max_cells || max_cells > cell_count) ); ++it) {
    if((cell = *it)->has_expired(ttl))
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

  if(it == cend()) {
    free();
    return;
  }
  auto it_end = it;
  do {
    --it;
    bytes -= (*it)->encoded_length();
    delete *it;
  } while(it != cbegin());
  erase(cbegin(), it_end);
}


void Result::print(std::ostream& out, Types::Column col_type,
                   bool with_cells) const {
  out << "CellsResult(size=" << size()
      << " bytes=" << bytes << " ttl=" << ttl;
  if(with_cells) {
    out << " cells=[";
    for(auto cell : *this)
      cell->print(out << '\n', col_type);
    out << "\n]";
  }
  out << ')';
}


}}}
