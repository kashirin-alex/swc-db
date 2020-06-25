
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_mngr_params_RgrGet_h
#define swc_db_protocol_mngr_params_RgrGet_h


#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RgrGetReq : public Serializable {
  public:

  RgrGetReq(cid_t cid=0, rid_t rid=0, bool next_range=false);

  virtual ~RgrGetReq();
  
  cid_t          cid;
  rid_t          rid;
  DB::Cell::Key  range_begin, range_end;
  bool           next_range;
  
  std::string to_string();

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RgrGetRsp : public Common::Params::HostEndPoints {
  public:

  RgrGetRsp(cid_t cid=0, rid_t rid=0);

  virtual ~RgrGetRsp();

  int             err;         
  cid_t           cid; 
  rid_t           rid; 
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

  std::string to_string() const;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/RgrGet.cc"
#endif 

#endif // swc_db_protocol_mngr_params_RgrGet_h
