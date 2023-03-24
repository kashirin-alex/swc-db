/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Columns_Schema_h
#define swcdb_db_Columns_Schema_h


#include "swcdb/db/Types/Encoder.h"
#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Types/SystemColumn.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB {



struct SchemaPrimitives {

  static constexpr const cid_t     NO_CID = 0;

  SWC_CAN_INLINE
  SchemaPrimitives() noexcept
      : cid(NO_CID),
        revision(0),
        cell_versions(1),
        cell_ttl(0),
        blk_size(0),
        blk_cells(0),
        cs_size(0),
        col_seq(Types::KeySeq::LEXIC),
        col_type(Types::Column::PLAIN),
        blk_encoding(Types::Encoder::DEFAULT),
        cs_replication(0),
        cs_max(0),
        log_rollout_ratio(0),
        log_compact_cointervaling(0),
        log_fragment_preload(0),
        compact_percent(0) {
  }

  SWC_CAN_INLINE
  SchemaPrimitives(const SchemaPrimitives& other) noexcept
      : cid(other.cid),
        revision(other.revision),
        cell_versions(other.cell_versions),
        cell_ttl(other.cell_ttl),
        blk_size(other.blk_size),
        blk_cells(other.blk_cells),
        cs_size(other.cs_size),
        col_seq(other.col_seq),
        col_type(other.col_type),
        blk_encoding(other.blk_encoding),
        cs_replication(other.cs_replication),
        cs_max(other.cs_max),
        log_rollout_ratio(other.log_rollout_ratio),
        log_compact_cointervaling(other.log_compact_cointervaling),
        log_fragment_preload(other.log_fragment_preload),
        compact_percent(other.compact_percent) {
  }

  SWC_CAN_INLINE
  SchemaPrimitives(const uint8_t** bufp, size_t* remainp)
    : cid(Serialization::decode_vi64(bufp, remainp)),
      revision(Serialization::decode_vi64(bufp, remainp)),
      cell_versions(Serialization::decode_vi32(bufp, remainp)),
      cell_ttl(Serialization::decode_vi32(bufp, remainp)),
      blk_size(Serialization::decode_vi32(bufp, remainp)),
      blk_cells(Serialization::decode_vi32(bufp, remainp)),
      cs_size(Serialization::decode_vi32(bufp, remainp)),
      col_seq(Types::KeySeq(Serialization::decode_i8(bufp, remainp))),
      col_type(Types::Column(Serialization::decode_i8(bufp, remainp))),
      blk_encoding(Types::Encoder(Serialization::decode_i8(bufp, remainp))),
      cs_replication(Serialization::decode_i8(bufp, remainp)),
      cs_max(Serialization::decode_i8(bufp, remainp)),
      log_rollout_ratio(Serialization::decode_i8(bufp, remainp)),
      log_compact_cointervaling(Serialization::decode_i8(bufp, remainp)),
      log_fragment_preload(Serialization::decode_i8(bufp, remainp)),
      compact_percent(Serialization::decode_i8(bufp, remainp)) {
  }

