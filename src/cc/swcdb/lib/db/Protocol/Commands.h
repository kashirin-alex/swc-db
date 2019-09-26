
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_Protocol_Commands_h
#define swc_lib_db_Protocol_Commands_h


namespace SWC { namespace Protocol {

  enum Command {
    MNGR_REQ_MNGRS_STATE        = 0x00,
    CLIENT_REQ_ACTIVE_MNGR      = 0x01,
    
    REQ_MNGR_MNG_RS_ID          = 0x02,

    REQ_RS_ASSIGN_ID_NEEDED     = 0x03,
    REQ_RS_IS_RANGE_LOADED      = 0x04,
    REQ_RS_LOAD_RANGE           = 0x05,
    REQ_RS_UNLOAD_RANGE         = 0x06,
    REQ_RS_COLUMN_DELETE        = 0x07,
    REQ_RS_SCHEMA_UPDATE        = 0x08,
    
    MNGR_UPDATE_RANGESERVERS    = 0x09,
    MNGR_UPDATE_COLUMN          = 0x0a,

    CLIENT_REQ_MNG_COLUMN       = 0x0b,
    CLIENT_REQ_GET_COLUMN       = 0x0c,
    CLIENT_REQ_RS_GET           = 0x0d,
    RANGE_LOCATE                = 0x0e,

    REQ_ECHO                    = 0xff,      
    //

    RS_RSP_SCAN,
    RS_RSP_RANGE_LOADED,

    MNGR_RSP_COLUMN_ADDED,
    MNGR_RSP_CID,
    MNGR_RSP_RS_ADDR,
    MNGR_RSP_CID_NOT_MANAGED,

    CLIENT_REQ_ADD_COLUMN,
    CLIENT_REQ_CID_NAME,
    CLIENT_REQ_RS_ADDR,
    CLIENT_REQ_SCAN,

  };

}}

#endif // swc_lib_db_Protocol_Commands_h