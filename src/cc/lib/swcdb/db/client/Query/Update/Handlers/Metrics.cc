/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/client/Query/Update/Handlers/Metrics.h"


namespace SWC { namespace client { namespace Query { namespace Update {
namespace Handlers { namespace Metric {


const char* aggregation_to_string(uint8_t agg) noexcept {
  switch(agg) {
    case Aggregation::SUM:
      return "SUM";
    case Aggregation::MIN:
      return "MIN";
    case Aggregation::MAX:
      return "MAX";
    case Aggregation::AVG:
      return "AVG";
    case Aggregation::AVG_PROP:
      return "AVG_PROP";
    default:
      return "UNKNOWN";
  }
}


void Level::definitions(Handlers::Base::Column* colp,
                        const DB::Cell::KeyVec& parent_key) {
  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);
  for(auto& m : metrics)
    m->definitions(colp, key);
}

void Level::report(uint64_t for_ns, Handlers::Base::Column* colp,
                   const DB::Cell::KeyVec& parent_key) {
  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);
  for(auto& m : metrics)
    m->report(for_ns, colp, key);
}

void Level::reset() {
  for(auto& m : metrics)
    m->reset();
}

Level* Level::get_level(const char* _name, bool inner) {
  if(!inner)
    return Condition::str_eq(name.c_str(), _name) ? this : nullptr;

  Level* level;
  for(auto& m : metrics) {
    if((level = m->get_level(_name, false)))
      return level;
  }
  metrics.emplace_back((level = new Level(_name)));
  return level;
}



void Item_MinMaxAvgCount::report(uint64_t for_ns,
                                 Handlers::Base::Column* colp,
                                 const DB::Cell::KeyVec& parent_key) {
  SWC::Common::Stats::MinMaxAvgCount<uint64_t> value;
  gather(value);
  if(!value.count)
    return;

  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);

  DB::Cells::Cell cell;
  cell.flag = DB::Cells::INSERT;
  cell.set_time_order_desc(true);
  cell.set_timestamp(for_ns);
  cell.key.add(key);

  size_t sz = 4;
  uint64_t avg = value.avg();
  if(avg != value.min)
    sz += 2 + Serialization::encoded_length_vi64(value.min);
  if(avg != value.max)
    sz += 2 + Serialization::encoded_length_vi64(value.max);
  sz += Serialization::encoded_length_vi64(avg);
  sz += Serialization::encoded_length_vi64(value.count);

  DB::Cell::Serial::Value::FieldsWriter wfields;
  wfields.ensure(sz);
  if(avg != value.min)
    wfields.add(FIELD_ID_MIN,   int64_t(value.min));
  if(avg != value.max)
    wfields.add(FIELD_ID_MAX,   int64_t(value.max));
  wfields.add(FIELD_ID_COUNT, int64_t(value.count));
  wfields.add(FIELD_ID_AVG,   int64_t(avg));
  cell.set_value(wfields.base, wfields.fill(), false);

  colp->add(cell);
}


void Item_Count::report(uint64_t for_ns, Handlers::Base::Column* colp,
                        const DB::Cell::KeyVec& parent_key) {
  int64_t _count = m_count.exchange(0);

  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);

  DB::Cells::Cell cell;
  cell.flag = DB::Cells::INSERT;
  cell.set_time_order_desc(true);
  cell.set_timestamp(for_ns);
  cell.key.add(key);

  DB::Cell::Serial::Value::FieldsWriter wfields;
  wfields.add(FIELD_ID_COUNT, _count);

  cell.set_value(wfields.base, wfields.fill(), false);
  colp->add(cell);
}



void Item_Volume::report(uint64_t for_ns, Handlers::Base::Column* colp,
                         const DB::Cell::KeyVec& parent_key) {
  int64_t _volume = m_volume.load();

  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);

  DB::Cells::Cell cell;
  cell.flag = DB::Cells::INSERT;
  cell.set_time_order_desc(true);
  cell.set_timestamp(for_ns);
  cell.key.add(key);

  DB::Cell::Serial::Value::FieldsWriter wfields;
  wfields.add(FIELD_ID_VOLUME, _volume);

