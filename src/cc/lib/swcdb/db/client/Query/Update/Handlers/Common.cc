/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/Handlers/Common.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace client { namespace Query { namespace Update {


namespace Handlers {


Common::Common(const Clients::Ptr& a_clients,
               Cb_t&& cb, const Comm::IoContextPtr& io,
               Clients::Flag a_executor) noexcept
              : BaseUnorderedMap(a_clients, a_executor),
                valid_state(true),
                m_cb(std::move(cb)), m_dispatcher_io(io),
                m_mutex(), m_cv() {
}

bool Common::requires_commit() noexcept {
  return valid() && BaseUnorderedMap::requires_commit();
}

bool Common::valid() noexcept {
  return valid_state;
}

void Common::response(int err) {
  if(!completion.is_last()) {
    Core::ScopedLock lock(m_mutex);
    return m_cv.notify_all();
  }

  if(!err && requires_commit()) {
    commit();
    Core::ScopedLock lock(m_mutex);
    return m_cv.notify_all();
  }

  if(err)
    error(err);
  else if(!empty())
    error(Error::CLIENT_DATA_REMAINED);

  profile.finished();
  if(m_cb) {
    struct Task {
      Ptr hdlr;
      SWC_CAN_INLINE
      Task(Ptr&& a_hdlr) noexcept : hdlr(std::move(a_hdlr)) { }
      SWC_CAN_INLINE
      Task(Task&& other) noexcept : hdlr(std::move(other.hdlr)) { }
      Task(const Task&) = delete;
      Task& operator=(Task&&) = delete;
      Task& operator=(const Task&) = delete;
      ~Task() noexcept { }
      void operator()() {
        hdlr->m_cb(hdlr);

        Core::ScopedLock lock(hdlr->m_mutex);
        hdlr->m_cv.notify_all();
      }
    };
    (m_dispatcher_io ? m_dispatcher_io : clients->get_io())
      ->post(Task(std::dynamic_pointer_cast<Common>(shared_from_this())));

  } else {
    Core::ScopedLock lock(m_mutex);
    m_cv.notify_all();
  }
}


void Common::wait() {
  Core::UniqueLock lock_wait(m_mutex);
  if(completion.count()) {
    m_cv.wait(lock_wait, [this]() { return !completion.count(); });
  }
}

bool Common::wait_ahead_buffers(uint64_t from) {
  size_t bytes = size_bytes();

  Core::UniqueLock lock_wait(m_mutex);
  if(from == completion.count())
    return bytes >= buff_sz;

  if(bytes < buff_sz * buff_ahead)
    return false;

  if(from < completion.count()) {
    m_cv.wait(lock_wait, [this, from]() {
      return from == completion.count() ||
             size_bytes() < buff_sz * buff_ahead;
    });
  }
  return from == completion.count() && size_bytes() >= buff_sz;
}

void Common::commit_or_wait(Base::Column* colp, uint64_t from) {
  if(wait_ahead_buffers(from))
    colp ? commit(colp) : commit();
}

void Common::commit_if_need() {
  if(!completion.count() && size_bytes())
    commit();
}



}}}}}

