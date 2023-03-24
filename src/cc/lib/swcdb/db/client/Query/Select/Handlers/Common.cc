/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"



namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {


Common::Common(const Clients::Ptr& a_clients,
               Cb_t&& cb, bool rsp_partials,
               const Comm::IoContextPtr& io,
               Clients::Flag a_executor) noexcept
              : BaseUnorderedMap(a_clients, a_executor),
                valid_state(true),
                m_mutex(),
                m_cb(std::move(cb)), m_dispatcher_io(io),
                m_notify(m_cb && rsp_partials),
                m_cv(),
                m_sending_result(false) {
}

bool Common::add_cells(const cid_t cid, StaticBuffer& buffer,
                       bool reached_limit, DB::Specs::Interval& interval) {
  bool more = BaseUnorderedMap::add_cells(
    cid, buffer, reached_limit, interval);
  response_partials();
  return more;
}

void Common::get_cells(const cid_t cid, DB::Cells::Result& cells) {
  BaseUnorderedMap::get_cells(cid, cells);
  if(m_notify) {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  }
}

void Common::free(const cid_t cid) {
  BaseUnorderedMap::free(cid);
  if(m_notify) {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  }
}

void Common::response(int err) {
  if(err) {
    int at = Error::OK;
    state_error.compare_exchange_weak(at, err);
  }

  profile.finished();

  if(m_cb) {
    if(!m_sending_result.running())
      send_result();
  } else {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  }
}

bool Common::valid() noexcept {
  return valid_state;
}

void Common::response_partials() {
  if(!m_notify)
    return;

  if(m_sending_result.running()) {
    Core::UniqueLock lock_wait(m_mutex);
    if(m_sending_result.running()) {
      if(wait_on_partials()) {
        m_cv.wait(lock_wait, [this] () {
          return !m_sending_result || !wait_on_partials();
        });
      }
      return;
    }
  }
  send_result();
}

bool Common::wait_on_partials() {
  return valid() && get_size_bytes() > buff_sz * buff_ahead;
}

void Common::wait() {
  _wait: {
    {
      Core::UniqueLock lock_wait(m_mutex);
      if(m_sending_result || completion.count()) {
        m_cv.wait(lock_wait, [this] () {
          return !m_sending_result && !completion.count();
        });
      }
    }
    if(m_notify && valid() && !empty()) {
      if(!m_sending_result.running())
        send_result();
      goto _wait;
    }
  }
}

void Common::send_result() {
  struct Task {
    Ptr hdlr;
    SWC_CAN_INLINE
    Task(Ptr&& a_hdlr) noexcept : hdlr(std::move(a_hdlr)) { }
    ~Task() noexcept { }
    void operator()() {
      hdlr->m_cb(hdlr);

      Core::ScopedLock lock(hdlr->m_mutex);
      hdlr->m_sending_result.stop();
      hdlr->m_cv.notify_all();
    }
  };
  (m_dispatcher_io ? m_dispatcher_io : clients->get_io())
    ->post(Task(std::dynamic_pointer_cast<Common>(shared_from_this())));
}






}}}}}
