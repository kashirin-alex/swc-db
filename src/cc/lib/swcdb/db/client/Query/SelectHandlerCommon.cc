/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/SelectHandlerCommon.h"



namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {


Common::Common(Cb_t&& cb, bool rsp_partials,
               const Comm::IoContextPtr& io) noexcept
              : valid_state(true), 
                m_cb(std::move(cb)), m_dispatcher_io(io),
                m_notify(m_cb && rsp_partials),
                m_sending_result(false) {
  buff_sz.store(Env::Clients::ref().cfg_recv_buff_sz->get());
  buff_ahead.store(Env::Clients::ref().cfg_recv_ahead->get());
  timeout.store(Env::Clients::ref().cfg_recv_timeout->get());
}

bool Common::add_cells(const cid_t cid, const StaticBuffer& buffer,
                       bool reached_limit, DB::Specs::Interval& interval) {
  bool more = BaseUnorderedMap::add_cells(
    cid, buffer, reached_limit, interval);
  response_partials();
  return more;
}

void Common::get_cells(const cid_t cid, DB::Cells::Result& cells) {
  BaseUnorderedMap::get_cells(cid, cells);
  if(m_notify) {
    std::scoped_lock lock(m_mutex);
    m_cv.notify_all();
  }
}

void Common::free(const cid_t cid) {
  BaseUnorderedMap::free(cid);
  if(m_notify) {
    std::scoped_lock lock(m_mutex);
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
    std::scoped_lock lock(m_mutex);
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
    std::unique_lock lock_wait(m_mutex);
    if(m_sending_result.running()) {
      if(wait_on_partials()) {
        m_cv.wait(
          lock_wait,
          [this, hdlr=shared_from_this()] () {
            return !m_sending_result || !wait_on_partials();
          }
        );
      }
      return;
    }
  }
  send_result();
}

bool Common::wait_on_partials() {
  return get_size_bytes() > buff_sz * buff_ahead;
}

void Common::wait() {
  _wait: {
    {
      std::unique_lock lock_wait(m_mutex);
      if(m_sending_result || completion.count()) {
        m_cv.wait(
          lock_wait,
          [this, hdlr=shared_from_this()] () {
            return !m_sending_result && !completion.count();
          }
        );
      }
    }
    if(m_notify && !empty()) {
      if(!m_sending_result.running())
        send_result();
      goto _wait;
    }
  }
}

void Common::send_result() {
  auto hdlr = std::dynamic_pointer_cast<Common>(shared_from_this());
  if(m_dispatcher_io) {
    m_dispatcher_io->post([this, hdlr](){
      m_cb(hdlr);

      std::scoped_lock lock(m_mutex);
      m_sending_result.stop();
      m_cv.notify_all();
    });
  } else {
    Env::IoCtx::post([this, hdlr](){
      m_cb(hdlr);

      std::scoped_lock lock(m_mutex);
      m_sending_result.stop();
      m_cv.notify_all();
    });
  }
}






}}}}}