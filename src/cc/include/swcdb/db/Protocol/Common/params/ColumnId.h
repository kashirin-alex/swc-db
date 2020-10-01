
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_common_params_ColumnId_h
#define swcdb_db_protocol_common_params_ColumnId_h

#include "swcdb/core/Serializable.h"

namespace SWC { namespace Protocol { namespace Common { namespace Params {


class ColumnId : public Serializable {
  public:

  ColumnId(cid_t cid = DB::Schema::NO_CID)
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

#endif // swcdb_db_protocol_common_params_ColumnId_h
