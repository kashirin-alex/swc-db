/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Columns_Schema_h
#define swcdb_db_Columns_Schema_h

#include <memory>
#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Types/Encoding.h"
#include "swcdb/db/Types/MetaColumn.h"


namespace SWC { namespace DB {


class Schema final {

  public:
  
  typedef std::shared_ptr<Schema>  Ptr;

  static constexpr const cid_t     NO_CID = 0;

  static Ptr make();

  static Ptr make(const Ptr& other);

  Schema();

  Schema(const Schema& other);
  
  Schema(const uint8_t** bufp, size_t* remainp);

  ~Schema();

  bool equal(const Ptr& other, bool with_rev=true);
  
  size_t encoded_length() const;
 
  void encode(uint8_t** bufp) const;

  std::string to_string() const;

  void display(std::ostream& out) const;

  cid_t           cid;
  std::string     col_name;

  Types::KeySeq   col_seq;
  Types::Column   col_type;

  uint32_t        cell_versions;
  uint32_t        cell_ttl;

  Types::Encoding blk_encoding;
  uint32_t        blk_size;
  uint32_t        blk_cells;

  uint8_t         cs_replication;
  uint32_t        cs_size;
  uint8_t         cs_max;
  uint8_t         log_rollout_ratio;
  uint8_t         compact_percent;

  int64_t         revision;
};

}} // SWC::DB namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Columns/Schema.cc"
#endif 

#endif // swcdb_db_Columns_Schema_h