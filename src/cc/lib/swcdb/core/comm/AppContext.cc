
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/comm/AppContext.h"
#include <iostream>

namespace SWC {

AppContext::AppContext() { }

AppContext::~AppContext(){}

void AppContext::handle(ConnHandlerPtr conn, const Event::Ptr& ev) {
  ev->display();
  std::cerr << "AppContext(handle is Virtual!)\n";
}

void AppContext::init(const EndPoints& endpoints) {
  m_endpoints = endpoints;
}
  
void AppContext::stop() {
  std::cerr << "AppContext(stop is Virtual)!\n";
}


}
