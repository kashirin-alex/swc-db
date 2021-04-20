/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeUnload_h
#define swcdb_ranger_callbacks_RangeUnload_h


#include "swcdb/db/Protocol/Rgr/params/RangeUnload.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeUnload : public ManageBase {
  public:

  typedef std::shared_ptr<RangeUnload> Ptr;

  const cid_t                                 cid;
  const rid_t                                 rid;
  const bool                                  completely;
  Comm::Protocol::Rgr::Params::RangeUnloadRsp rsp_params;

  RangeUnload(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
              const cid_t cid, const rid_t rid,
              const bool completely) noexcept;

  virtual ~RangeUnload() { }

  void response_params();

};


}}}
#endif // swcdb_ranger_callbacks_RangeUnload_h
