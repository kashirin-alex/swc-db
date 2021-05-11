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


namespace SWC { namespace DB {


class Schema final {

  public:

  typedef std::shared_ptr<Schema>  Ptr;

  static constexpr const cid_t     NO_CID = 0;

  static Ptr make() {
    return std::make_shared<Schema>();
  }

  static Ptr make(const Ptr& other) {
    return std::make_shared<Schema>(*other.get());
  }

  Schema() noexcept;

  Schema(const Schema& other);

  Schema(const uint8_t** bufp, size_t* remainp);

  //~Schema() { }

  bool equal(const Ptr& other, bool with_rev=true) noexcept;

  uint32_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void display(std::ostream& out) const;

  std::string to_string() const;

  void print(std::ostream& out) const;

  cid_t                 cid;
  std::string           col_name;

  Types::KeySeq         col_seq;
  Types::Column         col_type;

  uint32_t              cell_versions;
  uint32_t              cell_ttl;

  Types::Encoder        blk_encoding;
  uint32_t              blk_size;
  uint32_t              blk_cells;

  uint8_t               cs_replication;
  uint32_t              cs_size;
  uint8_t               cs_max;

  uint8_t               log_rollout_ratio;
  uint8_t               log_compact_cointervaling;
  uint8_t               log_fragment_preload;

  uint8_t               compact_percent;

  int64_t               revision;
};

}} // SWC::DB namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Columns/Schema.cc"
#endif

#endif // swcdb_db_Columns_Schema_h
