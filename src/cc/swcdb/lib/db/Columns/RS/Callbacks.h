/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_lib_db_Columns_Callbacks_h
#define swc_lib_db_Columns_Callbacks_h

#include <functional>

namespace SWC { namespace server { namespace RS {
  
namespace Callback {

typedef std::function<void(bool)> RangeUnloaded_t;

}

}}}

#endif // swc_lib_db_Columns_Callbacks_h