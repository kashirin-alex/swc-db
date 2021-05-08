/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Handler_Base_h
#define swcdb_db_client_Query_Select_Handler_Base_h


#include "swcdb/core/CompletionCounter.h"
#include "swcdb/db/client/Query/Profiling.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace client { namespace Query { namespace Select {



//! The SWC-DB Select Handlers C++ namespace 'SWC::client::Query::Select::Handlers'
namespace Handlers {



class Base : public std::enable_shared_from_this<Base>{
  public:
  typedef std::shared_ptr<Base>     Ptr;

  Profiling                         profile;
  Core::Atomic<int>                 state_error;
  Core::CompletionCounter<uint64_t> completion;

  Core::Atomic<uint32_t>            timeout;
  Core::Atomic<uint32_t>            buff_sz;
  Core::Atomic<uint8_t>             buff_ahead;

  Base() noexcept
      : state_error(Error::OK), completion(0),
        timeout(0), buff_sz(0), buff_ahead(0) {
  }

  virtual bool valid() noexcept = 0;

  virtual void error(const cid_t cid, int err) = 0;

  virtual bool add_cells(const cid_t cid, const StaticBuffer& buffer,
                         bool reached_limit,
                         DB::Specs::Interval& interval) = 0;

  virtual size_t get_size_bytes() noexcept = 0;

  virtual void response(int err=Error::OK) = 0;


  virtual int error() noexcept {
    return state_error;
  }

  virtual void error(int err) noexcept {
    int at = Error::OK;
    state_error.compare_exchange_weak(at, err);
  }

  protected:

  virtual ~Base() { }

};



}}}}}

/* if not only pure functions
#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/SelectHandlerBase.cc"
#endif
*/

#endif // swcdb_db_client_Query_Select_Handler_Base_h
