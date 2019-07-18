
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_RangeServers_h
#define swc_lib_manager_RangeServers_h

#include <memory>

namespace SWC { namespace server { namespace Mngr {

class RangeServers {

  public:
  RangeServers(){}
  virtual ~RangeServers(){}


};
typedef std::shared_ptr<RangeServers> RangeServersPtr;

}}}

#endif // swc_lib_manager_RangeServers_h