  SWC_CAN_INLINE
  uint32_t encoded_length() const noexcept {
    return  Serialization::encoded_length_vi64(cid)
          + Serialization::encoded_length_vi64(revision)
          + Serialization::encoded_length_vi32(cell_versions)
          + Serialization::encoded_length_vi32(cell_ttl)
          + Serialization::encoded_length_vi32(blk_size)
          + Serialization::encoded_length_vi32(blk_cells)
          + Serialization::encoded_length_vi32(cs_size)
          + 9;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_vi64(bufp, cid);
    Serialization::encode_vi64(bufp, revision);
    Serialization::encode_vi32(bufp, cell_versions);
    Serialization::encode_vi32(bufp, cell_ttl);
    Serialization::encode_vi32(bufp, blk_size);
    Serialization::encode_vi32(bufp, blk_cells);
    Serialization::encode_vi32(bufp, cs_size);
    Serialization::encode_i8(bufp, uint8_t(col_seq));
    Serialization::encode_i8(bufp, uint8_t(col_type));
    Serialization::encode_i8(bufp, uint8_t(blk_encoding));
    Serialization::encode_i8(bufp, cs_replication);
    Serialization::encode_i8(bufp, cs_max);
    Serialization::encode_i8(bufp, log_rollout_ratio);
    Serialization::encode_i8(bufp, log_compact_cointervaling);
    Serialization::encode_i8(bufp, log_fragment_preload);
    Serialization::encode_i8(bufp, compact_percent);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    cid = Serialization::decode_vi64(bufp, remainp);
    revision = Serialization::decode_vi64(bufp, remainp);
    cell_versions = Serialization::decode_vi32(bufp, remainp);
    cell_ttl = Serialization::decode_vi32(bufp, remainp);
    blk_size = Serialization::decode_vi32(bufp, remainp);
    blk_cells = Serialization::decode_vi32(bufp, remainp);
    cs_size = Serialization::decode_vi32(bufp, remainp);
    col_seq = Types::KeySeq(Serialization::decode_i8(bufp, remainp));
    col_type = Types::Column(Serialization::decode_i8(bufp, remainp));
    blk_encoding = Types::Encoder(Serialization::decode_i8(bufp, remainp));
    cs_replication = Serialization::decode_i8(bufp, remainp);
    cs_max = Serialization::decode_i8(bufp, remainp);
    log_rollout_ratio = Serialization::decode_i8(bufp, remainp);
    log_compact_cointervaling = Serialization::decode_i8(bufp, remainp);
    log_fragment_preload = Serialization::decode_i8(bufp, remainp);
    compact_percent = Serialization::decode_i8(bufp, remainp);
  }

  SWC_CAN_INLINE
  bool equal(const SchemaPrimitives& other,
             bool with_rev=true) const noexcept {
    return  cid == other.cid &&
            (!with_rev || revision == other.revision) &&
            cell_versions == other.cell_versions &&
            cell_ttl == other.cell_ttl &&
            blk_size == other.blk_size &&
            blk_cells == other.blk_cells &&
            cs_size == other.cs_size &&
            col_seq == other.col_seq &&
            col_type == other.col_type &&
            blk_encoding == other.blk_encoding &&
            cs_replication == other.cs_replication &&
            cs_max == other.cs_max &&
            log_rollout_ratio == other.log_rollout_ratio &&
            log_compact_cointervaling == other.log_compact_cointervaling &&
            log_fragment_preload == other.log_fragment_preload &&
            compact_percent == other.compact_percent
          ;
  }

  cid_t                 cid;
  int64_t               revision;
  uint32_t              cell_versions;
  uint32_t              cell_ttl;
  uint32_t              blk_size;
  uint32_t              blk_cells;
  uint32_t              cs_size;
  Types::KeySeq         col_seq;
  Types::Column         col_type;
  Types::Encoder        blk_encoding;
  uint8_t               cs_replication;
  uint8_t               cs_max;
  uint8_t               log_rollout_ratio;
  uint8_t               log_compact_cointervaling;
  uint8_t               log_fragment_preload;
  uint8_t               compact_percent;

};



class Schema final : public SchemaPrimitives {
  public:

  typedef std::shared_ptr<Schema>   Ptr;
  typedef Core::Vector<std::string> Tags;

  SWC_CAN_INLINE
  static Ptr make() {
    return Ptr(new Schema());
  }

  SWC_CAN_INLINE
  static Ptr make(const Ptr& other) {
    return Ptr(new Schema(*other.get()));
  }

  SWC_CAN_INLINE
  static Ptr make(const uint8_t** bufp, size_t* remainp) {
    return Ptr(new Schema(bufp, remainp));
  }

  SWC_CAN_INLINE
  Schema() noexcept : col_name(), tags() { }

  Schema(const Schema& other);

  Schema(const uint8_t** bufp, size_t* remainp);

  ~Schema() noexcept;

  bool SWC_PURE_FUNC
  equal(const Ptr& other, bool with_rev=true) const noexcept;

  uint32_t SWC_PURE_FUNC encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void display(std::ostream& out) const;

  SWC_CAN_INLINE
  std::string to_string() const {
    std::string s;
    {
      std::stringstream ss;
      display(ss);
      s = ss.str();
    }
    return s;
  }

  void print(std::ostream& out) const;

  std::string           col_name;
  Tags                  tags;
};


typedef Core::Vector<Schema::Ptr> SchemasVec;


}} // SWC::DB namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Columns/Schema.cc"
#endif

#endif // swcdb_db_Columns_Schema_h
