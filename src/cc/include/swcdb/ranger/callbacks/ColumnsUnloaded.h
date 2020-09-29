/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_ranger_callbacks_ColumnsUnloaded_h
#define swc_ranger_callbacks_ColumnsUnloaded_h

#include "swcdb/core/comm/ResponseCallback.h"
#include "swcdb/db/Protocol/Rgr/params/ColumnsUnload.h"

namespace SWC { namespace Ranger { namespace Callback {

class ColumnsUnloaded : public Comm::ResponseCallback {
  public:

  std::vector<Column::Ptr>  cols;
  std::atomic<size_t>       unloading;

  ColumnsUnloaded(const Comm::ConnHandlerPtr& conn, 
                  const Comm::Event::Ptr& ev)
                  : Comm::ResponseCallback(conn, ev), unloading(0) {
  }

  virtual ~ColumnsUnloaded() { }

  void response(int, const RangePtr& range) { // err
    bool last = --unloading;

    if(range && !range->deleted()) {
      Mutex::scope lock(m_mutex);
      m_rsp_params.columns[range->cfg->cid].push_back(range->rid);
    }
    if(!last)
      return;

    try {
      auto cbp = Comm::CommBuf::make(m_rsp_params);
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);

    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
  }

  private:
  
  Mutex                                     m_mutex;
  Protocol::Rgr::Params::ColumnsUnloadRsp   m_rsp_params;

};


}
}}
#endif // swc_ranger_callbacks_ColumnsUnloaded_h
