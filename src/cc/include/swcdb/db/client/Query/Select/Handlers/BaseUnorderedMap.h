/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Handlers_BaseUnorderedMap_h
#define swcdb_db_client_Query_Select_Handlers_BaseUnorderedMap_h


#include "swcdb/db/client/Query/Select/Handlers/Base.h"
#include "swcdb/db/Cells/Result.h"
#include "swcdb/db/Cells/SpecsScan.h"


namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {



class BaseUnorderedMap : public Base {
  public:

  typedef std::shared_ptr<BaseUnorderedMap>   Ptr;

  SWC_CAN_INLINE
  BaseUnorderedMap(const Clients::Ptr& clients,
                   Clients::Flag executor=Clients::DEFAULT) noexcept
                  : Base(clients, executor) {
  }

  virtual ~BaseUnorderedMap() { }

  using Base::error;

  virtual void error(const cid_t cid, int err) override {
    get_columnn(cid)->error(err);
  }

  virtual bool add_cells(const cid_t cid, StaticBuffer& buffer,
                         bool reached_limit,
                         DB::Specs::Interval& interval) override {
    return get_columnn(cid)->add_cells(buffer, reached_limit, interval);
  }

  virtual size_t get_size_bytes() noexcept override;

  virtual void get_cells(const cid_t cid, DB::Cells::Result& cells) {
    get_columnn(cid)->get_cells(cells);
  }

  virtual void free(const cid_t cid) {
    get_columnn(cid)->free();
  }


  struct Rsp final {
    public:
    typedef std::shared_ptr<Rsp> Ptr;

    SWC_CAN_INLINE
    Rsp() noexcept : m_err(Error::OK) { }

    //~Rsp() { }

    bool add_cells(StaticBuffer& buffer, bool reached_limit,
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

  SWC_CAN_INLINE
  size_t get_size(const cid_t cid) {
    return get_columnn(cid)->get_size();
  }

  bool empty() noexcept;

  cids_t get_cids();

  void remove(const cid_t cid);

  private:
  Core::MutexSptd                     m_mutex;
  std::unordered_map<cid_t, Rsp::Ptr> m_columns;

};



}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Select/Handlers/BaseUnorderedMap.cc"
#endif


#endif // swcdb_db_client_Query_Select_Handlers_BaseUnorderedMap_h