  cell.set_value(wfields.base, wfields.fill(), false);
  colp->add(cell);
}



void Item_CountVolume::report(uint64_t for_ns, Handlers::Base::Column* colp,
                              const DB::Cell::KeyVec& parent_key) {
  int64_t _count = m_count.exchange(0);
  int64_t _volume = m_volume.load();

  DB::Cell::KeyVec key;
  key.reserve(parent_key.size() + 1);
  key.copy(parent_key);
  key.add(name);

  DB::Cells::Cell cell;
  cell.flag = DB::Cells::INSERT;
  cell.set_time_order_desc(true);
  cell.set_timestamp(for_ns);
  cell.key.add(key);

  DB::Cell::Serial::Value::FieldsWriter wfields;
  wfields.ensure(
    + Serialization::encoded_length_vi64(_count)
    + Serialization::encoded_length_vi64(_volume)
    + 4
  );
  wfields.add(FIELD_ID_COUNT,   _count);
  wfields.add(FIELD_ID_VOLUME,  _volume);

  cell.set_value(wfields.base, wfields.fill(), false);
  colp->add(cell);
}





Reporting::Reporting(const Clients::Ptr& a_clients,
                     const Comm::IoContextPtr& a_io,
                     Config::Property::Value_int32_g::Ptr a_cfg_intval,
                     Clients::Flag a_executor)
            : BaseSingleColumn(
                a_clients,
                DB::Types::SystemColumn::SYS_CID_DEFINE_LEXIC,
                DB::Types::KeySeq::LEXIC, 1, 0, DB::Types::Column::SERIAL,
                a_executor
              ),
              io(a_io),
              cfg_intval(a_cfg_intval),
              metrics(),
              running(false),
              m_mutex(),
              m_timer(io->executor()) {
}

void Reporting::stop() {
  bool at = true;
  if(running.compare_exchange_weak(at, false)) {
    Core::MutexSptd::scope lock(m_mutex);
    m_timer.cancel();
  }
}

void Reporting::wait() {
  for(size_t n=0; completion.count(); ++n) {
    if(n % 30000 == 0)
      SWC_LOGF(LOG_WARN, "Reporting::wait completion=" SWC_FMT_LU,
               completion.count());
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }
}

Level* Reporting::get_level(const char* _name) {
  Level* level;
  for(auto& m : metrics) {
    if((level = m->get_level(_name, false)))
      return level;
  }
  metrics.emplace_back((level = new Level(_name)));
  return level;
}

uint64_t Reporting::apply_time(uint32_t intval, DB::Cell::KeyVec& key) {
  uint64_t for_ns = (::time(nullptr) / intval) * intval;
  key.add(std::to_string(for_ns));
  return for_ns * 1000000000;
}

void Reporting::response(int err) {
  if(!completion.is_last())
    return;

  if(!err && running && requires_commit()) {
    commit(&column);
    return;
  }

  if(err)
    error(err);
  else if(!empty())
    error(Error::CLIENT_DATA_REMAINED);

  profile.finished();

  if(error() || column.error()) {
    SWC_LOG_OUT(LOG_WARN,
      SWC_LOG_OSTREAM << "Problem Updating Statistics cid=" << column.cid;
      Error::print(SWC_LOG_OSTREAM << " hdlr=", error());
      Error::print(SWC_LOG_OSTREAM << " colm=", column.error());
    );
    // reset-state
    column.state_error.store(Error::OK);
    state_error.store(Error::OK);
  } else if(column.cid == DB::Types::SystemColumn::SYS_CID_DEFINE_LEXIC) {
    column.cid = DB::Types::SystemColumn::SYS_CID_STATS;
    report();
    return;
  }

  schedule();
}

void Reporting::report() {
  auto intval = cfg_intval->get();
  if(!intval) {
    for(auto& m : metrics)
      m->reset();
    return schedule();
  }
  if(!running)
    return;

  DB::Cell::KeyVec key;
  if(column.cid == DB::Types::SystemColumn::SYS_CID_DEFINE_LEXIC) {
    key.add(std::to_string(DB::Types::SystemColumn::SYS_CID_STATS));
    for(auto& m : metrics)
      m->definitions(&column, key);
  } else {
    uint64_t for_ns = apply_time(intval, key);
    for(auto& m : metrics)
      m->report(for_ns, &column, key);
  }
  profile.reset();

  column.empty() ? schedule() : commit(&column);
}

void Reporting::schedule() {
  auto intval = cfg_intval->get();
  if(!intval)
    intval = 300;
  uint32_t secs = ::time(nullptr);
  secs = ((secs/intval) * intval + intval) - secs;
  Core::MutexSptd::scope lock(m_mutex);
  if(!running)
    return;
  m_timer.expires_after(std::chrono::seconds(secs));
  struct TimerTask {
    Reporting* ptr;
    SWC_CAN_INLINE
    TimerTask(Reporting* a_ptr) noexcept : ptr(a_ptr) { }
    void operator()(const asio::error_code& ec) {
      if(ec != asio::error::operation_aborted)
        ptr->report();
    }
  };
  m_timer.async_wait(TimerTask(this));
}





}}}}}}
