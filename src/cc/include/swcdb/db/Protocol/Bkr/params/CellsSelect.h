/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_params_CellsSelect_h
#define swcdb_db_protocol_bkr_params_CellsSelect_h

#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Cells/SpecsScan.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Params {

class CellsSelectReq final : public Serializable {
  public:

  CellsSelectReq() noexcept { }

  CellsSelectReq(const DB::Specs::Scan& specs)
                : specs(specs) { }

  CellsSelectReq(DB::Specs::Scan&& specs) noexcept
                : specs(std::move(specs)) { }

  //~CellsSelectReq() { }

  void print(std::ostream& out) const;

  DB::Specs::Scan specs;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class CellsSelectRsp final : public Serializable {
  public:

  CellsSelectRsp(int err, cid_t cid=0, bool more=false) noexcept
                 : err(err), cid(cid), more(more) {
  }

  CellsSelectRsp(int err, StaticBuffer& buffer) noexcept
                 : err(err), cid(0), more(false), data(buffer) {
  }

  //~CellsSelectRsp() { }

  void print(std::ostream& out) const;

  int32_t       err;
  cid_t         cid;
  bool          more;
  StaticBuffer  data;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/params/CellsSelect.cc"
#endif

#endif // swcdb_db_protocol_bkr_params_CellsSelect_h
