/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_ColumnDelete_h
#define swcdb_ranger_callbacks_ColumnDelete_h


namespace SWC { namespace Ranger { namespace Callback {


class ColumnDelete : public ManageBase {
  public:

  typedef std::shared_ptr<ColumnDelete> Ptr;

  const cid_t  cid;
  ColumnPtr    col;

  ColumnDelete(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
               const cid_t cid) noexcept;

  virtual ~ColumnDelete() { }

  void add(const RangePtr& range);

  void removed(const RangePtr& range);

  void complete();

  private:
  Core::MutexSptd        m_mutex;
  Core::Vector<RangePtr> m_ranges;

};


}}}
#endif // swcdb_ranger_callbacks_ColumnDelete_h
