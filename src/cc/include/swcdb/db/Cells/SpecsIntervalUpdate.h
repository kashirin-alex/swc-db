/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsIntervalUpdate_h
#define swcdb_db_cells_SpecsIntervalUpdate_h


#include "swcdb/db/Cells/SpecsUpdateOp.h"


namespace SWC { namespace DB { namespace Specs {


class IntervalUpdate final {
  private:

  SWC_CAN_INLINE
  static uint8_t* alloc_value(uint32_t len) {
    return len ? new uint8_t[len] : nullptr;
  }

  SWC_CAN_INLINE
  static uint8_t* copy_value(const uint8_t* v, uint32_t len) {
    return len
      ? static_cast<uint8_t*>(memcpy(new uint8_t[len], v, len))
      : nullptr;
  }

  public:

  typedef std::unique_ptr<IntervalUpdate> Ptr;

  template<typename... ArgsT>
  SWC_CAN_INLINE
  static Ptr make(ArgsT&&... args) {
    return Ptr(new IntervalUpdate(std::forward<ArgsT>(args)...));
  }

  IntervalUpdate(int64_t a_timestamp=DB::Cells::TIMESTAMP_NULL) noexcept
                : timestamp(a_timestamp),
                  value(nullptr), vlen(0) {
  }

  IntervalUpdate(uint32_t a_vlen,
                 int64_t a_timestamp=DB::Cells::TIMESTAMP_NULL)
                : timestamp(a_timestamp),
                  value(alloc_value(a_vlen)), vlen(a_vlen) {
  }

  IntervalUpdate(uint8_t* a_value, uint32_t a_vlen,
                 int64_t a_timestamp, const UpdateOP& op, bool cp)
                : operation(op), timestamp(a_timestamp),
                  value(cp ? copy_value(a_value, a_vlen) : a_value),
                  vlen(a_vlen) {
  }

  SWC_CAN_INLINE
  IntervalUpdate(const uint8_t** bufp, size_t* remainp)
                : operation(bufp, remainp),
                  timestamp(Serialization::decode_vi64(bufp, remainp)) {
    size_t len;
    const uint8_t* ptr = Serialization::decode_bytes(bufp, remainp, &len);
    vlen = len;
    value = copy_value(ptr, vlen);
  }

  IntervalUpdate(const IntervalUpdate& other)
                : operation(other.operation),
                  timestamp(other.timestamp),
                  value(copy_value(other.value, other.vlen)),
                  vlen(other.vlen) {
  }

  IntervalUpdate(IntervalUpdate&& other) noexcept
                : operation(other.operation),
                  timestamp(other.timestamp),
                  value(other.value), vlen(other.vlen) {
    other.value = nullptr;
    other.vlen = 0;
  }

  IntervalUpdate(const DB::Cells::Cell& cell)
                : timestamp(cell.get_timestamp()) {
    StaticBuffer _v;
    cell.get_value(_v);
    value = copy_value(_v.base, _v.size);
    vlen = _v.size;
  }

  IntervalUpdate(DB::Cells::Cell&& cell)
                : timestamp(cell.get_timestamp()) {
    if(cell.have_encoder()) {
      StaticBuffer _v;
      cell.get_value(_v);
      value = _v.base;
      vlen = _v.size;
      cell.free();
    } else {
      value = cell.value;
      vlen = cell.vlen;
      cell.value = nullptr;
      cell.vlen = 0;
    }
  }

  ~IntervalUpdate() noexcept {
    _free();
  }

  SWC_CAN_INLINE
  void _free() noexcept {
    delete [] value;
  }

  SWC_CAN_INLINE
  void free() noexcept {
    if(value) { // always owns value
      _free();
      value = nullptr;
      vlen = 0;
    }
  }

  IntervalUpdate& operator=(const IntervalUpdate& other) {
    _free();
    operation = other.operation;
    timestamp = other.timestamp;
    value = copy_value(other.value, other.vlen);
    vlen = other.vlen;
    return *this;
  }

  IntervalUpdate& operator=(IntervalUpdate&& other) noexcept {
    _free();
    operation = other.operation;
    timestamp = other.timestamp;
    value = other.value;
    vlen = other.vlen;
    other.value = nullptr;
    other.vlen = 0;
    return *this;
  }

  IntervalUpdate& operator=(const DB::Cells::Cell& cell) {
    _free();
    operation = UpdateOP();
    timestamp = cell.get_timestamp();
    StaticBuffer _v;
    cell.get_value(_v);
    value = copy_value(_v.base, _v.size);
    vlen = _v.size;
    return *this;
  }

  IntervalUpdate& operator=(DB::Cells::Cell&& cell) noexcept {
    _free();
    operation = UpdateOP();
    timestamp = cell.get_timestamp();
    if(cell.have_encoder()) {
      StaticBuffer _v;
      cell.get_value(_v);
      value = _v.base;
      vlen = _v.size;
      cell.free();
    } else {
      value = cell.value;
      vlen = cell.vlen;
      cell.value = nullptr;
      cell.vlen = 0;
    }
    return *this;
  }

  SWC_CAN_INLINE
  bool equal(const IntervalUpdate& other) const noexcept {
    return operation.equal(other.operation) &&
           timestamp == other.timestamp &&
           vlen == other.vlen &&
           Condition::mem_eq(value, other.value, vlen);
  }

  void set(uint8_t* a_value, uint32_t a_vlen,
           int64_t a_timestamp=DB::Cells::TIMESTAMP_NULL, bool cp=false) {
    _free();
    operation = UpdateOP();
    timestamp = a_timestamp;
    value = cp ? copy_value(a_value, a_vlen) : a_value;
    vlen = a_vlen;
  }

  SWC_CAN_INLINE
  void set(const UpdateOP& op) noexcept {
    operation = op;
  }

  SWC_CAN_INLINE
  size_t encoded_length() const noexcept {
    return operation.encoded_length() +
           Serialization::encoded_length_vi64(timestamp) +
           Serialization::encoded_length_bytes(vlen);
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    operation.encode(bufp);
    Serialization::encode_vi64(bufp, timestamp);
    Serialization::encode_bytes(bufp, value, vlen);
  }

  void print(std::ostream& out) const {
    out << "Value(";
    if(vlen)
      display(out, true);
    out << ')';
  }

  void display(std::ostream& out, bool pretty) const {
    out << "ts=" << timestamp;
    operation.print(out << " op=(");
    out << ") len=" << vlen;
    if(vlen) {
      out << ' ' << '"';
      char hex[5];
      hex[4] = '\0';
      const uint8_t* end = value + vlen;
      for(const uint8_t* ptr = value; ptr < end; ++ptr) {
        if(*ptr == '"')
          out << '\\';
        if(!pretty || (31 < *ptr && *ptr < 127)) {
          out << *ptr;
        } else {
          sprintf(hex, "0x%X", *ptr);
          out << hex;
        }
      }
      out << '"';
    }
  }

  UpdateOP   operation;
  int64_t    timestamp;
  uint8_t*   value;
  uint32_t   vlen;

};


}}}

#endif // swcdb_db_cells_SpecsIntervalUpdate_h
