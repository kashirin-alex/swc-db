/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {

Scan::Scan(uint32_t reserve): columns(0) {
  columns.reserve(reserve);
}

Scan::Scan(Columns& columns): columns(columns) {}

Scan::Scan(const uint8_t** bufp, size_t* remainp) {
  decode(bufp, remainp);
}

void Scan::copy(const Scan &other) {
  free();
  columns.resize(other.columns.size());
  int i = 0;
  for(const auto& col : other.columns)
    columns[i++] = Column::make_ptr(col);
  flags.copy(other.flags);
}

Scan::~Scan() {
  free();
}

void Scan::free() {
  columns.clear();
}

bool Scan::equal(const Scan &other) const noexcept {
  if(columns.size() != other.columns.size() || !other.flags.equal(flags))
    return false;

  auto it2=other.columns.begin();
  for(auto it1=columns.begin(); it1 < columns.end(); ++it1, ++it2)
    if(!(*it1)->equal(*(*it2)))
      return false;
  return true;
}

size_t Scan::encoded_length() const noexcept {
  size_t len = Serialization::encoded_length_vi32(columns.size());
  for(auto& col : columns)
    len += col->encoded_length();
  return len + flags.encoded_length();
}

void Scan::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, columns.size());
  for(auto& col : columns)
    col->encode(bufp);
  flags.encode(bufp);
}

void Scan::decode(const uint8_t** bufp, size_t* remainp) {
  free();
  columns.resize(Serialization::decode_vi32(bufp, remainp));
  for(auto& col : columns)
    col = Column::make_ptr(bufp, remainp);
  flags.decode(bufp, remainp);
}

std::string Scan::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Scan::print(std::ostream& out) const {
  out << "Scan(columns=[";
  for(auto& col : columns) {
    col->print(out);
    out << ", ";
  }
  flags.print(out << "], flags=");
  out << ')';
}

void Scan::display(std::ostream& out, bool pretty, std::string offset) const {
  out << offset << "SpecsScan(\n"
      << offset << " columns=[\n";
  for(auto& col : columns)
    col->display(out, pretty, "  ");
  out << offset << " ]\n";

  out << offset << " Flags(";
  flags.display(out);
  out << ")\n";
  out << offset << ")\n";
}


}}}