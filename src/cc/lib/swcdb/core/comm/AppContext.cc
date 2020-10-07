
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Logger.h"
#include "swcdb/core/comm/AppContext.h"
#include <iostream>

namespace SWC { namespace Comm {

AppContext::AppContext(Config::Property::V_GENUM::Ptr cfg_encoder) 
                      : cfg_encoder(cfg_encoder) {
}

AppContext::~AppContext(){}

void AppContext::handle(ConnHandlerPtr, const Event::Ptr& ev) {
  SWC_LOG_OUT(LOG_WARN,
    ev->print(SWC_LOG_OSTREAM << "AppContext(handle is Virtual!)\n");
  );
}

void AppContext::init(const EndPoints& endpoints) {
  m_endpoints = endpoints;
}
  
void AppContext::stop() {
  SWC_LOG(LOG_WARN, "AppContext(stop is Virtual)!");
}


}}
