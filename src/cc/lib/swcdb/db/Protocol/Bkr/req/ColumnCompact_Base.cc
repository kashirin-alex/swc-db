/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Bkr/req/ColumnCompact_Base.h"
#include "swcdb/db/Protocol/Commands.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {



void ColumnCompact_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    callback(Mngr::Params::ColumnCompactRsp(Error::CLIENT_STOPPING));
  } else if(!valid()) {
    callback(Mngr::Params::ColumnCompactRsp(Error::CANCELLED));
  } else if(_bkr_idx.turn_around(get_clients()->brokers)) {
    request_again();
  } else {
    run();
  }
}



}}}}}
