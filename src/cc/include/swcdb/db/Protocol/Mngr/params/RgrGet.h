/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_RgrGet_h
#define swcdb_db_protocol_mngr_params_RgrGet_h


#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


class RgrGetReq final : public Serializable {
  public:

  RgrGetReq(cid_t cid=0, rid_t rid=0, bool next_range=false) noexcept
            : cid(cid), rid(rid), next_range(next_range) {
  }

  //~RgrGetReq() { }

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



class RgrGetRsp final : public Common::Params::HostEndPoints {
  public:

  RgrGetRsp(int err = Error::OK) noexcept
            : err(err), cid(0), rid(0) {
  }

  RgrGetRsp(cid_t cid, rid_t rid) noexcept
            : err(Error::OK), cid(cid), rid(rid) {
  }

  //~RgrGetRsp() { }

  int             err;
  cid_t           cid;
  rid_t           rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

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
