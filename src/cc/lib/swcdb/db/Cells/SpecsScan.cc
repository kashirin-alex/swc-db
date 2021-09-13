/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {



bool Scan::equal(const Scan &other) const noexcept {
  if(columns.size() != other.columns.size() || !other.flags.equal(flags))
    return false;

  auto it2 = other.columns.cbegin();
  for(auto it1 = columns.cbegin(); it1 != columns.cend(); ++it1, ++it2)
    if(!it1->equal(*it2))
      return false;
  return true;
}

size_t Scan::encoded_length() const noexcept {
  size_t len = Serialization::encoded_length_vi32(columns.size());
  for(auto& col : columns)
    len += col.encoded_length();
  return len + flags.encoded_length();
}

void Scan::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, columns.size());
  for(auto& col : columns)
    col.encode(bufp);
  flags.encode(bufp);
}

void Scan::decode(const uint8_t** bufp, size_t* remainp) {
  free();
  uint32_t len = Serialization::decode_vi32(bufp, remainp);
  columns.reserve(len);
  for(; len; --len)
    columns.emplace_back(bufp, remainp);
  flags.decode(bufp, remainp);
}

void Scan::print(std::ostream& out) const {
  out << "Scan(columns=[";
  for(auto& col : columns) {
    col.print(out);
    out << ", ";
  }
  flags.print(out << "], flags=");
  out << ')';
}

void Scan::display(std::ostream& out, bool pretty, std::string offset) const {
  out << offset << "SpecsScan(\n"
      << offset << " columns=[\n";
  for(auto& col : columns)
    col.display(out, pretty, "  ");
  out << offset << " ]\n";

  out << offset << " Flags(";
  flags.display(out);
  out << ")\n";
  out << offset << ")\n";
}


}}}
