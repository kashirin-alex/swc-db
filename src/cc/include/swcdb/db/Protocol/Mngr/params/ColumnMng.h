/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_ColumnMng_h
#define swcdb_db_protocol_mngr_params_ColumnMng_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class ColumnMng : public Serializable {
  public:

  enum Function : uint8_t { // corelation-sequence required
    INTERNAL_LOAD_ALL     = 0,

    INTERNAL_LOAD         = 1,
    INTERNAL_ACK_LOAD     = 2,

    CREATE                = 3,
    INTERNAL_ACK_CREATE   = 4,

    REMOVE                = 5,
    INTERNAL_ACK_DELETE   = 6,

    MODIFY                = 7,
    INTERNAL_ACK_MODIFY   = 8,

    INTERNAL_EXPECT       = 9,
  };

  SWC_CAN_INLINE
  ColumnMng() noexcept : function(), schema() { }

  SWC_CAN_INLINE
  ColumnMng(Function a_function, const DB::Schema::Ptr& a_schema) noexcept
            : function(a_function), schema(a_schema) {
  }

  ~ColumnMng() noexcept { }

  Function        function;
  DB::Schema::Ptr schema;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

  };


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.cc"
#endif

#endif // swcdb_db_protocol_mngr_params_ColumnMng_h
