/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Handlers_BaseSingleColumn_h
#define swcdb_db_client_Query_Select_Handlers_BaseSingleColumn_h


#include "swcdb/db/client/Query/Select/Handlers/Base.h"
#include "swcdb/db/Cells/Result.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {



class BaseSingleColumn : public Base {
  public:

  typedef std::shared_ptr<BaseSingleColumn>   Ptr;
  using Base::error;

  const cid_t         cid;

  SWC_CAN_INLINE
  BaseSingleColumn(const Clients::Ptr& a_clients, const cid_t a_cid,
                   Clients::Flag a_executor=Clients::DEFAULT) noexcept
                  : Base(a_clients, a_executor),
                    cid(a_cid), m_mutex(), m_cells() {
  }

  virtual ~BaseSingleColumn() noexcept { }


  virtual void error(const cid_t _cid, int err) override;

  virtual bool add_cells(const cid_t _cid, StaticBuffer& buffer,
                         bool reached_limit,
                         DB::Specs::Interval& interval) override;

  virtual size_t get_size_bytes() noexcept override;


  virtual size_t get_size() noexcept;

  virtual bool empty() noexcept;

  virtual void get_cells(DB::Cells::Result& cells);

  private:
  Core::MutexSptd    m_mutex;
  DB::Cells::Result  m_cells;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Select/Handlers/BaseSingleColumn.cc"
#endif


#endif // swcdb_db_client_Query_Select_Handlers_BaseSingleColumn_h
