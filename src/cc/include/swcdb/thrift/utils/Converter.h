/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_thrift_utils_Converter_h
#define swcdb_thrift_utils_Converter_h


namespace SWC {  namespace Thrift { namespace Converter {

void exception(int err, const std::string& msg = "") {
  Exception e;
  e.__set_code(err);
  e.__set_message(msg + " - " + Error::get_text(err));
  SWC_LOG_OUT(LOG_DEBUG, e.printTo(SWC_LOG_OSTREAM); );
  throw e;
}

void set(const SpecFlags& flags, DB::Specs::Flags& dbspec) {
  if(flags.__isset.limit)
    dbspec.limit = flags.limit;
  if(flags.__isset.offset)
    dbspec.offset = flags.offset;
  if(flags.__isset.max_versions)
    dbspec.max_versions = flags.max_versions;
  if(flags.__isset.max_buffer)
    dbspec.max_buffer = flags.max_buffer;
  if(flags.__isset.options)
    dbspec.options = flags.options;
}

void set(const Key& key, DB::Cell::Key& dbkey) {
  dbkey.read(key);
}

void set(const SpecKey& spec, DB::Specs::Key& dbspec) {
  for(auto& fraction : spec) {
    if((uint8_t)fraction.comp > 0x8) {
      std::string msg("Key ext-fraction-comp(");
      msg.append(Condition::to_string((Condition::Comp)fraction.comp, true));
      msg += ')';
      exception(Error::INCOMPATIBLE_OPTIONS, msg);
    }
    dbspec.add(fraction.f, (Condition::Comp)fraction.comp);
  }
}

void set(const SpecValue& spec, DB::Specs::Value& dbspec) {
  dbspec.set(spec.v, (Condition::Comp)spec.comp);
}

void set(const SpecTimestamp& spec, DB::Specs::Timestamp& dbspec) {
  dbspec.set(spec.ts, (Condition::Comp)spec.comp);
}

void set(const SpecInterval& intval, DB::Specs::Interval& dbintval) {
  if(intval.__isset.range_begin)
    set(intval.range_begin, dbintval.range_begin);

  if(intval.__isset.range_end)
    set(intval.range_end, dbintval.range_end);

  if(intval.__isset.range_offset)
    set(intval.range_offset, dbintval.range_offset);

  if(intval.__isset.offset_key)
    set(intval.offset_key, dbintval.offset_key);

  if(intval.__isset.offset_rev)
    dbintval.offset_rev = intval.offset_rev;

  if(intval.__isset.key_start)
    set(intval.key_start, dbintval.key_start);

  if(intval.__isset.key_finish)
    set(intval.key_finish, dbintval.key_finish);

  if(intval.__isset.value)
    set(intval.value, dbintval.value);

  if(intval.__isset.ts_start)
    set(intval.ts_start, dbintval.ts_start);

  if(intval.__isset.ts_finish)
    set(intval.ts_finish, dbintval.ts_finish);

  if(intval.__isset.flags)
    set(intval.flags, dbintval.flags);
}

void set(const DB::Schema::Ptr& dbschema, Schema& schema) {
  schema.__set_cid(dbschema->cid);
  schema.__set_col_name(dbschema->col_name);

  schema.__set_col_seq(
    (KeySeq::type)(uint8_t)dbschema->col_seq);
  schema.__set_col_type(
    (ColumnType::type)(uint8_t)dbschema->col_type);

  schema.__set_cell_versions(dbschema->cell_versions);
  schema.__set_cell_ttl(dbschema->cell_ttl);

  schema.__set_blk_encoding(
    (EncodingType::type)(uint8_t)dbschema->blk_encoding);
  schema.__set_blk_size(dbschema->blk_size);
  schema.__set_blk_cells(dbschema->blk_cells);

  schema.__set_cs_replication(dbschema->cs_replication);
  schema.__set_cs_size(dbschema->cs_size);
  schema.__set_cs_max(dbschema->cs_max);
      
  schema.__set_log_rollout_ratio(dbschema->log_rollout_ratio);
      
  schema.__set_compact_percent(dbschema->compact_percent);

  schema.__set_revision(dbschema->revision);
}

void set(const Schema& schema, DB::Schema::Ptr& dbschema) {
  if(schema.__isset.cid)
    dbschema->cid = schema.cid;
  if(schema.__isset.col_name)
    dbschema->col_name = schema.col_name;

  if(schema.__isset.col_seq)
    dbschema->col_seq = (DB::Types::KeySeq)(uint8_t)schema.col_seq;
  if(schema.__isset.col_type)
    dbschema->col_type = (DB::Types::Column)(uint8_t)schema.col_type;
    
  if(schema.__isset.cell_versions)
    dbschema->cell_versions = schema.cell_versions;
  if(schema.__isset.cell_ttl)
    dbschema->cell_ttl = schema.cell_ttl;
  
  if(schema.__isset.blk_encoding)
    dbschema->blk_encoding = (DB::Types::Encoder)(uint8_t)schema.blk_encoding;
  if(schema.__isset.blk_size)
    dbschema->blk_size = schema.blk_size;
  if(schema.__isset.blk_cells)
    dbschema->blk_cells = schema.blk_cells;
    
  if(schema.__isset.cs_replication)
    dbschema->cs_replication = schema.cs_replication;
  if(schema.__isset.cs_size)
    dbschema->cs_size = schema.cs_size;
  if(schema.__isset.cs_max)
    dbschema->cs_max = schema.cs_max;
      
  if(schema.__isset.log_rollout_ratio)
    dbschema->log_rollout_ratio = schema.log_rollout_ratio;
      
  if(schema.__isset.compact_percent)
    dbschema->compact_percent = schema.compact_percent;

  if(schema.__isset.revision)
    dbschema->revision = schema.revision;
}

}






}}

#endif // swcdb_thrift_utils_Converter_h
