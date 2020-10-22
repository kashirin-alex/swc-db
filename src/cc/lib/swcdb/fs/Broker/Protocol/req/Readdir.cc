/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Readdir.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


Readdir::Readdir(uint32_t timeout, const std::string& name, 
                 const FS::Callback::ReaddirCb_t& cb) 
                : Base(Buffers::make(Params::ReaddirReq(name))),
                  name(name), cb(cb) {
  cbp->header.set(FUNCTION_READDIR, timeout);
  SWC_LOGF(LOG_DEBUG, "readdir path='%s'", name.c_str());
}

void Readdir::handle(ConnHandlerPtr, const Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, FUNCTION_READDIR, &ptr, &remain))
    return;

  
  FS::DirentList listing;
  if(!error) {
    try {
      Params::ReaddirRsp params(listing);
      params.decode(&ptr, &remain);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOGF(LOG_DEBUG, "readdir path='%s' error='%d' sz='%lu'",
             name.c_str(), error, listing.size());
  
  cb(error, listing);
}



}}}}}
