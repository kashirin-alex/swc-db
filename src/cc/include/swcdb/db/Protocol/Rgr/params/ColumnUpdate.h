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
  ColumnUpdate(const DB::Schema::Ptr& schema) noexcept : schema(schema) { }

  //~ColumnUpdate() { }

  DB::Schema::Ptr schema;

  private:

  size_t internal_encoded_length() const override {
    return schema->encoded_length();
  }

  void internal_encode(uint8_t** bufp) const override {
    schema->encode(bufp);
  }

  void internal_decode(const uint8_t** bufp, size_t* remainp) override {
    schema = std::make_shared<DB::Schema>(bufp, remainp);
  }

};


}}}}}

#endif // swcdb_db_protocol_rgr_params_ColumnUpdate_h
