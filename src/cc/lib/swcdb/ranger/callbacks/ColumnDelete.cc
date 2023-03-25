/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


SWC_CAN_INLINE
ColumnDelete::ColumnDelete(const Comm::ConnHandlerPtr& conn,
                           const Comm::Event::Ptr& ev,
                           const cid_t a_cid) noexcept
                          : ManageBase(conn, ev, ManageBase::COLUMN_DELETE),
                            cid(a_cid), col(nullptr), m_mutex(), m_ranges() {
}

void ColumnDelete::add(const RangePtr& range) {
  Core::MutexSptd::scope lock(m_mutex);
  m_ranges.push_back(range);
}

void ColumnDelete::removed(const RangePtr& range) {
  col->internal_delete(range->rid);
  SWC_LOG_OUT(LOG_INFO, range->print(SWC_LOG_OSTREAM << "REMOVED RANGE "); );
  {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = std::find(m_ranges.cbegin(), m_ranges.cend(), range);
    if(it != m_ranges.cend())
      m_ranges.erase(it);
    if(!m_ranges.empty())
      return;
  }
  complete();
}

void ColumnDelete::complete() {
  Env::Rgr::columns()->internal_delete(col->cfg->cid);
  response_ok();
  col->run_mng_queue();
}


}}}
