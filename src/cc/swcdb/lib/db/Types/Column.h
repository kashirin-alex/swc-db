
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_types_Column_h
#define swc_lib_db_types_Column_h


namespace SWC { namespace Types { 


enum class Column {
  PLAIN       = 1,
  COUNTER_I64 = 2
};

}}

#endif // swc_lib_db_types_Column_h