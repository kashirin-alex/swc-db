
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_types_Encoding_h
#define swc_lib_db_types_Encoding_h


namespace SWC { namespace Types { 


enum class Encoding {
  PLAIN   = 1,
  ZLIB    = 2,
  SNAPPY  = 3
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
      return std::string("unknown");
  }
}
}}

#endif // swc_lib_db_types_Encoding_h