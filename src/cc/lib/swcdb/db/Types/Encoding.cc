
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */



#include "swcdb/db/Types/Encoding.h"
#include <cstring>


namespace SWC { namespace Types { 


std::string to_string(Encoding typ) {
  switch(typ){
    case Encoding::DEFAULT:
      return std::string("DEFAULT");
    case Encoding::PLAIN:
      return std::string("PLAIN");
    case Encoding::ZLIB:
      return std::string("ZLIB");
    case Encoding::SNAPPY:
      return std::string("SNAPPY");
    case Encoding::ZSTD:
      return std::string("ZSTD");
    default:
      return std::string("UNKNOWN(" + std::to_string((uint8_t)typ) +")");
  }
}

Encoding encoding_from(const std::string& typ) {

  if(strncasecmp(typ.data(), "PLAIN", typ.length()) == 0 || 
     typ.compare("1") == 0)
    return Encoding::PLAIN;

  if(strncasecmp(typ.data(), "ZLIB", typ.length()) == 0 ||
     typ.compare("2") == 0)
    return Encoding::ZLIB;

  if(strncasecmp(typ.data(), "SNAPPY", typ.length()) == 0 ||
     typ.compare("3") == 0)
    return Encoding::SNAPPY;
  
  if(strncasecmp(typ.data(), "ZSTD", typ.length()) == 0 ||
     typ.compare("4") == 0)
    return Encoding::ZSTD;

  return Encoding::DEFAULT;
}

std::string repr_encoding(int typ) {
  return to_string((Encoding)typ);
}

int from_string_encoding(const std::string& typ) {
  return (int)encoding_from(typ);
}


}}
