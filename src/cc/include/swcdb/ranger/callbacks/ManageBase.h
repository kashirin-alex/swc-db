/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_ManageBase_h
#define swcdb_ranger_callbacks_ManageBase_h


#include "swcdb/core/comm/ResponseCallback.h"


namespace SWC { namespace Ranger {


//! The SWC-DB Callback C++ namespace 'SWC::Ranger::Callback'
namespace Callback {


class ManageBase : public Comm::ResponseCallback {
  public:

  typedef std::shared_ptr<ManageBase> Ptr;

  enum Action {
    RANGE_LOAD,
    RANGE_UNLOAD,
    RANGE_UNLOAD_INTERNAL,
    COLUMNS_UNLOAD,
    COLUMN_DELETE
  };

  Action action;

  SWC_CAN_INLINE
  ManageBase(const Comm::ConnHandlerPtr& conn,
             const Comm::Event::Ptr& ev,
             Action a_action)
            : Comm::ResponseCallback(conn, ev),
              action(a_action) {
    Env::Rgr::in_process(1);
  }

  virtual ~ManageBase() noexcept {
    Env::Rgr::in_process(-1);
  }

};


}
}}
#endif // swcdb_ranger_callbacks_ManageBase_h
