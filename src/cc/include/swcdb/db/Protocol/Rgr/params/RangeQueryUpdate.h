
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_rgr_params_RangeQueryUpdate_h
#define swc_db_protocol_rgr_params_RangeQueryUpdate_h

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

  cid_t           cid;
  rid_t           rid;
  
  std::string to_string() const;

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

  int32_t       err;
  DB::Cell::Key range_prev_end;
  DB::Cell::Key range_end;

  std::string to_string() const;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.cc"
#endif 

#endif // swc_db_protocol_rgr_params_RangeQueryUpdate_h
