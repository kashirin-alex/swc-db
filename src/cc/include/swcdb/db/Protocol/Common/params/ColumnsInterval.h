
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_common_params_ColumnsInterval_h
#define swcdb_db_protocol_common_params_ColumnsInterval_h

#include "swcdb/core/comm/Serializable.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Common { namespace Params {


class ColumnsInterval : public Serializable {
  public:

  ColumnsInterval(cid_t cid_begin = DB::Schema::NO_CID, 
                  cid_t cid_end = DB::Schema::NO_CID)
                 : cid_begin(cid_begin), cid_end(cid_end) {
  }
             
  virtual ~ColumnsInterval() { }
  
  cid_t cid_begin;
  cid_t cid_end;

  protected:
    
  size_t internal_encoded_length() const {
    return Serialization::encoded_length_vi64(cid_begin)
         + Serialization::encoded_length_vi64(cid_end);
  }
    
  void internal_encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, cid_begin);
    Serialization::encode_vi64(bufp, cid_end);
  }
    
  void internal_decode(const uint8_t** bufp, size_t* remainp) {
    cid_begin = Serialization::decode_vi64(bufp, remainp);
    cid_end = Serialization::decode_vi64(bufp, remainp);
  }


};
  

}}}}}

#endif // swcdb_db_protocol_common_params_ColumnsInterval_h
