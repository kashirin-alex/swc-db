/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Column_h
#define swcdb_ranger_db_Column_h


#include "swcdb/ranger/db/Range.h"

#include <unordered_map>


namespace SWC { namespace Ranger {


class Column final :
    private std::unordered_map<rid_t, RangePtr>,
    public std::enable_shared_from_this<Column> {
  public:

  const ColumnCfg::Ptr cfg;

  Column(const cid_t cid, const DB::SchemaPrimitives& schema);

  ~Column();

  size_t size_of() const noexcept;

  size_t ranges_count() noexcept;

  bool removing() noexcept;

  bool is_not_used() noexcept;

  RangePtr get_range(const rid_t rid);

  RangePtr get_next(cid_t& last_rid, size_t &idx);

  void get_rids(std::vector<rid_t>& rids);

  void schema_update(const DB::SchemaPrimitives& schema);

  void compact();

  void add_managing(const Callback::ManageBase::Ptr& req);

  void run_mng_queue();

  size_t release(size_t bytes=0);

  void print(std::ostream& out, bool minimal=true);


  RangePtr internal_create(int& err, rid_t rid, bool compacting);

  void internal_unload(const rid_t rid);

  void internal_unload(const rid_t rid, bool& chk_empty);

  void internal_remove(int &err, const rid_t rid);

  void internal_delete(rid_t rid);

  private:

  struct TaskRunMngReq;

  void load(const Callback::RangeLoad::Ptr& req);

  void unload(const Callback::RangeUnload::Ptr& req);

  void unload(const Callback::RangeUnloadInternal::Ptr& req);

  void unload_all(const Callback::ColumnsUnload::Ptr& req);

  void remove(const Callback::ColumnDelete::Ptr& req);

  Core::MutexSptd                                   m_mutex;
  Core::QueueSafeStated<Callback::ManageBase::Ptr>  m_q_mng;
  Core::StateRunning                                m_release;

};

}}

#endif // swcdb_ranger_db_Column_h