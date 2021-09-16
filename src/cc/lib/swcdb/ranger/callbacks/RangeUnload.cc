/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


SWC_CAN_INLINE
RangeUnload::RangeUnload(const Comm::ConnHandlerPtr& conn,
                         const Comm::Event::Ptr& ev,
                         const cid_t a_cid, const rid_t a_rid,
                         const bool a_completely) noexcept
                        : ManageBase(conn, ev, ManageBase::RANGE_UNLOAD),
                          cid(a_cid), rid(a_rid),
                          completely(a_completely), rsp_params(Error::OK) {
}

void RangeUnload::response_params() {
  m_conn->send_response(Comm::Buffers::make(m_ev, rsp_params));
}

}}}
