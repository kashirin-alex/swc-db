
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_mngr_params_RgrGet_h
#define swc_db_protocol_mngr_params_RgrGet_h


#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class RgrGetReq : public Serializable {
  public:

  RgrGetReq(int64_t cid=0, int64_t rid=0, bool next_range=false);

  virtual ~RgrGetReq();
  
  int64_t        cid;
  int64_t        rid;
  DB::Cell::Key  range_begin, range_end;
  bool           next_range;
  
  const std::string to_string();

  private:

  uint8_t encoding_version() const;

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};



class RgrGetRsp : public Common::Params::HostEndPoints {
  public:

  RgrGetRsp(int64_t cid=0, int64_t rid=0);

  virtual ~RgrGetRsp();

  int             err;         
  int64_t         cid; 
  int64_t         rid; 
  DB::Cell::Key   range_end;
  DB::Cell::Key   range_begin;

  const std::string to_string() const;

  private:

  uint8_t encoding_version() const;
    
  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};
  

}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/RgrGet.cc"
#endif 

#endif // swc_db_protocol_mngr_params_RgrGet_h
