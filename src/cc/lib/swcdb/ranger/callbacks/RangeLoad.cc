/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Protocol/Rgr/params/RangeLoad.h"

namespace SWC { namespace Ranger { namespace Callback {


RangeLoad::RangeLoad(const Comm::ConnHandlerPtr& conn,
                     const Comm::Event::Ptr& ev,
                     const cid_t cid, const rid_t rid) noexcept
                    : ManageBase(conn, ev, ManageBase::RANGE_LOAD),
                      cid(cid), rid(rid) {
}

void RangeLoad::loaded(int& err) {
  if(!err && (Env::Rgr::is_shuttingdown() ||
              (Env::Rgr::is_not_accepting() &&
               DB::Types::MetaColumn::is_data(cid))))
    err = Error::SERVER_SHUTTING_DOWN;

  RangePtr range;
  if(!err) {
    range = Env::Rgr::columns()->get_range(err, cid, rid);
    if(err || !range || !range->is_loaded())
      err = Error::RGR_NOT_LOADED_RANGE;
  }

  if(err) {
    SWC_LOG_OUT(LOG_WARN,
      Error::print(SWC_LOG_OSTREAM
        << "BAD LOAD RANGE, Unloading(" << cid << '/' << rid << ") ", err);
    );
    col->internal_unload(rid);
    Env::Rgr::columns()->erase_if_empty(cid);

    m_conn->send_error(err, "", m_ev);

  } else {
    Comm::Protocol::Rgr::Params::RangeLoaded params(range->cfg->key_seq);
    if((params.intval = range->cfg->range_type == DB::Types::Range::MASTER))
      range->get_interval(params.interval);

    auto cbp = Comm::Buffers::make(m_ev, params, 4);
    cbp->append_i32(err);
    m_conn->send_response(cbp);
  }

  col->run_mng_queue();
}


}}}
