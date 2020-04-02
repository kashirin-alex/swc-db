/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Columns_Schema_h
#define swcdb_db_Columns_Schema_h

#include <memory>
#include "swcdb/db/Types/RangeSeq.h"
#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Types/Encoding.h"


namespace SWC { namespace DB {


class Schema final {

  public:
  
  typedef std::shared_ptr<Schema> Ptr;

  static constexpr const int64_t NO_CID = 0;

  static Ptr make(
         const std::string& col_name, 
         Types::Column col_type=Types::Column::PLAIN,
         uint32_t cell_versions=1, uint32_t cell_ttl=0,
         Types::Encoding blk_encoding=Types::Encoding::DEFAULT,
         uint32_t blk_size=0, uint32_t blk_cells=0,
         uint8_t cs_replication=0, uint32_t cs_size=0, uint8_t cs_max=0,
         uint8_t compact_percent=0, 
         int64_t revision=0);

  static Ptr make(
         int64_t cid, const std::string& col_name, 
         Types::Column col_type=Types::Column::PLAIN,
         uint32_t cell_versions=1, uint32_t cell_ttl=0,
         Types::Encoding blk_encoding=Types::Encoding::DEFAULT,
         uint32_t blk_size=0, uint32_t blk_cells=0,
         uint8_t cs_replication=0, uint32_t cs_size=0, uint8_t cs_max=0, uint8_t compact_percent=0, 
         int64_t revision=0);
  
  static Ptr make(int64_t cid, Ptr other, int64_t revision);
  
  static Ptr make(const std::string& col_name, Ptr other, 
                         int64_t revision);

  Schema(int64_t cid, const std::string& col_name, Types::Column col_type,
         uint32_t cell_versions, uint32_t cell_ttl,
         Types::Encoding blk_encoding, uint32_t blk_size, uint32_t blk_cells,
         uint8_t cs_replication, uint32_t cs_size, uint8_t cs_max, 
         uint8_t compact_percent, 
         int64_t revision);
  
  Schema(const uint8_t **bufp, size_t *remainp);

  ~Schema();

  bool equal(const Ptr &other, bool with_rev=true);
  
  size_t encoded_length() const;
 
  void encode(uint8_t **bufp) const;

  std::string to_string() const;

  void display(std::ostream& out) const;


  Types::RangeSeq range_seq;

  const int64_t 		    cid;
  const std::string 		col_name;
  const Types::Column   col_type;

  const uint32_t 		    cell_versions;
  const uint32_t 		    cell_ttl;

  const Types::Encoding blk_encoding;
  const uint32_t        blk_size;
  const uint32_t        blk_cells;

  const uint8_t 		    cs_replication;
  const uint32_t        cs_size;
  const uint8_t         cs_max;
  const uint8_t         compact_percent;

  const int64_t         revision;
};

}} // SWC::DB namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Columns/Schema.cc"
#endif 

#endif // swcdb_db_Columns_Schema_h