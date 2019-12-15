/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_ColumnCfg_h
#define swcdb_lib_db_Columns_Rgr_ColumnCfg_h

namespace SWC { namespace DB { 


class ColumnCfg final {
  
  public:
  
  const int64_t                         cid;
  mutable Types::Column                 col_type;

  mutable std::atomic<uint32_t>         blk_size;
  mutable std::atomic<uint32_t>         blk_cells;
  mutable std::atomic<Types::Encoding>  blk_enc;
  mutable std::atomic<uint32_t>         blk_replication;

  mutable std::atomic<uint32_t>         cell_versions; 
  mutable std::atomic<uint32_t>         cell_ttl;

  const gInt32tPtr      cfg_blk_size;
  const gInt32tPtr      cfg_blk_cells;
  const gEnumExtPtr     cfg_blk_enc;

  ColumnCfg(const int64_t cid) 
            : cid(cid), col_type(Types::Column::PLAIN),
              blk_size(0), blk_cells(0), blk_enc(Types::Encoding::DEFAULT),
              cell_versions(1), cell_ttl(0), 
              cfg_blk_size(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.Range.block.size")), 
              cfg_blk_cells(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.Range.block.cells")),
              cfg_blk_enc(Env::Config::settings()->get_ptr<gEnumExt>(
                "swc.rgr.Range.block.encoding")){
  }

  void update(const Schema& schema) const {
    col_type = schema.col_type;

    blk_size = schema.blk_size;
    blk_cells = schema.blk_cells;
    blk_enc = schema.blk_encoding;
    
    cell_versions = schema.cell_versions;
    cell_ttl = schema.cell_ttl;
  }

  const uint32_t block_size() const {
    return blk_size ? blk_size.load() : cfg_blk_size->get();
  }
  const uint32_t block_cells() const {
    return blk_cells ? blk_cells.load() : cfg_blk_cells->get();
  }
  const Types::Encoding block_enc() const {
    return blk_enc != Types::Encoding::DEFAULT ? 
            blk_enc.load() : (Types::Encoding)cfg_blk_enc->get();
  }

  virtual ~ColumnCfg() { }


  const std::string to_string() const {
    std::string s("cid=");
    s.append(std::to_string(cid));
    s.append(" col_type=");
    s.append(Types::to_string(col_type));
    s.append(" blk_size=");
    s.append(std::to_string(blk_size));
    s.append(" blk_cells=");
    s.append(std::to_string(blk_cells));
    s.append(" blk_enc=");
    s.append(Types::to_string(blk_enc));
    s.append(" cell_versions=");
    s.append(std::to_string(cell_versions));
    s.append(" cell_ttl=");
    s.append(std::to_string(cell_ttl));
    s.append(")");
    return s;
  }
};

}}

#endif