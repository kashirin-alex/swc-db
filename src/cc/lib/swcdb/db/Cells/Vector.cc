/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/db/Cells/Vector.h"


namespace SWC { namespace DB { namespace Cells {


Vector::Vector(const uint32_t max_revs, const uint64_t ttl_ns, 
               const Types::Column type)
              : bytes(0), type(type), max_revs(max_revs), 
                ttl(ttl_ns) {
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
  clear();
  bytes = 0;
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

void Vector::take(Vector& other) {
  bytes += other.bytes;
  insert(end(), other.begin(), other.end());
  
  other.clear();
  other.bytes = 0;
}

void Vector::add(const Cell& cell, bool no_value) {
  Cell* adding;
  push_back(adding = new Cell(cell, no_value));
  bytes += adding->encoded_length();
}

const size_t Vector::add(const uint8_t* ptr, size_t remain) {
  size_t count = 0;
  bytes += remain;
  while(remain) {
    push_back(new Cell(&ptr, &remain, true));
    ++count;
  }
  return count;
}

const size_t Vector::size_bytes() const {
  return bytes;
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

void Vector::write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                            Interval& intval, uint32_t threshold, 
                            uint32_t max_cells) {
  if(empty())
    return;
  cells.ensure(bytes < threshold? bytes: threshold);
  Cell* first = nullptr;
  Cell* last = nullptr;
  size_t count = 0;
  auto it = begin();
  for(Cell* cell; it < end() && ((!threshold || threshold > cells.fill()) && 
                  (!max_cells || max_cells > count) ); ++it) {
    
    if((cell = *it)->has_expired(ttl))
      continue;

    cell->write(cells);
    intval.expand(cell->timestamp);
    cell->key.align(intval.aligned_min, intval.aligned_max);
    if(!first)
      first = cell;
    else 
      last = cell;
    ++count;
  }
  if(first) {
    intval.expand_begin(*first);
    intval.expand_end(*(last ? last : first));
  }
  cell_count += count;
  
  if(it == end()) {
    free();
    return;
  }
  auto it_end = it;
  do {
    --it;
    bytes -= (*it)->encoded_length();
    delete *it; 
  } while(it > begin());
  erase(begin(), it_end);
}

void Vector::write(DynamicBuffer& cells) const {
  cells.ensure(bytes);
  for(auto cell : *this) {
    if(cell->has_expired(ttl))
      continue;
    cell->write(cells);
  }
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


}}}