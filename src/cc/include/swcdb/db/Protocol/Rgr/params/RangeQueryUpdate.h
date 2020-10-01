
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeQueryUpdate_h
#define swcdb_db_protocol_rgr_params_RangeQueryUpdate_h

#include "swcdb/core/Error.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeQueryUpdateReq : public Serializable {
  public:

  RangeQueryUpdateReq();

  RangeQueryUpdateReq(cid_t cid, rid_t rid);

  virtual ~RangeQueryUpdateReq();

  void print(std::ostream& out) const;

  cid_t           cid;
  rid_t           rid;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RangeQueryUpdateRsp  : public Serializable {
  public:

  RangeQueryUpdateRsp(int err = Error::OK);

  RangeQueryUpdateRsp(int err, const DB::Cell::Key& range_prev_end,
                               const DB::Cell::Key& range_end);

  virtual ~RangeQueryUpdateRsp();

  void print(std::ostream& out) const;

  int32_t       err;
  DB::Cell::Key range_prev_end;
  DB::Cell::Key range_end;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.cc"
#endif 

#endif // swcdb_db_protocol_rgr_params_RangeQueryUpdate_h
