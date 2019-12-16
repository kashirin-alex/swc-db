/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_ColumnCfg_h
#define swcdb_lib_db_Columns_Rgr_ColumnCfg_h

namespace SWC { namespace DB { 


class ColumnCfg final {
  
  public:
  
  const int64_t                         cid;

  mutable std::atomic<Types::Column>    col_type;

  mutable std::atomic<uint32_t>         compact_perc;
  mutable std::atomic<uint32_t>         cs_size;
  mutable std::atomic<uint32_t>         cs_max;

  mutable std::atomic<uint32_t>         blk_size;
  mutable std::atomic<uint32_t>         blk_cells;
  mutable std::atomic<Types::Encoding>  blk_enc;
  mutable std::atomic<uint8_t>          blk_replica;

  mutable std::atomic<uint32_t>         c_versions; 
  mutable std::atomic<uint32_t>         c_ttl;

  mutable std::atomic<bool>             deleting;


  ColumnCfg(const int64_t cid) 
            : cid(cid), col_type(Types::Column::PLAIN),
              compact_perc(0), cs_size(0), cs_max(0), 
              blk_size(0), blk_cells(0), blk_enc(Types::Encoding::DEFAULT),
              blk_replica(0),
              c_versions(1), c_ttl(0), 
              deleting(false) {
  }

  ~ColumnCfg() { }

  void update(const Schema& schema) const {
    col_type = schema.col_type;

    blk_size = schema.blk_size;
    blk_cells = schema.blk_cells;
    blk_enc = schema.blk_encoding;
    blk_replica = schema.blk_replication;

    c_versions = schema.cell_versions;
    c_ttl = schema.cell_ttl;
  }

  const Types::Column column_type() const {
    return col_type.load();
  }

  const uint32_t compact_percent() const {
    return compact_perc 
            ? compact_perc.load() 
            : RangerEnv::get()->cfg_compact_percent->get();
  }

  const uint32_t cellstore_size() const {
    return cs_size 
            ? cs_size.load() 
            : RangerEnv::get()->cfg_cs_sz->get();
  }

  const uint32_t cellstore_max() const {
    return cs_max 
            ? cs_max.load() 
            : RangerEnv::get()->cfg_cs_max->get();
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

  const Types::Encoding block_enc() const {
    return blk_enc != Types::Encoding::DEFAULT
            ?  blk_enc.load() 
            : (Types::Encoding)RangerEnv::get()->cfg_blk_enc->get();
  }

  const uint8_t blk_replication() const {
    return blk_replica.load();
  }

  const uint32_t cell_versions() const {
    return c_versions.load();
  }

  const uint32_t cell_ttl() const {
    return c_ttl.load();
  }



  const std::string to_string() const {
    std::string s("cid=");
    s.append(std::to_string(cid));
    if(deleting) 
      s.append(" DELETING");
    s.append(" col_type=");
    s.append(Types::to_string(col_type));
    s.append(" blk_size=");
    s.append(std::to_string(blk_size));
    s.append(" blk_cells=");
    s.append(std::to_string(blk_cells));
    s.append(" blk_enc=");
    s.append(Types::to_string(blk_enc));
    s.append(" cell_versions=");
    s.append(std::to_string(c_versions));
    s.append(" cell_ttl=");
    s.append(std::to_string(c_ttl));
    return s;
  }
};

}}

#endif