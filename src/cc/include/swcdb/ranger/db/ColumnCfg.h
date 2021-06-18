/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
*/

#ifndef swcdb_ranger_db_ColumnCfg_h
#define swcdb_ranger_db_ColumnCfg_h

#include "swcdb/core/NotMovableSharedPtr.h"
#include "swcdb/db/Cells/KeyComparator.h"
#include "swcdb/db/Columns/Schema.h"

namespace SWC { namespace Ranger {


class ColumnCfg final : public Core::NotMovableSharedPtr<ColumnCfg> {
  public:

  typedef Core::NotMovableSharedPtr<ColumnCfg>    Ptr;

  const cid_t                                     cid;
  const DB::Types::Range                          range_type;
  const uint8_t                                   meta_cid;
  const DB::Types::KeySeq                         key_seq;

  mutable Core::Atomic<DB::Types::Column>         col_type;

  mutable Core::Atomic<uint32_t>                  c_versions;
  mutable Core::Atomic<uint64_t>                  c_ttl;

  mutable Core::Atomic<DB::Types::Encoder>        blk_enc;
  mutable Core::Atomic<uint32_t>                  blk_size;
  mutable Core::Atomic<uint32_t>                  blk_cells;

  mutable Core::Atomic<uint8_t>                   cs_replication;
  mutable Core::Atomic<uint32_t>                  cs_size;
  mutable Core::Atomic<uint8_t>                   cs_max;

  mutable Core::Atomic<uint8_t>                   log_rout_ratio;
  mutable Core::Atomic<uint8_t>                   log_compact;
  mutable Core::Atomic<uint8_t>                   log_preload;

  mutable Core::Atomic<uint8_t>                   compact_perc;

  mutable Core::AtomicBool                        deleting;


  SWC_CAN_INLINE
  ColumnCfg(const cid_t cid, const DB::Schema& schema)
      : cid(cid),
        range_type(
          DB::Types::SystemColumn::get_range_type(cid)),
        meta_cid(
          DB::Types::SystemColumn::get_sys_cid(schema.col_seq, range_type)),
        key_seq(schema.col_seq),
        deleting(false) {
    update(schema);
  }

  ColumnCfg(const ColumnCfg&) = delete;

  ColumnCfg(const ColumnCfg&&) = delete;

  ColumnCfg& operator=(const ColumnCfg&) = delete;

  //~ColumnCfg() { }

  void update(const DB::Schema& schema) const {
    col_type.store(schema.col_type);

    c_versions.store(schema.cell_versions);
    c_ttl.store(uint64_t(schema.cell_ttl) * 1000000000);

    blk_enc.store(schema.blk_encoding);
    blk_size.store(schema.blk_size);
    blk_cells.store(schema.blk_cells);

    cs_replication.store(schema.cs_replication);
    cs_size.store(schema.cs_size);
    cs_max.store(schema.cs_max);

    log_rout_ratio.store(schema.log_rollout_ratio);
    log_compact.store(schema.log_compact_cointervaling);
    log_preload.store(schema.log_fragment_preload);

    compact_perc.store(schema.compact_percent);
  }

  SWC_CAN_INLINE
  DB::Types::Column column_type() const {
    return col_type.load();
  }

  SWC_CAN_INLINE
  uint32_t cell_versions() const {
    return c_versions.load();
  }

  SWC_CAN_INLINE
  uint64_t cell_ttl() const {
    return c_ttl.load();
  }


  SWC_CAN_INLINE
  DB::Types::Encoder block_enc() const {
    DB::Types::Encoder tmp(blk_enc);
    return tmp == DB::Types::Encoder::DEFAULT
            ? DB::Types::Encoder(Env::Rgr::get()->cfg_blk_enc->get())
            : tmp;
  }

  SWC_CAN_INLINE
  uint32_t block_size() const {
    uint32_t tmp(blk_size);
    return tmp ? tmp : Env::Rgr::get()->cfg_blk_size->get();
  }

  SWC_CAN_INLINE
  uint32_t block_cells() const {
    uint32_t tmp(blk_cells);
    return tmp ? tmp : Env::Rgr::get()->cfg_blk_cells->get();
  }

  SWC_CAN_INLINE
  uint8_t file_replication() const {
    uint8_t tmp(cs_replication);
    return tmp ? tmp : Env::Rgr::get()->cfg_cs_replication->get();
  }

  SWC_CAN_INLINE
  uint32_t cellstore_size() const {
    uint32_t tmp(cs_size);
    return tmp ? tmp : Env::Rgr::get()->cfg_cs_sz->get();
  }

  SWC_CAN_INLINE
  uint8_t cellstore_max() const {
    uint8_t tmp(cs_max);
    return tmp ? tmp : Env::Rgr::get()->cfg_cs_max->get();
  }

  SWC_CAN_INLINE
  uint8_t log_rollout_ratio() const {
    uint8_t tmp(log_rout_ratio);
    return tmp ? tmp : Env::Rgr::get()->cfg_log_rollout_ratio->get();
  }

  SWC_CAN_INLINE
  uint8_t log_compact_cointervaling() const {
    uint8_t tmp(log_compact);
    return tmp ? tmp : Env::Rgr::get()->cfg_log_compact_cointervaling->get();
  }

  SWC_CAN_INLINE
  uint8_t log_fragment_preload() const {
    uint8_t tmp(log_preload);
    return tmp ? tmp : Env::Rgr::get()->cfg_log_fragment_preload->get();
  }

  SWC_CAN_INLINE
  uint8_t compact_percent() const {
    uint8_t tmp(compact_perc);
    return tmp ? tmp : Env::Rgr::get()->cfg_compact_percent->get();
  }

  void print(std::ostream& out) const {
    out << "col(";
    if(deleting)
      out << "DELETING ";
    out
      << "cid="   << cid
      << " seq="  << DB::Types::to_string(key_seq)
      << " type=" << DB::Types::to_string(col_type)
      << " range_type=" << DB::Types::to_string(range_type)
      << " meta_cid="   << int(meta_cid)
      << " compact="    << int(compact_perc.load()) << '%'
      << ')'
      << " cell(versions=" << c_versions.load()
      << " ttl="           << c_ttl.load()
      << ')'
      << " blk(enc="  << Core::Encoder::to_string(blk_enc)
      << " size="     << blk_size.load()
      << " cells="    << blk_cells.load()
      << ')'
      << " cs(replication=" << int(cs_replication.load())
      << " size="           << cs_size.load()
      << " max="            << int(cs_max.load())
      << ')'
      << " log(rollout="            << int(log_rout_ratio.load())
      << " compact_cointervaling="  << int(log_compact.load())
      << " preload="                << int(log_preload.load())
      << ')';
  }

  friend std::ostream& operator<<(std::ostream& out, const ColumnCfg& cfg) {
    cfg.print(out);
    return out;
  }

};


}}

#endif // swcdb_ranger_db_ColumnCfg_h