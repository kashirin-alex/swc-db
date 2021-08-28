/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/db/Range.h"


namespace SWC { namespace Manager {



void Range::set_state_none() {
  set_state(State::NOTSET, 0);
  Env::Mngr::columns()->assign_add(shared_from_this());
}

void Range::set_deleted() {
  Env::Mngr::columns()->assign_remove(shared_from_this());
  Core::ScopedLock lock(m_mutex);
  m_state = State::DELETED;
}

void Range::set_state_queued(rgrid_t rgrid) {
  Env::Mngr::columns()->assign_remove(shared_from_this());
  int64_t ts = Time::now_ms();
  Core::ScopedLock lock(m_mutex);
  m_state = State::QUEUED;
  m_rgrid = rgrid;
  m_load_revision = 0;
  m_check_ts = ts;
}



}}
