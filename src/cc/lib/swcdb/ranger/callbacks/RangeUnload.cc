/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace Ranger { namespace Callback {


RangeUnload::RangeUnload(const Comm::ConnHandlerPtr& conn,
                         const Comm::Event::Ptr& ev,
                         const cid_t cid, const rid_t rid,
                         const bool completely) noexcept
                        : ManageBase(conn, ev, ManageBase::RANGE_UNLOAD),
                          cid(cid), rid(rid), completely(completely) {
}

RangeUnload::~RangeUnload() { }


}}}
