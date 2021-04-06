/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/queries/select/CheckMeta.h"


namespace SWC { namespace Ranger { namespace Query { namespace Select {


void CheckMeta::run(const RangePtr& range,
                    const Callback::RangeLoad::Ptr& req) {
  auto hdlr = std::make_shared<CheckMeta>(range, req);
  client::Query::Select::scan(
    hdlr, range->cfg->key_seq, hdlr->cid, hdlr->spec);
}

CheckMeta::CheckMeta(const RangePtr& range,
                     const Callback::RangeLoad::Ptr& req)
          : client::Query::Select::Handlers::BaseSingleColumn(
              range->cfg->meta_cid),
            range(range), req(req), spec(DB::Types::Column::SERIAL) {

    buff_sz.store(Env::Clients::ref().cfg_recv_buff_sz->get());
    buff_ahead.store(Env::Clients::ref().cfg_recv_ahead->get());
    timeout.store(Env::Clients::ref().cfg_recv_timeout->get());

    auto& key_intval = spec.key_intervals.add();
    key_intval->start.add(std::to_string(range->cfg->cid), Condition::EQ);
    key_intval->start.add("", Condition::GE);

    DB::Specs::Serial::Value::Fields fields;
    fields.add(
      DB::Specs::Serial::Value::Field_INT64::make(
        0, Condition::EQ, range->rid));
    fields.encode(spec.values.add());
}

bool CheckMeta::valid() noexcept {
  if(state_error ||
     Env::Rgr::is_shuttingdown() ||
     (Env::Rgr::is_not_accepting() &&
      DB::Types::MetaColumn::is_data(range->cfg->cid))) { // ? any-false
    return false;
  }
  return true;
}

void CheckMeta::response(int err) {
  if(err) {
    int at = Error::OK;
    state_error.compare_exchange_weak(at, err);
  }
  profile.finished();

  Env::Rgr::post(
    [this, hdlr=std::dynamic_pointer_cast<CheckMeta>(shared_from_this())]() {
      range->check_meta(hdlr); });
}



}}}}
