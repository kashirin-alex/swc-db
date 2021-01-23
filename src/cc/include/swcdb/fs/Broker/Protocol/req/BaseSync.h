/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_req_BaseSync_h
#define swcdb_fs_Broker_Protocol_req_BaseSync_h


#include <future>


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


class BaseSync : private std::promise<void> {
  public:

  void wait() {
    get_future().wait();
  }

  protected:

  void acknowledge() {
    set_value();
  }

};



}}}}}


#endif // swcdb_fs_Broker_Protocol_req_BaseSync_h
