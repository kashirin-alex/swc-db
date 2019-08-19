
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

}}

#endif // swc_lib_db_types_Encoding_h