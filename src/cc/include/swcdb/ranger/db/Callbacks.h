/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_Callbacks_h
#define swc_ranger_db_Callbacks_h

#include <functional>
#include "swcdb/core/comm/ResponseCallback.h"

namespace SWC { namespace Ranger { namespace Callback {


typedef std::function<void(int)> RangeUnloaded_t;

typedef std::function<void(int)> ColumnDeleted_t;


class ColumnsUnloaded;
typedef std::shared_ptr<ColumnsUnloaded> ColumnsUnloadedPtr;


}}}

#endif // swc_ranger_db_Callbacks_h