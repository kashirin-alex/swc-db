
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_types_Encoding_h
#define swc_lib_db_types_Encoding_h

#include <string>

namespace SWC { namespace Types { 


enum class Encoding {
  DEFAULT = 0x00,
  PLAIN   = 0x01,
  ZLIB    = 0x02,
  SNAPPY  = 0x03
};

const std::string to_string(Encoding typ) {
  switch(typ){
    case Encoding::PLAIN:
      return std::string("PLAIN");
    case Encoding::ZLIB:
      return std::string("ZLIB");
    case Encoding::SNAPPY:
      return std::string("SNAPPY");
    default:
      return std::string("UMKNOWN(" + std::to_string((uint8_t)typ) +")");
  }
}

const std::string repr_encoding(int typ) {
  return to_string((Encoding)typ);
}

const int from_string_encoding(std::string typ) {
  std::transform(typ.begin(), typ.end(), typ.begin(), 
        [](unsigned char c){ return std::toupper(c); }
  );

  if(typ.compare("ZLIB") == 0)
    return (int)Encoding::ZLIB;

  if(typ.compare("SNAPPY") == 0)
    return (int)Encoding::SNAPPY;
  
  return (int)Encoding::PLAIN;
}

}}

#endif // swc_lib_db_types_Encoding_h