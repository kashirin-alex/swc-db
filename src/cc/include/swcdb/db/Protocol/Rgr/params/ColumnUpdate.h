/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_ColumnUpdate_h
#define swcdb_db_protocol_rgr_params_ColumnUpdate_h

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {

class ColumnUpdate final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnUpdate() noexcept { }

  SWC_CAN_INLINE
  ColumnUpdate(const DB::Schema::Ptr& schema) noexcept
              : schema_primitives(*schema.get()) {
  }

  //~ColumnUpdate() { }

  DB::SchemaPrimitives schema_primitives;

  private:

  size_t internal_encoded_length() const override {
    return schema_primitives.encoded_length();
  }

  void internal_encode(uint8_t** bufp) const override {
    schema_primitives.encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    schema_primitives.decode(bufp, remainp);
  }

};


}}}}}

#endif // swcdb_db_protocol_rgr_params_ColumnUpdate_h
