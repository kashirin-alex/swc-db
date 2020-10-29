/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


ColumnDelete::ColumnDelete(const Comm::ConnHandlerPtr& conn, 
                           const Comm::Event::Ptr& ev,
                           const cid_t cid)
                          : ManageBase(conn, ev, ManageBase::COLUMN_DELETE), 
                            cid(cid) {
  Env::Rgr::in_process(1);
}

ColumnDelete::~ColumnDelete() { 
  Env::Rgr::in_process(-1);
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
    auto it = std::find(m_ranges.begin(), m_ranges.end(), range);
    if(it != m_ranges.end())
      m_ranges.erase(it);
    if(!m_ranges.empty())
      return;
  }
  response();
}

void ColumnDelete::response() {
  Env::Rgr::columns()->erase_if_empty(col->cfg->cid);
  if(!expired())
    response_ok();
  col->run_mng_queue();
}


}}}
