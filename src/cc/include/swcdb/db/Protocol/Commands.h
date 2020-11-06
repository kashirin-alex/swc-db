
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_Protocol_Commands_h
#define swcdb_db_Protocol_Commands_h


namespace SWC { namespace Comm { namespace Protocol {



/**
 * @brief The SWC-DB Ranger Communications Protocol C++ namespace 'SWC::Comm::Protocol::Rgr'
 *
 * \ingroup Database
 */
namespace Rgr {

  // Ranger Protocol Commands
  enum Command : uint8_t {
    NOT_IMPLEMENTED      = 0x00,
    COLUMN_DELETE        = 0x01,
    COLUMN_COMPACT       = 0x02,
    SCHEMA_UPDATE        = 0x03,
    RANGE_IS_LOADED      = 0x04,
    RANGE_LOAD           = 0x05,
    RANGE_UNLOAD         = 0x06,
    RANGE_LOCATE         = 0x07,
    RANGE_QUERY_UPDATE   = 0x08,
    RANGE_QUERY_SELECT   = 0x09,
    REPORT               = 0x0A,
    COLUMNS_UNLOAD       = 0x0B,
    ASSIGN_ID_NEEDED     = 0x0C,
    MAX_CMD              = 0x0D
  };

}



/**
 * @brief The SWC-DB Manager Communications Protocol C++ namespace 'SWC::Comm::Protocol::Mngr'
 *
 * \ingroup Database
 */
namespace Mngr {

  // Manager Protocol Commands
  enum Command : uint8_t {
    NOT_IMPLEMENTED      = 0x00,
    MNGR_STATE           = 0x01,
    MNGR_ACTIVE          = 0x02,
    COLUMN_MNG           = 0x03,
    COLUMN_UPDATE        = 0x04,
    COLUMN_GET           = 0x05,
    COLUMN_LIST          = 0x06, 
    COLUMN_COMPACT       = 0x07,
    RGR_MNG_ID           = 0x08,
    RGR_UPDATE           = 0x09,
    RGR_GET              = 0x0A,
    RANGE_CREATE         = 0x0B,
    RANGE_UNLOADED       = 0x0C,
    RANGE_REMOVE         = 0x0D,
    REPORT               = 0x0E,
    DO_ECHO              = 0x0F,
    MAX_CMD              = 0x10
  };

}


}}}

#endif // swcdb_db_Protocol_Commands_h
