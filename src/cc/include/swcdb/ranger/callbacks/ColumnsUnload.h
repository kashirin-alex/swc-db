/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_ColumnsUnload_h
#define swcdb_ranger_callbacks_ColumnsUnload_h

#include "swcdb/db/Protocol/Rgr/params/ColumnsUnload.h"

namespace SWC { namespace Ranger {


namespace Callback {


class ColumnsUnload : public ManageBase {
  public:

  typedef std::shared_ptr<ColumnsUnload> Ptr;

  const bool  completely;
  const cid_t cid_begin;
  const cid_t cid_end;

  ColumnsUnload(const Comm::ConnHandlerPtr& conn, const Comm::Event::Ptr& ev,
                bool completely, cid_t cid_begin, cid_t cid_end) noexcept;

  virtual ~ColumnsUnload() { }

  void add(const ColumnPtr& col);

  void run() override;

  virtual void unloaded(RangePtr range);

  virtual void unloaded(const ColumnPtr& col);

  virtual void complete();

  private:

  Core::MutexSptd                                 m_mutex;
  Core::Vector<ColumnPtr>                         m_cols;
  Comm::Protocol::Rgr::Params::ColumnsUnloadRsp   m_rsp_params;

};


}
}}
#endif // swcdb_ranger_callbacks_ColumnsUnload_h
