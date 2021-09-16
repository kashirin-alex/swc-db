/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_ColumnGet_h
#define swcdb_db_protocol_mngr_params_ColumnGet_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class ColumnGetReq final : public Serializable {
  public:

  enum Flag : uint8_t {
    SCHEMA_BY_ID    = 0x0,
    SCHEMA_BY_NAME  = 0x1,
    ID_BY_NAME      = 0x2
  };

  SWC_CAN_INLINE
  ColumnGetReq() noexcept { }

  SWC_CAN_INLINE
  ColumnGetReq(Flag a_flag, const std::string& a_name)
               : flag(a_flag), name(a_name) {
  }

  SWC_CAN_INLINE
  ColumnGetReq(Flag a_flag, cid_t a_cid) noexcept
              : flag(a_flag), cid(a_cid) {
  }

  SWC_CAN_INLINE
  ~ColumnGetReq() { }

  Flag        flag;
  std::string name;
  cid_t       cid {};

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class ColumnGetRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  ColumnGetRsp() noexcept { }

  SWC_CAN_INLINE
  ColumnGetRsp(ColumnGetReq::Flag a_flag, const DB::Schema::Ptr& a_schema)
               noexcept : flag(a_flag), schema(a_schema) {
  }

  SWC_CAN_INLINE
  ~ColumnGetRsp() { }

  ColumnGetReq::Flag  flag;
  DB::Schema::Ptr     schema;
  cid_t               cid {};

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};

}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/ColumnGet.cc"
#endif

#endif // swcdb_db_protocol_params_ColumnGetRsp_h
