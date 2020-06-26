
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_common_params_ColumnId_h
#define swc_db_protocol_common_params_ColumnId_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Common { namespace Params {


class ColumnId : public Serializable {
  public:

  ColumnId(cid_t cid = 0)
           : cid(cid){
  }
             
  virtual ~ColumnId() {}

  cid_t  cid;


  protected:
    
  size_t internal_encoded_length() const {
    return Serialization::encoded_length_vi64(cid);
  }
    
  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, cid);
  }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
  }


};
  

}}}}

#endif // swc_db_protocol_common_params_ColumnId_h
