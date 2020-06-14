/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/db/client/sql/ColumnSchema.h"
#include "swcdb/core/Error.h"


namespace SWC { namespace client { namespace SQL {



ColumnSchema::ColumnSchema(const std::string& sql, DB::Schema::Ptr& schema,
                           std::string& message)
                          : Reader(sql, message), schema(schema) {
}

ColumnSchema::~ColumnSchema() {}

int ColumnSchema::parse(ColumnSchema::Func* func) {
  bool token_cmd = false;

  while(remain && !err) {
 
    if(found_space())
      continue;

    if(!token_cmd) {
      if(token_cmd = (found_token("add", 3)    || 
                      found_token("create", 6))) {
        *func = Func::CREATE;
      } else if(token_cmd = (found_token("modify", 6) || 
                              found_token("update", 6) || 
                              found_token("change", 6))) {
        *func = Func::MODIFY;
      } else if(token_cmd = (found_token("delete", 6) || 
                             found_token("remove", 6))) {
        *func = Func::DELETE;
      }
    }
    if(!token_cmd)
      expect_token("CREATE|MODIFY|DELETE", 20, token_cmd);
    break;
  }
  return parse(*func, true);
}

int ColumnSchema::parse(ColumnSchema::Func func, bool token_cmd) {
  bool token_typ = false;
  bool bracket = false;

  while(remain && !err) {
 
    if(found_space())
      continue;

    if(!token_cmd && 
       (func == Func::CREATE && (found_token("add", 3)    || 
                                 found_token("create", 6))) || 
       (func == Func::MODIFY && (found_token("modify", 6) || 
                                 found_token("update", 6) || 
                                 found_token("change", 6))) || 
       (func == Func::DELETE && (found_token("delete", 6) || 
                                 found_token("remove", 6)))
      ) {   
      token_cmd = true;
      continue;
    } 
    if(!token_cmd) {
      expect_token("CREATE|MODIFY|DELETE", 20, token_cmd);
      break;
    }
    if(!token_typ && (found_token("column", 6) || found_token("schema", 6))) {   
      token_typ = true;
      continue;
    }
    if(!token_typ) {
      expect_token("column", 7, token_typ);
      break;
    }

    if(!bracket) {
      expect_token("(", 1 , bracket);
      continue;
    }

    read_schema_options(func);
    if(err)
      break;

    expect_token(")", 1 , bracket);
    break;
  }
  return err;
}

void ColumnSchema::read_schema_options(ColumnSchema::Func func) {

  bool any = true;
  bool was_set = false;

  schema = DB::Schema::make();

  const char* stop = ", )";
    std::string buff;

  while(any && remain && !err) {
    if(found_space())
      continue;    

    if(any = found_token("cid", 3)) {
      expect_eq();
      if(err)
        return;
      int64_t v;
      read_int64_t(v, was_set = false);
      if(was_set)
        schema->cid = v;
      continue;
    }

    if(any = found_token("name", 4)) {
      expect_eq();
      if(err)
        return;
      schema->col_name.clear();
      read(schema->col_name, stop);
      continue;
    }

    if(any = found_token("type", 4)) {
      expect_eq();
      if(err)
        return;
      read(buff, stop);
      if((schema->col_type = Types::column_type_from(buff)) 
          == Types::Column::UNKNOWN) {
        error_msg(Error::SQL_PARSE_ERROR, " unknown column type");
        return;
      }
      buff.clear();
      continue;
    }
    if(any = found_token("seq", 3)) {
      expect_eq();
      if(err)
        return;
      read(buff, stop);
      if((schema->col_seq = Types::range_seq_from(buff)) 
          == Types::KeySeq::UNKNOWN) {
        error_msg(Error::SQL_PARSE_ERROR, " unknown range seq type");
        return;
      }
      buff.clear();
      continue;
    }

    if(any = found_token("revision", 8)) {
      expect_eq();
      if(err)
        return;
      read_int64_t(schema->revision, was_set);
      continue;
    }

    if(any = found_token("cell_versions", 13)) {
      expect_eq();
      if(err)
        return;
      read_uint32_t(schema->cell_versions, was_set);
      continue;
    }

    if(any = found_token("cell_ttl", 8)) {
      expect_eq();
      if(err)
        return;
      read_uint32_t(schema->cell_ttl, was_set);
      continue;
    }

    if(any = found_token("blk_encoding", 12)) {
      expect_eq();
      if(err)
        return;
      read(buff, stop);
      if((schema->blk_encoding = Types::encoding_from(buff)) 
          == Types::Encoding::DEFAULT) {
        error_msg(Error::SQL_PARSE_ERROR, " unknown blk_encoding");
        return;
      }
      buff.clear();
      continue;
    }

    if(any = found_token("blk_size", 8)) {
      expect_eq();
      if(err)
        return;
      read_uint32_t(schema->blk_size, was_set);
      continue;
    }

    if(any = found_token("blk_cells", 9)) {
      expect_eq();
      if(err)
        return;
      read_uint32_t(schema->blk_cells, was_set);
      continue;
    }

    if(any = found_token("cs_replication", 14)) {
      expect_eq();
      if(err)
        return;
      read_uint8_t(schema->cs_replication, was_set);
      continue;
    }

    if(any = found_token("cs_size", 7)) {
      expect_eq();
      if(err)
        return;
      read_uint32_t(schema->cs_size, was_set);
      continue;
    }

    if(any = found_token("cs_max", 6)) {
      expect_eq();
      if(err)
        return;
      read_uint8_t(schema->cs_max, was_set);
      continue;
    }

    if(any = found_token("compact", 7)) {
      expect_eq();
      if(err)
        return;
      read_uint8_t(schema->compact_percent, was_set);
      continue;
    }

    if(any = found_token("log_rollout", 11)) {
      expect_eq();
      if(err)
        return;
      read_uint8_t(schema->log_rollout_ratio, was_set);
      continue;
    }

    break;
  }

  if(schema->col_name.empty()) {
    error_msg(
      Error::COLUMN_SCHEMA_NAME_EMPTY, 
      "create|delete|modify action require column name"
    );
  } else if((func == Func::MODIFY) && 
            !schema->col_name.empty() && 
            schema->cid == DB::Schema::NO_CID ) {
    error_msg(
      Error::COLUMN_SCHEMA_NAME_EMPTY, 
      "modify action require column cid"
    );
    return;
  }
}


}}} // SWC::client:SQL namespace

