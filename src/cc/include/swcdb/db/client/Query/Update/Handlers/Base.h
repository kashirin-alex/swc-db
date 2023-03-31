/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Update_Handlers_Base_h
#define swcdb_db_client_Query_Update_Handlers_Base_h


#include "swcdb/core/CompletionCounter.h"
#include "swcdb/db/Cells/Cell.h"
#include "swcdb/db/client/Query/Profiling.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace client { namespace Query { namespace Update {



//! The SWC-DB Update Handlers C++ namespace 'SWC::client::Query::Update::Handlers'
namespace Handlers {



class Base : public std::enable_shared_from_this<Base> {
  public:
  typedef std::shared_ptr<Base>     Ptr;


  class Column {
    public:
    typedef std::shared_ptr<Column>     Ptr;

    SWC_CAN_INLINE
    Column() noexcept { }

    Column(Column&&) = delete;
    Column(const Column&) = delete;
    Column& operator=(const Column&) = delete;
    Column& operator=(Column&&) = delete;

    virtual void print(std::ostream& out) = 0;

    virtual cid_t get_cid() const noexcept = 0;

    virtual DB::Types::KeySeq get_sequence() const noexcept = 0;

    virtual bool empty() noexcept = 0;

    virtual size_t size() noexcept = 0;

    virtual size_t size_bytes() noexcept = 0;

    virtual DB::Cell::Key::Ptr get_first_key() = 0;

    virtual DB::Cell::Key::Ptr get_key_next(const DB::Cell::Key& eval_key,
                                            bool start_key=false) = 0;

    virtual size_t add(const DynamicBuffer& cells,
                       const DB::Cell::Key& upto_key,
                       const DB::Cell::Key& from_key,
                       uint32_t skip, bool malformed) = 0;

    virtual size_t add(const DynamicBuffer& cells, bool finalized=false) = 0;

    virtual void add(const DB::Cells::Cell& cell, bool finalized=false) = 0;

    virtual bool get_buff(const DB::Cell::Key& key_start,
                          const DB::Cell::Key& key_end,
                          size_t buff_sz, bool& more,
                          DynamicBuffer& cells_buff) = 0;

    virtual bool get_buff(size_t buff_sz, bool& more,
                          DynamicBuffer& cells_buff) = 0;

    SWC_CAN_INLINE
    DynamicBuffer::Ptr get_buff(const DB::Cell::Key& key_start,
                                const DB::Cell::Key& key_end,
                                size_t buff_sz, bool& more) {
      DynamicBuffer cells_buff;
      return DynamicBuffer::Ptr(
        get_buff(key_start, key_end, buff_sz, more, cells_buff)
          ? new DynamicBuffer(std::move(cells_buff))
          : nullptr
      );
    }

    SWC_CAN_INLINE
    DynamicBuffer::Ptr get_buff(size_t buff_sz, bool& more) {
      DynamicBuffer cells_buff;
      return DynamicBuffer::Ptr(
        get_buff(buff_sz, more, cells_buff)
          ? new DynamicBuffer(std::move(cells_buff))
          : nullptr
      );
    }

    virtual void error(int err) noexcept = 0;

    virtual int error() noexcept  = 0;

    protected:

    virtual ~Column() noexcept { }

  };
  typedef Core::Vector<Column*> Colms;


  Clients::Ptr                      clients;
  Profiling                         profile;
  Core::Atomic<int>                 state_error;
  Core::CompletionCounter<uint64_t> completion;

  Core::Atomic<uint32_t>            timeout;
  Core::Atomic<uint32_t>            timeout_ratio;
  Core::Atomic<uint32_t>            buff_sz;
  Core::Atomic<uint8_t>             buff_ahead;
  const Clients::Flag               executor;

  SWC_CAN_INLINE
  Base(const Clients::Ptr& a_clients,
       Clients::Flag a_executor=Clients::Flag::DEFAULT) noexcept
      : clients(a_clients),
        profile(),
        state_error(Error::OK), completion(0),
        timeout(clients->cfg_send_timeout->get()),
        timeout_ratio(clients->cfg_send_timeout_ratio->get()),
        buff_sz(clients->cfg_send_buff_sz->get()),
        buff_ahead(clients->cfg_send_ahead->get()),
        executor(a_executor),
        m_resend_cells(0) {
    if(!timeout_ratio)
      timeout_ratio.store(1000);
  }

  Base(Base&&) = delete;
  Base(const Base&) = delete;
  Base& operator=(const Base&) = delete;
  Base& operator=(Base&&) = delete;

  virtual bool valid() noexcept = 0;

  virtual void response(int err=Error::OK) = 0;

  virtual bool requires_commit() noexcept = 0;

  virtual bool empty() noexcept = 0;

  virtual size_t size_bytes() noexcept = 0;

  virtual void next(Colms& cols) noexcept = 0;

  virtual Column* next(cid_t cid) noexcept = 0;

  virtual void error(cid_t cid, int err) noexcept = 0;


  virtual int error() noexcept {
    return state_error;
  }

  virtual void error(int err) noexcept {
    int at = Error::OK;
    state_error.compare_exchange_weak(at, err);
  }


  SWC_CAN_INLINE
  void add_resend_count(size_t count) noexcept {
    m_resend_cells.fetch_add(count);
  }

  SWC_CAN_INLINE
  size_t get_resend_count(bool reset = true) noexcept {
    return reset ? m_resend_cells.exchange(0) : m_resend_cells.load();
  }

  void commit() {
    completion.increment();
    Colms cols;
    next(cols);
    for(auto colp : cols)
      commit(colp);
    response();
  }

  SWC_CAN_INLINE
  void commit(const cid_t cid) {
    commit(next(cid));
  }

  void commit(Column* colp) {
    completion.increment();
    if(colp && !colp->empty())
      _execute(colp);
    response();
  }

  protected:

  virtual void _execute(Column* colp) {
    default_executor(colp);
  }

  virtual ~Base() noexcept { }

  private:

  void default_executor(Column* colp);

  Core::Atomic<size_t>              m_resend_cells;
};


}}}}}


#endif // swcdb_db_client_Query_Update_Handlers_Base_h
