/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_ranger_db_Callbacks_h
#define swc_ranger_db_Callbacks_h

#include <functional>

namespace SWC { namespace Ranger {
  
namespace Callback {

typedef std::function<void(int)> RangeUnloaded_t;
typedef std::function<void(int)> ColumnDeleted_t;

}

}}

#endif // swc_ranger_db_Callbacks_h