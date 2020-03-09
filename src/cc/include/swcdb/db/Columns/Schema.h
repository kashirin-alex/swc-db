/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Schema_h
#define swcdb_lib_db_Columns_Schema_h

#include "swcdb/core/Serialization.h"

#include "swcdb/db/Types/RangeSeq.h"
#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Types/Encoding.h"


namespace SWC { namespace DB {


class Schema final {

  public:
  
  typedef std::shared_ptr<Schema> Ptr;

  static const int64_t NO_CID = 0;

  inline static Ptr make(
         const std::string& col_name, 
         Types::Column col_type=Types::Column::PLAIN,
         uint32_t cell_versions=1, uint32_t cell_ttl=0,
         Types::Encoding blk_encoding=Types::Encoding::DEFAULT,
         uint32_t blk_size=0, uint32_t blk_cells=0,
         uint8_t cs_replication=0, uint32_t cs_size=0, uint8_t cs_max=0,
         uint8_t compact_percent=0, 
         int64_t revision=0) {
    return std::make_shared<Schema>(
      NO_CID, col_name, col_type, 
      cell_versions, cell_ttl, 
      blk_encoding, blk_size, blk_cells, 
      cs_replication, cs_size, cs_max, compact_percent, 
      revision
    );
  }

  inline static Ptr make(
         int64_t cid, const std::string& col_name, 
         Types::Column col_type=Types::Column::PLAIN,
         uint32_t cell_versions=1, uint32_t cell_ttl=0,
         Types::Encoding blk_encoding=Types::Encoding::DEFAULT,
         uint32_t blk_size=0, uint32_t blk_cells=0,
         uint8_t cs_replication=0, uint32_t cs_size=0, uint8_t cs_max=0, uint8_t compact_percent=0, 
         int64_t revision=0) {
    return std::make_shared<Schema>(
      cid, col_name, col_type, 
      cell_versions, cell_ttl, 
      blk_encoding, blk_size, blk_cells, 
      cs_replication, cs_size, cs_max, compact_percent, 
      revision
    );
  }
  
  inline static Ptr make(int64_t cid, Ptr other, int64_t revision){
    return std::make_shared<Schema>(
      cid, other->col_name, other->col_type, 
      other->cell_versions, other->cell_ttl, 
      other->blk_encoding, other->blk_size, other->blk_cells,
      other->cs_replication, other->cs_size, other->cs_max, 
      other->compact_percent,
      revision
    );
  }
  
  inline static Ptr make(const std::string& col_name, Ptr other, 
                         int64_t revision) {
    return std::make_shared<Schema>(
      other->cid, col_name, other->col_type, 
      other->cell_versions, other->cell_ttl, 
      other->blk_encoding, other->blk_size, other->blk_cells,
      other->cs_replication, other->cs_size, other->cs_max, 
      other->compact_percent,
      revision
    );
  }

  Schema(int64_t cid, const std::string& col_name, Types::Column col_type,
         uint32_t cell_versions, uint32_t cell_ttl,
         Types::Encoding blk_encoding, uint32_t blk_size, uint32_t blk_cells,
         uint8_t cs_replication, uint32_t cs_size, uint8_t cs_max, 
         uint8_t compact_percent, 
         int64_t revision)
        : cid(cid), col_name(col_name), col_type(col_type),
          cell_versions(cell_versions), cell_ttl(cell_ttl),
          blk_encoding(blk_encoding), blk_size(blk_size), blk_cells(blk_cells), 
          cs_replication(cs_replication), cs_size(cs_size), cs_max(cs_max), 
          compact_percent(compact_percent), 
          revision(revision) {
  }
  
  Schema(const uint8_t **bufp, size_t *remainp)
    : cid(Serialization::decode_vi64(bufp, remainp)),
      col_name(Serialization::decode_vstr(bufp, remainp)),
      col_type((Types::Column)Serialization::decode_i8(bufp, remainp)),

      cell_versions(Serialization::decode_vi32(bufp, remainp)),
      cell_ttl(Serialization::decode_vi32(bufp, remainp)),

