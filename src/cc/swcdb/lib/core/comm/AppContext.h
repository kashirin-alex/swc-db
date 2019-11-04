
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_AppContext_h
#define swc_core_comm_AppContext_h


#include <memory>
#include <iostream>

//forward declarations
namespace SWC {
class AppContext;
typedef std::shared_ptr<AppContext> AppContextPtr;
}

#include "Event.h"
#include "ConnHandler.h"

#include "IoContext.h"

namespace SWC {

class AppContext : public std::enable_shared_from_this<AppContext> {
  public:
  AppContext(){}

  virtual ~AppContext(){}

  virtual void handle(ConnHandlerPtr conn, Event::Ptr ev){
    ev->display();
    std::cerr << "AppContext(handle is Virtual!)\n";
  }

  virtual void init(const EndPoints& endpoints) {
    m_endpoints = endpoints;
  }
  
  virtual void stop() {
    std::cerr << "AppContext(stop is Virtual)!\n";
  }

  EndPoints m_endpoints;
};

}

#endif // swc_core_comm_AppContext_h