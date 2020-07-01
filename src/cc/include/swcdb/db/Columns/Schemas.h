/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Columns_Schemas_h
#define swcdb_db_Columns_Schemas_h

#include "swcdb/core/Mutex.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Columns/Schema.h"

#include <vector>
#include <unordered_map>
#include <shared_mutex>

namespace SWC { namespace DB {

class Schemas : private std::unordered_map<cid_t, Schema::Ptr> {
  public:

  struct Pattern {
    Pattern() {}
    Pattern(Condition::Comp comp, const std::string& value) 
            : comp(comp), value(value) { }
    Condition::Comp comp; 
    std::string     value;
  };

  Schemas();

  ~Schemas();
  
  void add(int& err, const Schema::Ptr& schema);

  void remove(cid_t cid);

  void replace(const Schema::Ptr& schema);

  Schema::Ptr get(cid_t cid);
  
  Schema::Ptr get(const std::string& name);

  void all(std::vector<Schema::Ptr>& entries);

  void matching(const std::vector<Pattern>& patterns, 
                std::vector<Schema::Ptr>& entries);

  void reset();

  protected:
  
  void _add(int& err, const Schema::Ptr& schema);

  void _remove(cid_t cid);

  void _replace(const Schema::Ptr& schema);

  Schema::Ptr _get(cid_t cid) const;
  
  Schema::Ptr _get(const std::string& name) const;

  Mutex  m_mutex;
};



}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Columns/Schemas.cc"
#endif 

#endif // swcdb_db_Columns_Schemas_h