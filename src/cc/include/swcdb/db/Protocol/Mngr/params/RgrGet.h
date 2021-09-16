/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_RgrGet_h
#define swcdb_db_protocol_mngr_params_RgrGet_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class RgrGetReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  RgrGetReq(cid_t a_cid=0, rid_t a_rid=0, bool a_next_range=false) noexcept
            : cid(a_cid), rid(a_rid), next_range(a_next_range) {
  }

  SWC_CAN_INLINE
  ~RgrGetReq() { }

  void print(std::ostream& out) const;

  cid_t          cid;
  rid_t          rid;
  DB::Cell::Key  range_begin, range_end;
  bool           next_range;
  //int            had_err;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RgrGetRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  RgrGetRsp(int a_err = Error::OK) noexcept
            : err(a_err), cid(0), rid(0), revision(0) {
  }

  SWC_CAN_INLINE
  RgrGetRsp(cid_t a_cid, rid_t a_rid) noexcept
            : err(Error::OK), cid(a_cid), rid(a_rid), revision(0) {
  }

  RgrGetRsp(int err, const uint8_t* ptr, size_t remain) noexcept;

  SWC_CAN_INLINE
  ~RgrGetRsp() { }

  EndPoints       endpoints;
  int             err;
  cid_t           cid;
  rid_t           rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;
  int64_t         revision;

  void print(std::ostream& out) const;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/RgrGet.cc"
#endif

#endif // swcdb_db_protocol_mngr_params_RgrGet_h
