
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_AppContext_h
#define swc_core_comm_AppContext_h
 
#include <asio.hpp>
#include "Resolver.h"

#include <memory>
#include <iostream>

//forward declarations
namespace SWC {
class AppContext;
typedef std::shared_ptr<AppContext> AppContextPtr;
}

#include "Event.h"
#include "ConnHandler.h"


namespace SWC {

class AppContext : public std::enable_shared_from_this<AppContext> {
  public:
  AppContext(){}

  virtual ~AppContext(){}

  virtual void handle(ConnHandlerPtr conn, EventPtr ev){
    ev->display();
    std::cerr << "AppContext handle is Virtual!\n";
  }

  virtual void init(EndPoints endpoints) {
    m_endpoints = endpoints;
  }

  EndPoints m_endpoints;
};

}

#endif // swc_core_comm_AppContext_h