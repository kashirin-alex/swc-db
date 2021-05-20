/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/client/Query/Update/Handlers/Base.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/client/Query/Update/BrokerCommitter.h"


namespace SWC { namespace client { namespace Query { namespace Update {
namespace Handlers {


void Base::default_executor(Column* colp) {
  switch(executor) {
    case Clients::DEFAULT:
      return Update::Committer::execute(shared_from_this(), colp);
    case Clients::BROKER:
      return Update::BrokerCommitter::execute(shared_from_this(), colp);
    default:
      break;
  }
  SWC_THROWF(Error::INVALID_ARGUMENT, "Bad Executor=%d", int(executor));
}


}}}}}


