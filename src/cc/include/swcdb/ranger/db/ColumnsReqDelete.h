/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_ColumnsReqDelete_h
#define swcdb_ranger_db_ColumnsReqDelete_h


namespace SWC { namespace Ranger {

struct ColumnsReqDelete {
  ColumnsReqDelete(const int64_t cid, const 
                   ConnHandlerPtr& conn, const Event::Ptr& ev) 
                  : cid(cid), conn(conn), ev(ev) {
  }
  ~ColumnsReqDelete() { }

  void cb(int err) {
    if(err)
      conn->send_error(err, "", ev);
    else
      conn->response_ok(ev);
  }

  const int64_t   cid;
  ConnHandlerPtr  conn;
  Event::Ptr      ev;
};

}} // namespace SWC::Ranger

#endif