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
    get_clients()->remove_mngr(endpoints);
    endpoints.clear();
    run();
  }
}



}}}}}
