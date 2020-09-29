/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Readdir.h"


namespace SWC { namespace FsBroker { namespace Protocol { namespace Req {


Readdir::Readdir(uint32_t timeout, const std::string& name, 
                 const FS::Callback::ReaddirCb_t& cb) 
                : name(name), cb(cb) {
  SWC_LOGF(LOG_DEBUG, "readdir path='%s'", name.c_str());

  cbp = Comm::Buffers::make(Params::ReaddirReq(name));
  cbp->header.set(Cmd::FUNCTION_READDIR, timeout);
}

std::promise<void> Readdir::promise() {
  std::promise<void>  r_promise;
  cb = [await=&r_promise](int, const FS::DirentList&){ await->set_value(); };
  return r_promise;
}

void Readdir::handle(Comm::ConnHandlerPtr, const Comm::Event::Ptr& ev) { 

  const uint8_t *ptr;
  size_t remain;

  if(!Base::is_rsp(ev, Cmd::FUNCTION_READDIR, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::ReaddirRsp params;
      params.decode(&ptr, &remain);
      params.get_listing(listing);

    } catch(...) {
      const Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }

  SWC_LOGF(LOG_DEBUG, "readdir path='%s' error='%d' sz='%lu'",
             name.c_str(), error, listing.size());
  
  cb(error, listing);
}



}}}}
