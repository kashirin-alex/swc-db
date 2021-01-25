/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeUnload_h
#define swcdb_ranger_callbacks_RangeUnload_h


namespace SWC { namespace Ranger { namespace Callback {


class RangeUnload : public ManageBase {
  public:

  typedef std::shared_ptr<RangeUnload> Ptr;

  const cid_t   cid;
  const rid_t   rid;
  const bool    completely;

  RangeUnload(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
              const cid_t cid, const rid_t rid,
              const bool completely) noexcept;

  virtual ~RangeUnload();

};


}}}
#endif // swcdb_ranger_callbacks_RangeUnload_h
