/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_rs_Range_h
#define swcdb_lib_rs_Range_h

#include "Schema.h"

#include <mutex>
#include <memory>
#include <unordered_map>


namespace SWC {


class Range : public std::enable_shared_from_this<Range> {
  public:

  Range(int64_t cid, SchemaPtr schema, int64_t rid): 
        m_cid(cid), m_schema(schema),
        m_rid(rid), loaded(false){
  // root_path
  }
  virtual ~Range(){}

  const int64_t range_id(){
    return m_rid;
  }

  private:

  std::mutex  m_mutex;
  const int64_t  m_cid;
  SchemaPtr   m_schema;
  const int64_t  m_rid;
  bool        loaded;
};
typedef std::shared_ptr<Range> RangePtr;




}
#endif