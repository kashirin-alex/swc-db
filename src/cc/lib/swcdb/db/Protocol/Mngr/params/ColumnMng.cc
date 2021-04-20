/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/params/ColumnMng.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {


size_t ColumnMng::internal_encoded_length() const {
  return 1 + schema->encoded_length();
}

void ColumnMng::internal_encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, function);
  schema->encode(bufp);
}

void ColumnMng::internal_decode(const uint8_t** bufp, size_t* remainp) {
  function = Function(Serialization::decode_i8(bufp, remainp));
  schema = std::make_shared<DB::Schema>(bufp, remainp);
}


}}}}}
