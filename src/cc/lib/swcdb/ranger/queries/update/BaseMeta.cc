/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/queries/update/BaseMeta.h"


namespace SWC { namespace Ranger { namespace Query { namespace Update {


SWC_CAN_INLINE
BaseMeta::BaseMeta(const RangePtr& a_range)
                  : client::Query::Update::Handlers::BaseSingleColumn(
                      Env::Clients::get(),
                      a_range->cfg->meta_cid, a_range->cfg->key_seq, 1, 0,
                      DB::Types::Column::SERIAL
                    ),
                    range(a_range) {
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
    commit(&column);
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
