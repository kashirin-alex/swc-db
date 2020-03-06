
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_types_Range_h
#define swc_lib_db_types_Range_h


namespace SWC { namespace Types { 

enum Range{
  MASTER  = 1,
  META    = 2,
  DATA    = 3
};

inline const std::string to_string(Range typ) {
  switch(typ){
    case Range::MASTER:
      return std::string("MASTER");
    case Range::META:
      return std::string("META");
    case Range::DATA:
      return std::string("DATA");
    default:
      return std::string("uknown");
  }
}

}}

#endif // swc_lib_db_types_Range_h