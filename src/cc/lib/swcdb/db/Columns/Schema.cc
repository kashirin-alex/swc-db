/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace DB {


Schema::Schema(const Schema& other)
      : SchemaPrimitives(other),
        col_name(other.col_name), tags(other.tags) {
}


namespace {
SWC_CAN_INLINE
Schema::Tags read_tags(const uint8_t** bufp, size_t* remainp) {
  Schema::Tags tags;
  if(uint32_t sz = Serialization::decode_vi32(bufp, remainp)) {
    tags.reserve(sz);
    for(;sz;--sz)
      tags.emplace_back(Serialization::decode_bytes_string(bufp, remainp));
  }
  return tags;
}
}

Schema::Schema(const uint8_t** bufp, size_t* remainp)
  : SchemaPrimitives(bufp, remainp),
    col_name(Serialization::decode_bytes_string(bufp, remainp)),
    tags(read_tags(bufp, remainp)) {
}

Schema::~Schema() { }

uint32_t Schema::encoded_length() const noexcept {
  uint32_t sz = SchemaPrimitives::encoded_length();
  sz += Serialization::encoded_length_bytes(col_name.size());
  sz += Serialization::encoded_length_vi32(tags.size());
  for(auto& tag : tags)
    sz += Serialization::encoded_length_bytes(tag.size());
  return sz;
}

void Schema::encode(uint8_t** bufp) const {
  SchemaPrimitives::encode(bufp);
  Serialization::encode_bytes(bufp, col_name.c_str(), col_name.size());
  Serialization::encode_vi32(bufp, tags.size());
  for(auto& tag : tags)
    Serialization::encode_bytes(bufp, tag.c_str(), tag.size());
}

bool Schema::equal(const Ptr& other, bool with_rev) const noexcept {
  return  SchemaPrimitives::equal(*other.get(), with_rev) &&
          Condition::str_eq(col_name, other->col_name) &&
          tags == other->tags;
}

void Schema::display(std::ostream& out) const {
  out
    << "Schema("
    << "cid=" << std::to_string(cid)
    << " name=\"" << col_name << "\""
    << " tags=[";
  for(auto it = tags.cbegin(); it != tags.cend(); ) {
    out << '"' << *it << '"';
    if(++it == tags.cend())
      break;
    out << ',';
  }
  out << ']'
    << " seq=" << Types::to_string(col_seq)
    << " type=" << Types::to_string(col_type)

    << " revision=" << std::to_string(revision)
    << " compact=" << std::to_string(int(compact_percent))

    << " cell_versions=" << std::to_string(cell_versions)
    << " cell_ttl=" << std::to_string(cell_ttl)

    << " blk_encoding=" << Core::Encoder::to_string(blk_encoding)
    << " blk_size=" << std::to_string(blk_size)
    << " blk_cells=" << std::to_string(blk_cells)
    << " cs_replication=" << std::to_string(int(cs_replication))
    << " cs_size=" << std::to_string(cs_size)
    << " cs_max=" << std::to_string(int(cs_max))
    << " log_rollout=" << std::to_string(int(log_rollout_ratio))
    << " log_compact=" << std::to_string(int(log_compact_cointervaling))
    << " log_preload=" << std::to_string(int(log_fragment_preload))
    << ")" ;
}

void Schema::print(std::ostream& out) const {
  display(out);
}

}}
