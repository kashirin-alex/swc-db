/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Query/Update/Handlers/Metrics.h"
#include "swcdb/db/client/Query/Update.h"
#include "swcdb/db/client/Clients.h"


namespace SWC { namespace client { namespace Query { namespace Update {
namespace Handlers { namespace Metric {



void Level::report(Handlers::Base::Column* colp,
                   const DB::Cell::KeyVec& parent_key) {
  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);
  for(auto& m : metrics)
    m->report(colp, key);
}

void Level::reset() {
  for(auto& m : metrics)
    m->reset();
}



void Item_MinMaxAvgCount::add(uint64_t v) {
  Core::MutexAtomic::scope lock(m_mutex);
  if(UINT64_MAX - m_total >= v) {
    m_total += v;
    ++m_count;
  }
  if(!m_min || v < m_min)
    m_min = v;
  if(v > m_max)
    m_max = v;
}

void Item_MinMaxAvgCount::report(Handlers::Base::Column* colp,
                                 const DB::Cell::KeyVec& parent_key) {
  m_mutex.lock();
  if(!m_count) {
    m_mutex.unlock();
    return;
  }
  auto _min = m_min;
  auto _max = m_max;
  auto _avg = m_total / m_count;
  auto _count = m_count;
  m_min = 0;
  m_max = 0;
  m_total = 0;
  m_count = 0;
  m_mutex.unlock();

  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);

  DB::Cells::Cell cell;
  cell.flag = DB::Cells::INSERT;
  cell.set_time_order_desc(true);
  cell.key.add(key);

  DB::Cell::Serial::Value::FieldsWriter wfields;
  wfields.ensure(
    + Serialization::encoded_length_vi64(_min)
    + Serialization::encoded_length_vi64(_max)
    + Serialization::encoded_length_vi64(_avg)
    + Serialization::encoded_length_vi64(_count)
    + 8
  );
  wfields.add(FIELD_ID_MIN,   int64_t(_min));
  wfields.add(FIELD_ID_MAX,   int64_t(_max));
  wfields.add(FIELD_ID_AVG,   int64_t(_avg));
  wfields.add(FIELD_ID_COUNT, int64_t(_count));
  cell.set_value(wfields.base, wfields.fill(), false);

  colp->add(cell);
}

void Item_MinMaxAvgCount::reset() {
  Core::MutexAtomic::scope lock(m_mutex);
  m_min = 0;
  m_max = 0;
  m_total = 0;
  m_count = 0;
}





Reporting::Reporting(const Comm::IoContextPtr& io,
                     Config::Property::V_GINT32::Ptr cfg_intval_ms)
            : BaseSingleColumn(
                9, DB::Types::KeySeq::LEXIC, 1, 0, DB::Types::Column::SERIAL),
              io(io),
              cfg_intval_ms(cfg_intval_ms),
              running(false),
              m_timer(io->executor()) {
  timeout.store(Env::Clients::ref().cfg_send_timeout->get());
  timeout_ratio.store(Env::Clients::ref().cfg_send_timeout_ratio->get());
  buff_sz.store(Env::Clients::ref().cfg_send_buff_sz->get());
  buff_ahead.store(Env::Clients::ref().cfg_send_ahead->get());
}

void Reporting::stop() {
  bool at = true;
  if(running.compare_exchange_weak(at, false)) {
    Core::MutexSptd::scope lock(m_mutex);
    m_timer.cancel();
  }
  while(completion.count())
    std::this_thread::sleep_for(std::chrono::microseconds(100));
}

void Reporting::response(int err) {
  if(!completion.is_last())
    return;

  if(!err && requires_commit()) {
    Update::commit(shared_from_this(), &column);
    return;
  }

  if(err)
    error(err);
  else if(!empty())
    error(Error::CLIENT_DATA_REMAINED);

  if(error() || column.error()) {
    SWC_LOG(LOG_WARN, "Problem Updating Statistics");

    // reset-state
    column.state_error.store(Error::OK);
    state_error.store(Error::OK);
  }

  profile.finished();
  schedule();
}

void Reporting::report() {
  if(!cfg_intval_ms->get()) {
    for(auto& m : metrics)
      m->reset();
    return schedule();
  }

  DB::Cell::KeyVec key;
  key.add(std::to_string(Time::now_ms()));
  for(auto& m : metrics)
    m->report(&column, key);

  profile.reset();

  column.empty()
    ? schedule()
    : Update::commit(shared_from_this(), &column);
}

void Reporting::schedule() {
  auto ms = cfg_intval_ms->get();
  auto in = std::chrono::milliseconds(ms ? ms : 300000);
  // OR at on cfg_intval_ms rounding
  Core::MutexSptd::scope lock(m_mutex);
  if(!running)
    return;
  m_timer.expires_after(in);
  m_timer.async_wait([this](const asio::error_code& ec) {
    if(ec != asio::error::operation_aborted)
      report();
  });
}





}}}}}}
