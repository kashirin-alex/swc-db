
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_manager_MngrRangeServers_h
#define swc_lib_manager_MngrRangeServers_h

#include <memory>

namespace SWC { namespace server { namespace Mngr {

class MngrRangeServers {

  public:
  MngrRangeServers(){}
  virtual ~MngrRangeServers(){}


};
typedef std::shared_ptr<MngrRangeServers> MngrRangeServersPtr;

}}}

#endif // swc_lib_manager_MngrRangeServers_h