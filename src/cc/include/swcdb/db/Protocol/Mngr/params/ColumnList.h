/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_ColumnList_h
#define swcdb_db_protocol_mngr_params_ColumnList_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class ColumnListReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnListReq() noexcept { }

  SWC_CAN_INLINE
  ColumnListReq(const DB::Schemas::SelectorPatterns& patterns)
               : patterns(patterns) {
  }

  SWC_CAN_INLINE
  ~ColumnListReq() { }

  DB::Schemas::SelectorPatterns patterns;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class ColumnListRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnListRsp() noexcept : expected(0) { }

  SWC_CAN_INLINE
  ~ColumnListRsp() { }

  uint64_t       expected;
  DB::SchemasVec schemas;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};

}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/ColumnList.cc"
#endif

#endif // swcdb_db_protocol_params_ColumnListRsp_h
