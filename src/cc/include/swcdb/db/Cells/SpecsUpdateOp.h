/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsUpdateOP_h
#define swcdb_db_cells_SpecsUpdateOP_h


#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Specs {



struct UpdateOP {

  static constexpr const uint8_t REPLACE   = 0x00;
  static constexpr const uint8_t APPEND    = 0x01;
  static constexpr const uint8_t PREPEND   = 0x02;
  static constexpr const uint8_t INSERT    = 0x03;
  static constexpr const uint8_t OVERWRITE = 0x04;
  static constexpr const uint8_t SERIAL    = 0x05;

  static const char* to_string(uint8_t op) noexcept {
    switch(op) {
      case REPLACE: {
        return "REPLACE";
      }
      case APPEND: {
        return "APPEND";
      }
      case PREPEND: {
        return "PREPEND";
      }
      case INSERT: {
        return "INSERT";
      }
      case OVERWRITE: {
        return "OVERWRITE";
      }
      case SERIAL: {
        return "SERIAL";
      }
      default: {
        return "UNKNOWN";
      }
    }
  }

  SWC_CAN_INLINE
  UpdateOP(uint8_t a_op=REPLACE, uint32_t a_pos=0) noexcept
          : op(a_op), pos(a_pos) {
  }

  SWC_CAN_INLINE
  UpdateOP(const uint8_t** bufp, size_t* remainp)
          : op(Serialization::decode_i8(bufp, remainp)),
            pos(has_pos() ? Serialization::decode_vi32(bufp, remainp) : 0) {
  }

  SWC_CAN_INLINE
  UpdateOP(const UpdateOP& other) noexcept
          : op(other.op), pos(other.pos) {
  }

  SWC_CAN_INLINE
  UpdateOP(UpdateOP&& other) noexcept
          : op(other.op), pos(other.pos) {
  }

  SWC_CAN_INLINE
  UpdateOP& operator=(const UpdateOP& other) noexcept {
    op = other.op;
    pos = other.pos;
    return *this;
  }

  SWC_CAN_INLINE
  UpdateOP& operator=(UpdateOP&& other) noexcept {
    op = other.op;
    pos = other.pos;
    return *this;
  }

  SWC_CAN_INLINE
  bool equal(const UpdateOP& other) const noexcept {
    return op == other.op && pos == other.pos;
  }

  SWC_CAN_INLINE
  void set_op(uint8_t a_op) noexcept {
    op = a_op;
  }

  SWC_CAN_INLINE
  uint8_t get_op() const noexcept {
    return op;
  }

  SWC_CAN_INLINE
  void set_pos(uint32_t a_pos) noexcept {
    pos = a_pos;
  }

  SWC_CAN_INLINE
  uint32_t get_pos() const noexcept {
    return pos;
  }

  SWC_CAN_INLINE
  bool has_pos() const noexcept {
    return op == INSERT || op == OVERWRITE;
  }

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    return 1 + (has_pos() ? Serialization::encoded_length_vi32(pos) : 0);
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_i8(bufp, op);
    if(has_pos())
      Serialization::encode_vi32(bufp, pos);
  }

  SWC_CAN_INLINE
  void print(std::ostream& out) const {
    out << to_string(get_op()) <<'[' << int(get_op()) << ']';
    if(has_pos())
      out << " pos=" << pos;
  }

  uint8_t    op;
  uint32_t   pos;

};



}}}

#endif // swcdb_db_cells_SpecsUpdateOP_h
