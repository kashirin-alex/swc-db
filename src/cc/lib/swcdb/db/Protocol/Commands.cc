/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {



namespace Rgr {


const char* Commands::to_string(uint8_t cmd) noexcept {
  switch(cmd) {
    case COLUMN_DELETE:
      return "COLUMN_DELETE";
    case COLUMN_COMPACT:
      return "COLUMN_COMPACT";
    case SCHEMA_UPDATE:
      return "SCHEMA_UPDATE";
    case RANGE_IS_LOADED:
      return "RANGE_IS_LOADED";
    case RANGE_LOAD:
      return "RANGE_LOAD";
    case RANGE_UNLOAD:
      return "RANGE_UNLOAD";
    case RANGE_LOCATE:
      return "RANGE_LOCATE";
    case RANGE_QUERY_UPDATE:
      return "RANGE_QUERY_UPDATE";
    case RANGE_QUERY_SELECT:
      return "RANGE_QUERY_SELECT";
    case REPORT:
      return "REPORT";
    case COLUMNS_UNLOAD:
      return "COLUMNS_UNLOAD";
    case ASSIGN_ID_NEEDED:
      return "ASSIGN_ID_NEEDED";
    default:
      return "NOIMPL";
  }
}


}





namespace Mngr {


const char* Commands::to_string(uint8_t cmd) noexcept {
  switch(cmd) {
    case MNGR_STATE:
      return "MNGR_STATE";
    case MNGR_ACTIVE:
      return "MNGR_ACTIVE";
    case COLUMN_MNG:
      return "COLUMN_MNG";
    case COLUMN_UPDATE:
      return "COLUMN_UPDATE";
    case COLUMN_GET:
      return "COLUMN_GET";
    case COLUMN_LIST:
      return "COLUMN_LIST";
    case COLUMN_COMPACT:
      return "COLUMN_COMPACT";
    case RGR_UPDATE:
      return "RGR_UPDATE";
    case RGR_GET:
      return "RGR_GET";
    case RANGE_CREATE:
      return "RANGE_CREATE";
    case RANGE_UNLOADED:
      return "RANGE_UNLOADED";
    case RANGE_REMOVE:
      return "RANGE_REMOVE";
    case REPORT:
      return "REPORT";
    case DO_ECHO:
      return "DO_ECHO";
    default:
      return "NOIMPL";
  }
}

}


}}}
