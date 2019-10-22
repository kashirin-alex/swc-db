
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_types_Encoding_h
#define swc_lib_db_types_Encoding_h


namespace SWC { namespace Types { 


enum class Encoding {
  PLAIN   = 0x01,
  ZLIB    = 0x02,
  SNAPPY  = 0x03
};

const std::string to_string(Encoding typ) {
  switch(typ){
    case Encoding::PLAIN:
      return std::string("plain");
    case Encoding::ZLIB:
      return std::string("zlib");
    case Encoding::SNAPPY:
      return std::string("snappy");
    default:
      return std::string("unknown(" + std::to_string((uint8_t)typ) +")");
  }
}
}}

#endif // swc_lib_db_types_Encoding_h