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


}}}
