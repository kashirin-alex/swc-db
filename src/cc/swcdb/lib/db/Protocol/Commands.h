
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_Protocol_Commands_h
#define swc_lib_db_Protocol_Commands_h


namespace SWC { namespace Protocol {

  enum Command {

    MNGR_REQ_MNGRS_STATE,
    CLIENT_REQ_ACTIVE_MNGR,
    RS_REQ_MNG_RS_ID,
    
    MNGR_REQ_LOAD_RANGE,
    //

    MNGR_REQ_IS_RANGE_LOADED,

    RS_REQ_NOTIFY_COLUMN_ADDED,
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