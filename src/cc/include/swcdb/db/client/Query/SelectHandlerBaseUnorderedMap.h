/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Handler_BaseUnorderedMap_h
#define swcdb_db_client_Query_Select_Handler_BaseUnorderedMap_h


#include "swcdb/db/client/Query/SelectHandlerBase.h"
#include "swcdb/db/Cells/Result.h"
#include "swcdb/db/Cells/SpecsScan.h"


namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {



class BaseUnorderedMap : public Base {
  public:

  typedef std::shared_ptr<BaseUnorderedMap>   Ptr;

  BaseUnorderedMap(const Clients::Ptr& clients) noexcept
                  : Base(clients) {
  }

  virtual ~BaseUnorderedMap() { }


  virtual void error(const cid_t cid, int err) override;

  virtual bool add_cells(const cid_t cid, const StaticBuffer& buffer,
                         bool reached_limit,
                         DB::Specs::Interval& interval) override;

  virtual size_t get_size_bytes() noexcept override;

  virtual void get_cells(const cid_t cid, DB::Cells::Result& cells);

  virtual void free(const cid_t cid);


  struct Rsp final {
    public:
    typedef std::shared_ptr<Rsp> Ptr;

    Rsp() noexcept : m_err(Error::OK) { }

    //~Rsp() { }

    bool add_cells(const StaticBuffer& buffer, bool reached_limit,
                   DB::Specs::Interval& interval);

    void get_cells(DB::Cells::Result& cells);

    size_t get_size() noexcept;

    size_t get_size_bytes() noexcept;

    bool empty() noexcept;

    void free();

    void error(int err) noexcept;

    int error() noexcept;

    private:
    Core::MutexSptd    m_mutex;
    DB::Cells::Result  m_cells;
    int                m_err;
  };


  void add_column(const cid_t cid);

  Rsp::Ptr& get_columnn(const cid_t cid);

  size_t get_size(const cid_t cid);

  bool empty() noexcept;

  std::vector<cid_t> get_cids();

  void remove(const cid_t cid);

  private:
  Core::MutexSptd                     m_mutex;
  std::unordered_map<cid_t, Rsp::Ptr> m_columns;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/SelectHandlerBaseUnorderedMap.cc"
#endif


#endif // swcdb_db_client_Query_Select_Handler_BaseUnorderedMap_h
