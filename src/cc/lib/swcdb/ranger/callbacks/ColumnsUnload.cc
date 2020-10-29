/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


ColumnsUnload::ColumnsUnload(const Comm::ConnHandlerPtr& conn, 
                             const Comm::Event::Ptr& ev,
                             bool completely)
                            : ManageBase(conn, ev, ManageBase::COLUMNS_UNLOAD), 
                              completely(completely) {
  Env::Rgr::in_process(1);
}

ColumnsUnload::~ColumnsUnload() {
  Env::Rgr::in_process(-1);
}

void ColumnsUnload::add(const ColumnPtr& col) {    
  Core::MutexSptd::scope lock(m_mutex);
  m_cols.push_back(col);
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
    auto it = std::find(m_cols.begin(), m_cols.end(), col);
    if(it != m_cols.end())
      m_cols.erase(it);
    if(!m_cols.empty())
      return;
  }
  response();
}

void ColumnsUnload::response() {
  if(!expired()) {
    try {
      auto cbp = Comm::Buffers::make(m_rsp_params);
      cbp->header.initialize_from_request_header(m_ev->header);
      m_conn->send_response(cbp);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }
  }
}


}}}
