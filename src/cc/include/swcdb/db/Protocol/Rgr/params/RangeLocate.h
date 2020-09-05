
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_rgr_params_RangeLocate_h
#define swc_db_protocol_rgr_params_RangeLocate_h


#include "swcdb/core/Error.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Types/Identifiers.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class RangeLocateReq : public Serializable {
  public:

  static const uint8_t NEXT_RANGE  = 0x01;
  static const uint8_t COMMIT      = 0x02;

  RangeLocateReq(cid_t cid=0, rid_t rid=0);
  
  virtual ~RangeLocateReq();

  void print(std::ostream& out) const;

  cid_t          cid;
  rid_t          rid;
  DB::Cell::Key  range_begin, range_end, range_offset;
  uint8_t        flags;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RangeLocateRsp  : public Serializable {
  public:

  RangeLocateRsp(int err = Error::OK);

  virtual ~RangeLocateRsp();

  void print(std::ostream& out) const;

  int             err;         
  cid_t           cid; 
  rid_t           rid;
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeLocate.cc"
#endif 

#endif // swc_db_protocol_rgr_params_RangeLocate_h
