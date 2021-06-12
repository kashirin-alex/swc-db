/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {



void ColumnMng_Base::handle_no_conn() {
  if(get_clients()->stopping()) {
    callback(Error::CLIENT_STOPPING);
  } else if(!valid()) {
    callback(Error::CANCELLED);
  } else if(_bkr_idx.turn_around(get_clients()->brokers)) {
    request_again();
  } else {
    run();
  }
}

}}}}}
