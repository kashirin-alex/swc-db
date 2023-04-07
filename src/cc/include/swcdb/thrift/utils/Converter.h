/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_thrift_utils_Converter_h
#define swcdb_thrift_utils_Converter_h


#include "swcdb/db/Cells/SpecsValueSerialFields.h"


namespace SWC {  namespace Thrift {


//! The SWC-DB to & from SWC::DB Converter C++ namespace 'SWC::Thrift::Converter'
namespace Converter {


[[noreturn]]
void exception(int err, const std::string& msg = "");

void exception(int err, const std::string& msg) {
  // switch on err with exception error type
  Exception e;
  if(err == Error::CLIENT_STOPPING)
    err = Error::SERVER_SHUTTING_DOWN;
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

void set(const CellValuesSerial& value,
         DB::Cell::Serial::Value::FieldsWriter& wfields) {
  size_t len = 0;
  for(auto& fields : value) {
    if(fields.__isset.v_int64)
      len += 16;
    if(fields.__isset.v_double)
      len += 24;
    if(!fields.v_bytes.empty())
      len += fields.v_bytes.size() + 8;
    if(!fields.v_key.empty())
      len += fields.v_key.size() + 8;
    if(!fields.v_li.empty())
      len += fields.v_li.size() * 16;
    for(auto& v : fields.v_lb)
      len += v.size() + 8;
  }
  wfields.ensure(len);
  for(auto& fields : value) {
    if(fields.__isset.v_int64)
      wfields.add(fields.field_id, fields.v_int64);
    if(fields.__isset.v_double) {
      long double v(fields.v_double);
      wfields.add(fields.field_id, v);
    }
    if(!fields.v_bytes.empty())
      wfields.add(fields.field_id, fields.v_bytes);
    if(!fields.v_key.empty()) {
      DB::Cell::Serial::Value::Field_KEY field;
      field.fid = fields.field_id;
      set(fields.v_key, field.key);
      //
      wfields.add(&field);
    }
    if(!fields.v_li.empty())
      wfields.add(fields.field_id, fields.v_li);
    if(!fields.v_lb.empty())
      wfields.add(fields.field_id, fields.v_lb);
  }
}

void set(const CellValuesSerialOp& value,
         DB::Cell::Serial::Value::FieldsWriter& wfields) {
  size_t len = 0;
  for(auto& fields : value) {
    if(fields.__isset.v_int64) {
      len += 16;
    }
    if(fields.__isset.v_double) {
      len += 24;
    }
    if(fields.__isset.v_bytes) {
      len += fields.v_bytes.v.size() + 16;
    }
    if(fields.__isset.v_key) {
      len += fields.v_key.size() + 16;
    }
    if(fields.__isset.v_li) {
      len += fields.v_li.v.size() * 16;
    }
    if(fields.__isset.v_lb) {
      for(auto& u : fields.v_lb.v)
        len += u.v.size() + 16;
      len += 16;
    }
  }
  wfields.ensure(len);
  for(auto& fields : value) {
    if(fields.__isset.v_int64) {
      DB::Cell::Serial::Value::FieldUpdate_MATH ufield(
        DB::Cell::Serial::Value::FieldUpdate_MATH::OP(
          uint8_t(fields.v_int64.op)
        ),
        fields.v_int64.ctrl
      );
      wfields.add(fields.field_id, fields.v_int64.v);
      wfields.add(&ufield);
    }
    if(fields.__isset.v_double) {
      DB::Cell::Serial::Value::FieldUpdate_MATH ufield(
        DB::Cell::Serial::Value::FieldUpdate_MATH::OP(
          uint8_t(fields.v_double.op)
        ),
        fields.v_double.ctrl
      );
      long double v(fields.v_double.v);
      wfields.add(fields.field_id, v);
      wfields.add(&ufield);
    }
    if(fields.__isset.v_bytes) {
      DB::Cell::Serial::Value::FieldUpdate_LIST ufield(
        DB::Cell::Serial::Value::FieldUpdate_LIST::OP(
          uint8_t(fields.v_bytes.op)
        ),
        fields.v_bytes.pos,
        fields.v_bytes.ctrl
      );
      // Fix applicable OP
      if(ufield.is_op_by_unique())
        exception(Error::INCOMPATIBLE_OPTIONS, "not supported OP::BY_UNIQUE");
      if(ufield.is_op_by_cond())
        exception(Error::INCOMPATIBLE_OPTIONS, "not supported OP::BY_COND");
      if(ufield.is_op_by_idx())
        exception(Error::INCOMPATIBLE_OPTIONS, "not supported OP::BY_INDEX");

      wfields.add(
        fields.field_id,
        ufield.is_delete_field() ? std::string() : fields.v_bytes.v
      );
      wfields.add(&ufield);
    }
    if(fields.__isset.v_key) {
      DB::Cell::Serial::Value::Field_KEY field;
      field.fid = fields.field_id;
      set(fields.v_key, field.key);
      wfields.add(&field);
    }
    if(fields.__isset.v_li) {
      DB::Cell::Serial::Value::FieldUpdate_LIST_INT64 ufield(
        DB::Cell::Serial::Value::FieldUpdate_LIST_INT64::OP(
          uint8_t(fields.v_li.op)
        ),
        fields.v_li.pos,
        fields.v_li.ctrl
      );

      Core::Vector<int64_t> items;
      if(!ufield.is_delete_field()) {
        items.reserve(fields.v_li.v.size());
        for(auto& u : fields.v_li.v) {
          if(ufield.is_op_by_idx()) {
            ufield.add_item(
              u.pos,
              DB::Cell::Serial::Value::FieldUpdate_MATH::OP(
                uint8_t(u.op)
              ),
              u.ctrl
            );
          } else if(ufield.is_op_by_unique()) {
            ufield.add_item(
              0,
              DB::Cell::Serial::Value::FieldUpdate_MATH::OP(
                uint8_t(u.op)
              ),
              u.ctrl
            );
          } else if(ufield.is_op_by_cond()) {
            ufield.add_item(
              uint32_t(Condition::Comp(uint8_t(u.comp))),
              DB::Cell::Serial::Value::FieldUpdate_MATH::OP(
                uint8_t(u.op)
              ),
              u.ctrl
            );
          }
          items.push_back(u.v);
        }
      }
      wfields.add(fields.field_id, items);
      wfields.add(&ufield);
    }
    if(fields.__isset.v_lb) {
      DB::Cell::Serial::Value::FieldUpdate_LIST_BYTES ufield(
        DB::Cell::Serial::Value::FieldUpdate_LIST_BYTES::OP(
          uint8_t(fields.v_lb.op)
        ),
        fields.v_lb.pos,
        fields.v_lb.ctrl
      );

      Core::Vector<std::string> items;
      if(!ufield.is_delete_field()) {
        items.reserve(fields.v_lb.v.size());
        for(auto& u : fields.v_lb.v) {
          if(ufield.is_op_by_idx()) {
            ufield.add_item(
              u.pos,
              DB::Cell::Serial::Value::FieldUpdate_LIST_BYTES::OP(
                uint8_t(u.op)
              ),
              0,
              u.ctrl
            );
          } else if(ufield.is_op_by_unique()) {
            ufield.add_item(
              0,
              DB::Cell::Serial::Value::FieldUpdate_LIST_BYTES::OP(
                uint8_t(u.op)
              ),
              0,
              u.ctrl
            );
          } else if(ufield.is_op_by_cond()) {
            ufield.add_item(
              uint32_t(Condition::Comp(uint8_t(u.comp))),
              DB::Cell::Serial::Value::FieldUpdate_LIST_BYTES::OP(
                uint8_t(u.op)
              ),
              0,
              u.ctrl
            );
          }
          items.push_back(u.v);
        }
      }
      wfields.add(fields.field_id, items);
      wfields.add(&ufield);
    }
  }
}

void set(const SpecKey& spec, DB::Specs::Key& dbspec) {
  dbspec.reserve(spec.size());
  for(const auto& fraction : spec) {
    if(uint8_t(fraction.comp) > 0x8 && uint8_t(fraction.comp) < 0x13) {
      std::string msg("Key ext-fraction-comp(");
      msg.append(Condition::to_string(Condition::Comp(fraction.comp)));
      msg.append(")");
      exception(Error::INCOMPATIBLE_OPTIONS, msg);
    }
    dbspec.add(fraction.f, Condition::Comp(fraction.comp));
  }
}

void set(const SpecTimestamp& spec, DB::Specs::Timestamp& dbspec) {
  dbspec.set(spec.ts, Condition::Comp(spec.comp));
}

void set(const SpecUpdateOP& spec, DB::Specs::UpdateOP& dbspec) {
  dbspec.set_op(uint8_t(spec.op));
  if(spec.__isset.pos && dbspec.has_pos())
    dbspec.set_pos(spec.pos);
}


// SpecIntervalPlain

void set(const SpecValuePlain& spec, DB::Specs::Value& dbspec) {
  dbspec.set(spec.v, Condition::Comp(spec.comp));
}

void set(const SpecValuesPlain& spec, DB::Specs::Values& dbspec) {
  for(const auto& value : spec)
    set(value, dbspec.add());
}

void set(const SpecIntervalPlain& intval, DB::Specs::Interval& dbintval) {
  if(!intval.range_begin.empty())
    set(intval.range_begin, dbintval.range_begin);

  if(!intval.range_end.empty())
    set(intval.range_end, dbintval.range_end);

  if(!intval.offset_key.empty())
    set(intval.offset_key, dbintval.offset_key);

  if(intval.__isset.offset_rev)
    dbintval.offset_rev = intval.offset_rev;

  dbintval.key_intervals.resize(intval.key_intervals.size());
  size_t n = 0;
  for(const auto& key_intval : intval.key_intervals) {
    if(!key_intval.start.empty())
      set(key_intval.start, dbintval.key_intervals[n].start);
    if(!key_intval.finish.empty())
      set(key_intval.finish, dbintval.key_intervals[n].finish);
    ++n;
  }

  if(intval.__isset.ts_start)
    set(intval.ts_start, dbintval.ts_start);

  if(intval.__isset.ts_finish)
    set(intval.ts_finish, dbintval.ts_finish);

  if(intval.__isset.flags)
    set(intval.flags, dbintval.flags);

  if(!intval.values.empty())
    set(intval.values, dbintval.values);

  if(intval.options & SpecIntervalOptions::type::UPDATING) {
    if(intval.__isset.updating) {
      dbintval.set_opt__updating();

      DB::Specs::UpdateOP update_op;
      if(intval.updating.__isset.update_op)
        set(intval.updating.update_op, update_op);

      dbintval.updating.reset(new DB::Specs::IntervalUpdate(
        intval.updating.__isset.encoder
          ? DB::Types::Encoder(uint8_t(intval.updating.encoder))
          : DB::Types::Encoder::DEFAULT,
        intval.updating.v,
        intval.updating.__isset.ts
          ? intval.updating.ts
          : DB::Cells::TIMESTAMP_AUTO,
        update_op
      ));
    }
  } else if(intval.options & SpecIntervalOptions::type::DELETING) {
    dbintval.set_opt__deleting();
  }
}


// SpecIntervalCounter

void set(const SpecValueCounter& spec, DB::Specs::Value& dbspec) {
  dbspec.set_counter(spec.v, Condition::Comp(spec.comp));
}

void set(const SpecValuesCounter& spec, DB::Specs::Values& dbspec) {
  for(const auto& value : spec)
    set(value, dbspec.add());
}

void set(const SpecIntervalCounter& intval, DB::Specs::Interval& dbintval) {
  if(!intval.range_begin.empty())
    set(intval.range_begin, dbintval.range_begin);

  if(!intval.range_end.empty())
    set(intval.range_end, dbintval.range_end);

  if(!intval.offset_key.empty())
    set(intval.offset_key, dbintval.offset_key);

  if(intval.__isset.offset_rev)
    dbintval.offset_rev = intval.offset_rev;

  dbintval.key_intervals.resize(intval.key_intervals.size());
  size_t n = 0;
  for(const auto& key_intval : intval.key_intervals) {
    if(!key_intval.start.empty())
      set(key_intval.start, dbintval.key_intervals[n].start);
    if(!key_intval.finish.empty())
      set(key_intval.finish, dbintval.key_intervals[n].finish);
    ++n;
  }

  if(intval.__isset.ts_start)
    set(intval.ts_start, dbintval.ts_start);

  if(intval.__isset.ts_finish)
    set(intval.ts_finish, dbintval.ts_finish);

  if(intval.__isset.flags)
    set(intval.flags, dbintval.flags);

  if(!intval.values.empty())
    set(intval.values, dbintval.values);

  if(intval.options & SpecIntervalOptions::type::UPDATING) {
    if(intval.__isset.updating) {
      dbintval.set_opt__updating();
      DB::Cells::Cell dbcell;
      dbcell.set_counter(
        uint8_t(intval.updating.op),
        intval.updating.v,
        dbintval.values.col_type
      );

      DB::Specs::UpdateOP update_op;
      if(intval.updating.__isset.update_op)
        set(intval.updating.update_op, update_op);
      dbintval.updating.reset(new DB::Specs::IntervalUpdate(
        DB::Types::Encoder::DEFAULT,
        dbcell.value,
        dbcell.vlen,
        intval.updating.__isset.ts
          ? intval.updating.ts
          : DB::Cells::TIMESTAMP_AUTO,
        update_op,
        false
      ));
      dbcell.value = nullptr;
      dbcell.vlen = 0;
    }
  } else if(intval.options & SpecIntervalOptions::type::DELETING) {
    dbintval.set_opt__deleting();
  }
}



// SpecIntervalSerial

void set(const SpecValueSerial& spec, DB::Specs::Value& dbspec) {
  DB::Specs::Serial::Value::Fields dbfields;

  for(const auto& fields : spec.fields) {
    if(fields.__isset.spec_int64) {
      dbfields.add(
        DB::Specs::Serial::Value::Field_INT64::make(
          fields.field_id,
          Condition::Comp(uint8_t(fields.spec_int64.comp)),
          fields.spec_int64.v));
    }
    if(fields.__isset.spec_double) {
      dbfields.add(
        DB::Specs::Serial::Value::Field_DOUBLE::make(
          fields.field_id,
          Condition::Comp(uint8_t(fields.spec_double.comp)),
          fields.spec_double.v));
    }
    if(!fields.spec_bytes.v.empty()) {
      dbfields.add(
        DB::Specs::Serial::Value::Field_BYTES::make(
          fields.field_id,
          Condition::Comp(uint8_t(fields.spec_bytes.comp)),
          fields.spec_bytes.v));
    }
    if(!fields.spec_key.v.empty()) {
      auto field = DB::Specs::Serial::Value::Field_KEY::make(
        fields.field_id, DB::Types::KeySeq(uint8_t(fields.spec_key.seq)));
      Converter::set(fields.spec_key.v, field->key);
      dbfields.add(std::move(field));
    }
    if(!fields.spec_li.v.empty()) {
      auto field = DB::Specs::Serial::Value::Field_LIST_INT64::make(
        fields.field_id, Condition::Comp(uint8_t(fields.spec_li.comp)));
      field->items.resize(fields.spec_li.v.size());
      size_t i = 0;
      for(const auto& data : fields.spec_li.v) {
        auto& f = field->items[i];
        f.comp = Condition::Comp(uint8_t(data.comp));
        f.value = data.v;
        ++i;
      }
      dbfields.add(std::move(field));
    }
    if(!fields.spec_lb.v.empty()) {
      auto field = DB::Specs::Serial::Value::Field_LIST_BYTES::make(
        fields.field_id, Condition::Comp(uint8_t(fields.spec_lb.comp)));
      field->items.resize(fields.spec_lb.v.size());
      size_t i = 0;
      for(const auto& data : fields.spec_lb.v) {
        auto& f = field->items[i];
        f.comp = Condition::Comp(uint8_t(data.comp));
        f.value = data.v;
        ++i;
      }
      dbfields.add(std::move(field));
    }
  }

  dbspec.comp = Condition::Comp(uint8_t(spec.comp));
  dbfields.encode(dbspec);
}

void set(const SpecValuesSerial& spec, DB::Specs::Values& dbspec) {
  for(const auto& value : spec)
    set(value, dbspec.add());
}

void set(const SpecIntervalSerial& intval, DB::Specs::Interval& dbintval) {
  if(!intval.range_begin.empty())
    set(intval.range_begin, dbintval.range_begin);

  if(!intval.range_end.empty())
    set(intval.range_end, dbintval.range_end);

  if(!intval.offset_key.empty())
    set(intval.offset_key, dbintval.offset_key);

  if(intval.__isset.offset_rev)
    dbintval.offset_rev = intval.offset_rev;

  dbintval.key_intervals.resize(intval.key_intervals.size());
  size_t n = 0;
  for(const auto& key_intval : intval.key_intervals) {
    if(!key_intval.start.empty())
      set(key_intval.start, dbintval.key_intervals[n].start);
    if(!key_intval.finish.empty())
      set(key_intval.finish, dbintval.key_intervals[n].finish);
    ++n;
  }

  if(intval.__isset.ts_start)
    set(intval.ts_start, dbintval.ts_start);

  if(intval.__isset.ts_finish)
    set(intval.ts_finish, dbintval.ts_finish);

  if(intval.__isset.flags)
    set(intval.flags, dbintval.flags);

  if(!intval.values.empty())
    set(intval.values, dbintval.values);

  if(intval.options & SpecIntervalOptions::type::UPDATING) {
    if(intval.__isset.updating) {
      dbintval.set_opt__updating();

      DB::Specs::UpdateOP update_op;
      if(intval.updating.__isset.update_op)
        set(intval.updating.update_op, update_op);

      DB::Cell::Serial::Value::FieldsWriter wfields;
      if(update_op.get_op() == DB::Specs::UpdateOP::SERIAL) {
        set(intval.updating.v_op, wfields);
      } else {
        set(intval.updating.v, wfields);
      }
      dbintval.updating.reset(new DB::Specs::IntervalUpdate(
        intval.updating.__isset.encoder
          ? DB::Types::Encoder(uint8_t(intval.updating.encoder))
          : DB::Types::Encoder::DEFAULT,
        wfields.base,
        wfields.fill(),
        intval.updating.__isset.ts
          ? intval.updating.ts
          : DB::Cells::TIMESTAMP_AUTO,
        update_op,
        true
      ));
    }
  } else if(intval.options & SpecIntervalOptions::type::DELETING) {
    dbintval.set_opt__deleting();
  }
}


void set(const DB::Schema::Ptr& dbschema, Schema& schema) {
  schema.__set_cid(dbschema->cid);
  schema.__set_col_name(dbschema->col_name);
  schema.col_tags.assign(dbschema->tags.cbegin(), dbschema->tags.cend());

  schema.__set_col_seq(
    KeySeq::type(uint8_t(dbschema->col_seq)));
  schema.__set_col_type(
    ColumnType::type(uint8_t(dbschema->col_type)));

  schema.__set_cell_versions(dbschema->cell_versions);
  schema.__set_cell_ttl(dbschema->cell_ttl);

  schema.__set_blk_encoding(
    EncodingType::type(uint8_t(dbschema->blk_encoding)));
  schema.__set_blk_size(dbschema->blk_size);
  schema.__set_blk_cells(dbschema->blk_cells);

  schema.__set_cs_replication(dbschema->cs_replication);
  schema.__set_cs_size(dbschema->cs_size);
  schema.__set_cs_max(dbschema->cs_max);

  schema.__set_log_rollout_ratio(dbschema->log_rollout_ratio);
  schema.__set_log_compact_cointervaling(dbschema->log_compact_cointervaling);
  schema.__set_log_fragment_preload(dbschema->log_fragment_preload);

  schema.__set_compact_percent(dbschema->compact_percent);

  schema.__set_revision(dbschema->revision);
}

void set(const Schema& schema, DB::Schema::Ptr& dbschema) {
  if(schema.__isset.cid)
    dbschema->cid = schema.cid;
  if(schema.__isset.col_name)
    dbschema->col_name = schema.col_name;
  dbschema->tags.assign(schema.col_tags.cbegin(), schema.col_tags.cend());

  if(schema.__isset.col_seq)
    dbschema->col_seq = DB::Types::KeySeq(uint8_t(schema.col_seq));
  if(schema.__isset.col_type)
    dbschema->col_type = DB::Types::Column(uint8_t(schema.col_type));

  if(schema.__isset.cell_versions)
    dbschema->cell_versions = schema.cell_versions;
  if(schema.__isset.cell_ttl)
    dbschema->cell_ttl = schema.cell_ttl;

  if(schema.__isset.blk_encoding)
    dbschema->blk_encoding = DB::Types::Encoder(uint8_t(schema.blk_encoding));
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
  if(schema.__isset.log_compact_cointervaling)
    dbschema->log_compact_cointervaling = schema.log_compact_cointervaling;
  if(schema.__isset.log_fragment_preload)
    dbschema->log_fragment_preload = schema.log_fragment_preload;

  if(schema.__isset.compact_percent)
    dbschema->compact_percent = schema.compact_percent;

  if(schema.__isset.revision)
    dbschema->revision = schema.revision;
}

CellValueSerial& get_fields(int32_t fid, CellValuesSerial& values) {
  for(auto& fields : values) {
    if(fid == fields.field_id)
      return fields;
  }
  auto& fields = values.emplace_back();
  fields.field_id = fid;
  return fields;
}

void set(const DB::Cells::Cell& dbcell, CellValuesSerial& values) {
  if(!dbcell.vlen)
    return;

  StaticBuffer v;
  dbcell.get_value(v);
  const uint8_t* ptr = v.base;
  size_t remain = v.size;

  while(remain) {
    switch(DB::Cell::Serial::Value::read_type(&ptr, &remain)) {

      case DB::Cell::Serial::Value::Type::INT64: {
        DB::Cell::Serial::Value::Field_INT64 dbfield(&ptr, &remain);
        auto& fields = get_fields(dbfield.fid, values);
        fields.__set_v_int64(dbfield.value);
        break;
      }
      case DB::Cell::Serial::Value::Type::DOUBLE: {
        DB::Cell::Serial::Value::Field_DOUBLE dbfield(&ptr, &remain);
        auto& fields = get_fields(dbfield.fid, values);
        fields.__set_v_double(dbfield.value);
        break;
      }
      case DB::Cell::Serial::Value::Type::BYTES: {
        DB::Cell::Serial::Value::Field_BYTES dbfield(&ptr, &remain, false);
        auto& fields = get_fields(dbfield.fid, values);
        fields.__isset.v_bytes = true;
        dbfield.convert_to(fields.v_bytes);
        break;
      }
      case DB::Cell::Serial::Value::Type::KEY: {
        DB::Cell::Serial::Value::Field_KEY dbfield(&ptr, &remain, false);
        auto& fields = get_fields(dbfield.fid, values);
        fields.__isset.v_key = true;
        dbfield.key.convert_to(fields.v_key);
        break;
      }
      case DB::Cell::Serial::Value::Type::LIST_INT64: {
        DB::Cell::Serial::Value::Field_LIST_INT64 dbfield(&ptr, &remain, false);
        auto& fields = get_fields(dbfield.fid, values);
        fields.__isset.v_li = true;
        dbfield.convert_to(fields.v_li);
        break;
      }
      case DB::Cell::Serial::Value::Type::LIST_BYTES: {
        DB::Cell::Serial::Value::Field_LIST_BYTES dbfield(&ptr, &remain, false);
        auto& fields = get_fields(dbfield.fid, values);
        fields.__isset.v_lb = true;
        dbfield.convert_to(fields.v_lb);
        break;
      }
      default: {
        exception(Error::SERIALIZATION_INPUT_OVERRUN, "Serial Cell Value");
        remain = 0;
        break;
      }
    }
  }
}




}

}}

#endif // swcdb_thrift_utils_Converter_h
