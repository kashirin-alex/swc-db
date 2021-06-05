/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


namespace SWC { namespace client { namespace Mngr {


void ContextManager::handle_disconnect(Comm::ConnHandlerPtr conn) {
  Env::Mngr::role()->disconnection(
    conn->endpoint_remote, conn->endpoint_local);
}


}}}
