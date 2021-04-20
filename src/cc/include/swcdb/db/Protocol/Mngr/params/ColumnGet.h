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

  ColumnGetReq() noexcept { }

  ColumnGetReq(Flag flag, const std::string& name);

  ColumnGetReq(Flag flag, cid_t cid)
               noexcept : flag(flag), cid(cid) {
  }

  //~ColumnGetReq() { }

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

  ColumnGetRsp() noexcept { }

  ColumnGetRsp(ColumnGetReq::Flag flag, const DB::Schema::Ptr& schema)
               noexcept : flag(flag), schema(schema) {
  }

  //~ColumnGetRsp() { }

  ColumnGetReq::Flag  flag;
  DB::Schema::Ptr     schema = nullptr;
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