      blk_encoding((Types::Encoding)Serialization::decode_i8(bufp, remainp)),
      blk_size(Serialization::decode_vi32(bufp, remainp)),
      blk_cells(Serialization::decode_vi32(bufp, remainp)),
      cs_replication(Serialization::decode_i8(bufp, remainp)),
      cs_size(Serialization::decode_vi32(bufp, remainp)),
      cs_max(Serialization::decode_i8(bufp, remainp)),
      compact_percent(Serialization::decode_i8(bufp, remainp)),
      revision(Serialization::decode_vi64(bufp, remainp)) {
  }

  ~Schema() {}

  bool equal(const Ptr &other, bool with_rev=true) {
    return   cid == other->cid
          && col_type == other->col_type
          && cell_versions == other->cell_versions
          && cell_ttl == other->cell_ttl
          && blk_encoding == other->blk_encoding
          && blk_size == other->blk_size
          && blk_cells == other->blk_cells
          && cs_replication == other->cs_replication
          && cs_size == other->cs_size
          && cs_max == other->cs_max
          && compact_percent == other->compact_percent
          && col_name.compare(other->col_name) == 0
          && (!with_rev || revision == other->revision)
    ;
  }
  
  const size_t encoded_length() const {
    return Serialization::encoded_length_vi64(cid)
         + Serialization::encoded_length_vstr(col_name.length())
         + 1
         + Serialization::encoded_length_vi32(cell_versions)
         + Serialization::encoded_length_vi32(cell_ttl)
         + 1 
         + Serialization::encoded_length_vi32(blk_size)
         + Serialization::encoded_length_vi32(blk_cells)
         + 1
         + Serialization::encoded_length_vi32(cs_size)
         + 2
         + Serialization::encoded_length_vi64(revision);
  } 
 
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vstr(bufp, col_name.data(), col_name.length());
    Serialization::encode_i8(bufp, (uint8_t)col_type);

    Serialization::encode_vi32(bufp, cell_versions);
    Serialization::encode_vi32(bufp, cell_ttl);

    Serialization::encode_i8(bufp, (uint8_t)blk_encoding);
    Serialization::encode_vi32(bufp, blk_size);
    Serialization::encode_vi32(bufp, blk_cells);
    
    Serialization::encode_i8(bufp, cs_replication);
    Serialization::encode_vi32(bufp, cs_size);
    Serialization::encode_i8(bufp, cs_max);
    Serialization::encode_i8(bufp, compact_percent);

    Serialization::encode_vi64(bufp, revision);
  }

  const std::string to_string() const {
    std::stringstream ss;
    ss << "Schema(" 
        << "cid=" << std::to_string(cid)
        << ", col_name=" << col_name
        << ", col_type=" << Types::to_string(col_type)
        << ", cell_versions=" << std::to_string(cell_versions)
        << ", cell_ttl=" << std::to_string(cell_ttl)
        << ", blk_encoding=" << Types::to_string(blk_encoding)
        << ", blk_size=" << std::to_string(blk_size)
        << ", blk_cells=" << std::to_string(blk_cells)
        << ", cs_replication=" << std::to_string(cs_replication)
        << ", cs_size=" << std::to_string(cs_size)
        << ", cs_max=" << std::to_string(cs_max)
        << ", compact_percent=" << std::to_string(compact_percent)
        << ", revision=" << std::to_string(revision)
        << ")"
       ;
    return ss.str();
  }

  void display(std::ostream& out) const {
    out << "cid=" << std::to_string(cid)
      << " name=\"" << col_name << "\""
      << " type=" << Types::to_string(col_type) 
      << " revision=" << std::to_string(revision)
      << " compact=" << std::to_string(compact_percent)
      
      << " cell_versions=" << std::to_string(cell_versions)
      << " cell_ttl=" << std::to_string(cell_ttl)
      
      << " blk_encoding=" << Types::to_string(blk_encoding)
      << " blk_size=" << std::to_string(blk_size)
      << " blk_cells=" << std::to_string(blk_cells)
      
      << " cs_replication=" << std::to_string(cs_replication)
      << " cs_size=" << std::to_string(cs_size)
      << " cs_max=" << std::to_string(cs_max)
    ;
  }

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

#endif