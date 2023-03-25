/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


SWC_CAN_INLINE
ColumnsUnload::ColumnsUnload(const Comm::ConnHandlerPtr& conn,
                             const Comm::Event::Ptr& ev,
                             bool a_completely,
                             cid_t a_cid_begin, cid_t a_cid_end) noexcept
                        : ManageBase(conn, ev, ManageBase::COLUMNS_UNLOAD),
                          completely(a_completely),
                          cid_begin(a_cid_begin), cid_end(a_cid_end),
                          m_mutex(), m_cols(), m_rsp_params() {
}

void ColumnsUnload::add(const ColumnPtr& col) {
  m_cols.push_back(col);
}

void ColumnsUnload::run() {
  if(m_cols.empty()) {
    complete();
  } else {
    Core::MutexSptd::scope lock(m_mutex);
    for(auto& col : m_cols)
      col->add_managing(
        std::dynamic_pointer_cast<ManageBase>(shared_from_this()));
  }
}

void ColumnsUnload::unloaded(RangePtr range) {
  if(!range->deleted()) {
    Core::MutexSptd::scope lock(m_mutex);
    m_rsp_params.columns[range->cfg->cid].push_back(range->rid);
  }
}

void ColumnsUnload::unloaded(const ColumnPtr& col) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    auto it = std::find(m_cols.cbegin(), m_cols.cend(), col);
    if(it != m_cols.cend())
      m_cols.erase(it);
    if(!m_cols.empty())
      return;
  }
  complete();
}

void ColumnsUnload::complete() {
  Env::Rgr::columns()->unload(
    cid_begin, cid_end,
    std::dynamic_pointer_cast<ColumnsUnload>(shared_from_this())
  );
  auto cbp = Comm::Buffers::make(m_ev, m_rsp_params);
  if(!m_cols.empty())
    cbp->header.flags |= Comm::Header::FLAG_RESPONSE_PARTIAL_BIT;
  m_conn->send_response(cbp);

  m_rsp_params.columns.clear();
  if(!m_cols.empty())
    run();
}


}}}
