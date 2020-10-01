/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Column_h
#define swcdb_ranger_db_Column_h


#include "swcdb/ranger/db/Range.h"

#include <unordered_map>

namespace SWC { namespace Ranger {



class Column final : private std::unordered_map<rid_t, RangePtr> {
  
  public:

  typedef std::shared_ptr<Column>  Ptr;

  const ColumnCfg  cfg;

  Column(const cid_t cid, const DB::Schema& schema);

  void init(int&);

  ~Column();

  size_t size_of() const;

  size_t ranges_count();

  void get_rids(std::vector<rid_t>& rids);

  void schema_update(const DB::Schema& schema);

  void compact();

  RangePtr get_range(int &err, const rid_t rid, bool initialize=false);

  void unload(const rid_t rid, const Callback::RangeUnloaded_t& cb);

  void unload_all(std::atomic<int>& unloaded, 
                  const Callback::RangeUnloaded_t& cb);

  void unload_all(const Callback::ColumnsUnloadedPtr& cb);
  
  void remove(int &err, const rid_t rid, bool meta=true);

  void remove_all(int &err);

  bool removing();

  RangePtr get_next(size_t &idx);

  size_t release(size_t bytes=0);

  void print(std::ostream& out, bool minimal=true);

  private:

  Mutex             m_mutex;
  std::atomic<bool> m_releasing;
};

}}

#endif // swcdb_ranger_db_Column_h