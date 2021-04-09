/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/queries/update/BaseMeta.h"


namespace SWC { namespace Ranger { namespace Query { namespace Update {


BaseMeta::BaseMeta(const RangePtr& range)
                  : client::Query::Update::Handlers::BaseSingleColumn(
                      range->cfg->meta_cid, range->cfg->key_seq, 1, 0,
                      DB::Types::Column::SERIAL),
                    range(range) {
  timeout.store(Env::Clients::ref().cfg_send_timeout->get());
  timeout_ratio.store(Env::Clients::ref().cfg_send_timeout_ratio->get());
  buff_sz.store(Env::Clients::ref().cfg_send_buff_sz->get());
  buff_ahead.store(Env::Clients::ref().cfg_send_ahead->get());
}

bool BaseMeta::valid() noexcept {
  if(range->state_unloading()) {
    error(Error::SERVER_SHUTTING_DOWN);
    return false;
  }
  return true;
}

bool BaseMeta::is_last_rsp(int err) {
  if(!completion.is_last())
    return false;

  if(!err && requires_commit()) {
    client::Query::Update::commit(shared_from_this(), &column);
    return false;
  }

  if(err)
    error(err);
  else if(!empty())
    error(Error::CLIENT_DATA_REMAINED);

  profile.finished();
  return true;
}


}}}}
