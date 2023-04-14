/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/queries/select/CheckMeta.h"


namespace SWC { namespace Ranger { namespace Query { namespace Select {


SWC_CAN_INLINE
void CheckMeta::run(const RangePtr& range,
                    const Callback::RangeLoad::Ptr& req) {
  Ptr hdlr(new CheckMeta(range, req));
  hdlr->scan(range->cfg->key_seq, hdlr->cid, hdlr->spec);
}

SWC_CAN_INLINE
CheckMeta::CheckMeta(const RangePtr& a_range,
                     const Callback::RangeLoad::Ptr& a_req)
          : client::Query::Select::Handlers::BaseSingleColumn(
              Env::Clients::get(), a_range->cfg->meta_cid),
            range(a_range), req(a_req), spec(DB::Types::Column::SERIAL) {
    auto& key_intval = spec.key_intervals.add();
    key_intval.start.reserve(2);
    key_intval.start.add(std::to_string(range->cfg->cid), Condition::EQ);
    key_intval.start.add("", Condition::FIP);

    DB::Specs::Serial::Value::Fields fields;
    fields.add(
      DB::Specs::Serial::Value::Field_INT64::make(
        0, Condition::EQ, range->rid));
    fields.encode(spec.values.add());
}

bool CheckMeta::valid() noexcept {
  return !state_error && !range->state_unloading();
}

void CheckMeta::response(int err) {
  if(err) {
    int at = Error::OK;
    state_error.compare_exchange_weak(at, err);
  }
  profile.finished();

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
    void operator()() { hdlr->range->check_meta(hdlr); }
  };
  Env::Rgr::post(
    Task(std::dynamic_pointer_cast<CheckMeta>(shared_from_this())));
}



}}}}
