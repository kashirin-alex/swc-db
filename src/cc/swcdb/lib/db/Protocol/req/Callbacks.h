
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_lib_db_protocol_req_Callbacks_h
#define swc_lib_db_protocol_req_Callbacks_h

namespace SWC { namespace Protocol { namespace Req {

namespace Callback {

  typedef std::function<void(bool)> LoadRange_t;
  typedef std::function<void(bool)> IsRangeLoaded_t;
  typedef std::function<void(bool)> RsIdReqNeeded_t;
  typedef std::function<void(bool)> RsColumnDelete_t;

}

}}}

#endif // swc_lib_db_protocol_req_Callbacks_h
