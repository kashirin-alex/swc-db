/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Mngr/req/RgrGet_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Req {



void RgrGet_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    Params::RgrGetRsp rsp(Error::CLIENT_STOPPING);
    callback(rsp);
  } else if(!valid()) {
    Params::RgrGetRsp rsp(Error::CANCELLED);
    callback(rsp);
  } else {
    get_clients()->remove_mngr(endpoints);
    endpoints.clear();
    run();
  }
}



}}}}}
