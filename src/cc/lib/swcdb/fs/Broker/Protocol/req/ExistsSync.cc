/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/ExistsSync.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


ExistsSync::ExistsSync(uint32_t timeout, const std::string& name) 
                      : Base(Buffers::make(Params::ExistsReq(name))),
                        state(false), name(name) {
  cbp->header.set(FUNCTION_EXISTS, timeout);
  SWC_LOGF(LOG_DEBUG, "exists path='%s'", name.c_str());
}

void ExistsSync::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_EXISTS, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::ExistsRsp params;
      params.decode(&ptr, &remain);
      state = params.exists;

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOGF(LOG_DEBUG, "exists path='%s' error='%d' state='%d'",
           name.c_str(), error, (int)state);
  
  BaseSync::acknowledge();
}



}}}}}
