
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_Encoding_h
#define swc_db_types_Encoding_h

#include <string>

namespace SWC { namespace Types { 

enum class Encoding {
  DEFAULT = 0x00,
  PLAIN   = 0x01,
  ZLIB    = 0x02,
  SNAPPY  = 0x03,
  ZSTD    = 0x04
};

std::string to_string(Encoding typ);

Encoding encoding_from(const std::string& typ);

std::string repr_encoding(int typ);

int from_string_encoding(const std::string& typ);


}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Types/Encoding.cc"
#endif 

#endif // swc_db_types_Encoding_h