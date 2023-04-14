/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeLoad_h
#define swcdb_ranger_callbacks_RangeLoad_h


namespace SWC { namespace Ranger { namespace Callback {


class RangeLoad : public ManageBase {
  public:

  typedef std::shared_ptr<RangeLoad> Ptr;

  const cid_t  cid;
  const rid_t  rid;

  ColumnPtr    col;

  RangeLoad(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
            const cid_t cid, const rid_t rid) noexcept;

  RangeLoad(RangeLoad&&) = delete;
  RangeLoad(const RangeLoad&) = delete;
  RangeLoad& operator=(RangeLoad&&) = delete;
  RangeLoad& operator=(const RangeLoad&) = delete;

  virtual ~RangeLoad() noexcept { }

  void loaded(int& err);

};


}}}
#endif // swcdb_ranger_callbacks_RangeLoad_h
