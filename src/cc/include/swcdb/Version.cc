/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/Version.h"



static const char SWCDB_VERSION[]   = SWCDB_SET_VERSION;

static const char SWCDB_COPYRIGHT[] = SWCDB_SET_COPYRIGHT;


const char* swcdb_version() noexcept {
  return SWCDB_VERSION;
}

const char* swcdb_copyrights() noexcept {
  return SWCDB_COPYRIGHT;
}



