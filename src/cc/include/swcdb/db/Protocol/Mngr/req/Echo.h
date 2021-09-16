/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_req_Echo_h
#define swcdb_db_protocol_mngr_req_Echo_h

#include "swcdb/db/Protocol/Commands.h"

namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {


class Echo : public DispatchHandler {
  public:
  typedef std::function<void(bool)> EchoCb_t;

  Echo(const ConnHandlerPtr& a_conn, EchoCb_t&& a_cb, size_t buf_sz=0)
       : conn(a_conn), cb(std::move(a_cb)) {

    if(!buf_sz) {
      cbp = Buffers::make();

    } else {
      StaticBuffer sndbuf(buf_sz);
      uint8_t* ptr = sndbuf.base;
      const uint8_t* end = sndbuf.base + buf_sz-4;

      uint8_t i=0;
      while(ptr < end) {
        if(i == 127)
          i = 0;
        else
          ++i;
        *ptr++ = i;
      }

      cbp = Buffers::make(sndbuf);
    }

    cbp->header.set(DO_ECHO, 60000);
  }

  virtual ~Echo() { }

  bool run() override {
    return conn->send_request(cbp, shared_from_this());
  }

  void handle_no_conn() override {
    cb(false);
  }

  void handle(ConnHandlerPtr, const Event::Ptr& ev) override {
    //SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());

    cb(ev->header.command == DO_ECHO && !ev->error);
  }

  private:
  ConnHandlerPtr        conn;
  EchoCb_t              cb;
  Buffers::Ptr          cbp;
};

}}}}}

#endif // swcdb_db_protocol_mngr_req_Echo_h
