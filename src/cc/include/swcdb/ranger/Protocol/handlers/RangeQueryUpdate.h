/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeQueryUpdate_h
#define swcdb_ranger_Protocol_handlers_RangeQueryUpdate_h

#include "swcdb/db/Protocol/Rgr/params/RangeQueryUpdate.h"
#include "swcdb/ranger/callbacks/RangeQueryUpdate.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


void range_query_update(const ConnHandlerPtr& conn, const Event::Ptr& ev) {
  int err = Error::OK;
  Params::RangeQueryUpdateReq params;
  Ranger::RangePtr range;

  try {      
    const uint8_t *ptr = ev->data.base;
    size_t remain = ev->data.size;
    params.decode(&ptr, &remain);

    range = Env::Rgr::columns()->get_range(err, params.cid, params.rid);
      
    if(!err && (!range || !range->is_loaded()))
      err = Error::RGR_NOT_LOADED_RANGE;

    if(!err && !ev->data_ext.size)
      err = Error::INVALID_ARGUMENT;

  } catch(...) {
    const Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }

  try{
    auto cb = std::make_shared<Ranger::Callback::RangeQueryUpdate>(
      conn, ev);
    if(err) {
      cb->response(err);
      return;
    }
      
    range->add(new Ranger::Range::ReqAdd(ev->data_ext, cb));

  } catch(...) {
    SWC_LOG_CURRENT_EXCEPTION("");
  }
  
}
  

}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeQueryUpdate_h
