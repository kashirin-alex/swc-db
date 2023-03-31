/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Handlers_Base_h
#define swcdb_db_client_Query_Select_Handlers_Base_h


#include "swcdb/core/CompletionCounter.h"
#include "swcdb/db/Cells/SpecsScan.h"
#include "swcdb/db/client/Query/Profiling.h"
#include "swcdb/db/client/Clients.h"



namespace SWC { namespace client { namespace Query { namespace Select {



//! The SWC-DB Select Handlers C++ namespace 'SWC::client::Query::Select::Handlers'
namespace Handlers {



class Base : public std::enable_shared_from_this<Base>{
  public:
  typedef std::shared_ptr<Base>     Ptr;

  Clients::Ptr                      clients;
  Profiling                         profile;
  Core::Atomic<int>                 state_error;
  Core::CompletionCounter<uint64_t> completion;

  Core::Atomic<uint32_t>            timeout;
  Core::Atomic<uint32_t>            buff_sz;
  Core::Atomic<uint8_t>             buff_ahead;
  const Clients::Flag               executor;

  SWC_CAN_INLINE
  Base(const Clients::Ptr& a_clients,
       Clients::Flag a_executor=Clients::DEFAULT) noexcept
      : clients(a_clients), profile(),
        state_error(Error::OK), completion(0),
        timeout(clients->cfg_recv_timeout->get()),
        buff_sz(clients->cfg_recv_buff_sz->get()),
        buff_ahead(clients->cfg_recv_ahead->get()),
        executor(a_executor) {
  }

  Base(Base&&) = delete;
  Base(const Base&) = delete;
  Base& operator=(const Base&) = delete;
  Base& operator=(Base&&) = delete;

  virtual bool valid() noexcept = 0;

  virtual void error(const cid_t cid, int err) = 0;

  virtual bool add_cells(const cid_t cid, StaticBuffer& buffer,
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

  SWC_CAN_INLINE
  void scan(const DB::Schema::Ptr& schema,
            const DB::Specs::Interval& intval) {
    _execute(schema->col_seq, schema->cid, intval);
  }

  SWC_CAN_INLINE
  void scan(const DB::Schema::Ptr& schema,
            DB::Specs::Interval&& intval) {
    _execute(schema->col_seq, schema->cid, std::move(intval));
  }

  SWC_CAN_INLINE
  void scan(DB::Types::KeySeq key_seq, cid_t cid,
            const DB::Specs::Interval& intval) {
    _execute(key_seq, cid, intval);
  }

  SWC_CAN_INLINE
  void scan(DB::Types::KeySeq key_seq, cid_t cid,
            DB::Specs::Interval&& intval) {
    _execute(key_seq, cid, std::move(intval));
  }

  SWC_CAN_INLINE
  void scan(int& err, const DB::Specs::Scan& specs) {
    _execute(err, specs);
  }

  SWC_CAN_INLINE
  void scan(int& err, DB::Specs::Scan&& specs) {
    _execute(err, std::move(specs));
  }

  protected:

  virtual void _execute(DB::Types::KeySeq key_seq, cid_t cid,
                        const DB::Specs::Interval& intval) {
    default_executor(key_seq, cid, intval);
  }

  virtual void _execute(DB::Types::KeySeq key_seq, cid_t cid,
                        DB::Specs::Interval&& intval) {
    default_executor(key_seq, cid, std::move(intval));
  }

  virtual void _execute(int& err, const DB::Specs::Scan& specs) {
    default_executor(err, specs);
  }

  virtual void _execute(int& err, DB::Specs::Scan&& specs) {
    default_executor(err, std::move(specs));
  }

  virtual ~Base() noexcept { }

  private:

  void default_executor(DB::Types::KeySeq key_seq, cid_t cid,
                        const DB::Specs::Interval& intval);

  void default_executor(DB::Types::KeySeq key_seq, cid_t cid,
                        DB::Specs::Interval&& intval);

  void default_executor(int& err, const DB::Specs::Scan& specs);

  void default_executor(int& err, DB::Specs::Scan&& specs);

};



}}}}}


#endif // swcdb_db_client_Query_Select_Handlers_Base_h
