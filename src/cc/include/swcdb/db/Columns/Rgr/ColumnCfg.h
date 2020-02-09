/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_ColumnCfg_h
#define swcdb_lib_db_Columns_Rgr_ColumnCfg_h

#include "swcdb/db/Columns/Schema.h"

namespace SWC { namespace DB { 


class ColumnCfg final {
  
  public:
  
  const int64_t                         cid;

  mutable std::atomic<Types::Column>    col_type;

  mutable std::atomic<uint32_t>         c_versions; 
  mutable std::atomic<uint32_t>         c_ttl;

  mutable std::atomic<Types::Encoding>  blk_enc;
  mutable std::atomic<uint32_t>         blk_size;
  mutable std::atomic<uint32_t>         blk_cells;

  mutable std::atomic<uint8_t>          cs_replication;
  mutable std::atomic<uint32_t>         cs_size;
  mutable std::atomic<uint8_t>          cs_max;
  mutable std::atomic<uint8_t>          compact_perc;

  mutable std::atomic<bool>             deleting;


  ColumnCfg(const int64_t cid) 
            : cid(cid), col_type(Types::Column::PLAIN),
              c_versions(1), c_ttl(0), 
              blk_enc(Types::Encoding::DEFAULT), blk_size(0), blk_cells(0),
              cs_replication(0), cs_size(0), cs_max(0), compact_perc(0), 
              deleting(false) {
  }

  ~ColumnCfg() { }

  void update(const Schema& schema) const {
    col_type = schema.col_type;

    c_versions = schema.cell_versions;
    c_ttl = schema.cell_ttl;

    blk_enc = schema.blk_encoding;
    blk_size = schema.blk_size;
    blk_cells = schema.blk_cells;
    
    cs_replication = schema.cs_replication;
    cs_size = schema.cs_size;
    cs_max = schema.cs_max;
    compact_perc = schema.compact_percent;
  }

  const Types::Column column_type() const {
    return col_type.load();
  }

  const uint32_t cell_versions() const {
    return c_versions.load();
  }

  const uint32_t cell_ttl() const {
    return c_ttl.load();
  }


  const Types::Encoding block_enc() const {
    return blk_enc != Types::Encoding::DEFAULT
            ?  blk_enc.load() 
            : (Types::Encoding)RangerEnv::get()->cfg_blk_enc->get();
  }

  const uint32_t block_size() const {
    return blk_size 
            ? blk_size.load() 
            : RangerEnv::get()->cfg_blk_size->get();
  }

  const uint32_t block_cells() const {
    return blk_cells 
            ? blk_cells.load() 
            : RangerEnv::get()->cfg_blk_cells->get();
  }


  const uint8_t file_replication() const {
    return cs_replication 
            ? cs_replication.load() 
            : RangerEnv::get()->cfg_cs_replication->get();
  }

  const uint32_t cellstore_size() const {
    return cs_size 
            ? cs_size.load() 
            : RangerEnv::get()->cfg_cs_sz->get();
  }

  const uint8_t cellstore_max() const {
    return cs_max 
            ? cs_max.load() 
            : RangerEnv::get()->cfg_cs_max->get();
  }

  const uint8_t compact_percent() const {
    return compact_perc 
            ? compact_perc.load() 
            : RangerEnv::get()->cfg_compact_percent->get();
  }


  const std::string to_string() const {
    std::string s("col(");
    if(deleting) 
      s.append("DELETING ");

    s.append("cid=");
    s.append(std::to_string(cid));
    s.append(" type=");
    s.append(Types::to_string(col_type));
    s.append(") ");

    s.append("cell(");
    s.append("versions=");
    s.append(std::to_string(c_versions));
    s.append(" ttl=");
    s.append(std::to_string(c_ttl));
    s.append(") ");

    s.append("blk(");
    s.append(" enc=");
    s.append(Types::to_string(blk_enc));
    s.append(" size=");
    s.append(std::to_string(blk_size));
    s.append(" cells=");
    s.append(std::to_string(blk_cells));
    s.append(") ");

    s.append("cs(");
    s.append("replication=");
    s.append(std::to_string(cs_replication));
    s.append("size=");
    s.append(std::to_string(cs_size));
    s.append(" max=");
    s.append(std::to_string(cs_max));
    s.append(" compact=");
    s.append(std::to_string(compact_perc));
    s.append("%) ");

    return s;
  }
};


}}

#endif