/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsFlags_h
#define swcdb_db_cells_SpecsFlags_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {


class Flags {
  public:

  static const uint8_t NONE               = 0x00;
  static const uint8_t LIMIT_BY_KEYS      = 0x01;
  static const uint8_t OFFSET_BY_KEYS     = 0x04;
  static const uint8_t ONLY_KEYS          = 0x08;
  static const uint8_t ONLY_DELETES       = 0x10;

  SWC_CAN_INLINE
  explicit Flags() noexcept
                : limit(0), offset(0),
                  max_versions(0), max_buffer(0),
                  options(0), was_set(false) {
  }

  SWC_CAN_INLINE
  void copy(const Flags &other) noexcept {
    limit           = other.limit;
    offset          = other.offset;
    max_versions    = other.max_versions;
    max_buffer      = other.max_buffer;
    options         = other.options;
    was_set         = other.was_set;
  }

  //~Flags() { }

  SWC_CAN_INLINE
  bool is_only_keys() const noexcept {
    return options & ONLY_KEYS;
  }

  SWC_CAN_INLINE
  bool is_only_deletes() const noexcept {
    return options & ONLY_DELETES;
  }

  SWC_CAN_INLINE
  void set_only_keys() noexcept {
    options |= ONLY_KEYS;
  }

  SWC_CAN_INLINE
  void set_only_deletes() noexcept {
    options |= ONLY_DELETES;
  }

  SWC_CAN_INLINE
  bool equal(const Flags &other) const noexcept {
    return  limit == other.limit &&
            offset == other.offset  &&
            max_versions == other.max_versions  &&
            max_buffer == other.max_buffer  &&
            options == other.options  &&
            was_set == other.was_set
            ;
  }

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    return  Serialization::encoded_length_vi64(limit)
          + Serialization::encoded_length_vi64(offset)
          + Serialization::encoded_length_vi32(max_versions)
          + Serialization::encoded_length_vi32(max_buffer)
          + 1;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, limit);
    Serialization::encode_vi64(bufp, offset);
    Serialization::encode_vi32(bufp, max_versions);
    Serialization::encode_vi32(bufp, max_buffer);
    Serialization::encode_i8(bufp, options);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    limit = Serialization::decode_vi64(bufp, remainp);
    offset = Serialization::decode_vi64(bufp, remainp);
    max_versions = Serialization::decode_vi32(bufp, remainp);
    max_buffer = Serialization::decode_vi32(bufp, remainp);
    options = Serialization::decode_i8(bufp, remainp);
  }

  std::string to_string() const;

  void print(std::ostream& out) const;

  void display(std::ostream& out) const;

  uint64_t  limit, offset;
  uint32_t  max_versions;
  uint32_t  max_buffer;
  uint8_t   options;
  bool      was_set;

};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsFlags.cc"
#endif

#endif //swcdb_db_cells_SpecsFlags_h
