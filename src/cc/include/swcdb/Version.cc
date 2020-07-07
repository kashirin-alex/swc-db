/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "Version.h"



extern inline const char SWCDB_VERSION[]   = SWCDB_SET_VERSION;

extern inline const char SWCDB_COPYRIGHT[] = SWCDB_SET_COPYRIGHT;


const char* swcdb_version() {
  return SWCDB_VERSION;
}

const char* swcdb_copyrights() {
  return SWCDB_COPYRIGHT;
}



