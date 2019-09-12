
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_Callbacks_h
#define swc_lib_db_protocol_req_Callbacks_h

namespace SWC { namespace Protocol { namespace Req {

namespace Callback {

  typedef std::function<void(bool)> RangeIsLoaded_t;

}

}}}

#endif // swc_lib_db_protocol_req_Callbacks_h
