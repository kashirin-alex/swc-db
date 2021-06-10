/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Base.h"
#include "swcdb/db/Protocol/Mngr/req/MngrActive.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



void ColumnMng_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    callback(Error::CLIENT_STOPPING);
  } else if(!valid()) {
    callback(Error::CANCELLED);
  } else {
    clear_endpoints();
    run();
  }
}

bool ColumnMng_Base::run() {
  if(endpoints.empty()) {
    get_clients()->get_mngr(DB::Types::MngrRole::SCHEMAS, endpoints);
    if(endpoints.empty()) {
      if(get_clients()->stopping()) {
        callback(Error::CLIENT_STOPPING);
      } else if(!valid()) {
        callback(Error::CANCELLED);
      } else {
        MngrActive::make(
          get_clients(), DB::Types::MngrRole::SCHEMAS, shared_from_this()
        )->run();
      }
      return false;
    }
  }
  get_clients()->get_mngr_queue(endpoints)->put(req());
  return true;
}

void ColumnMng_Base::clear_endpoints() {
  get_clients()->remove_mngr(endpoints);
  endpoints.clear();
}


}}}}}
