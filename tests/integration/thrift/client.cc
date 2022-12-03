/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/thrift/client/Client.h"
#include <iostream>
#include <cassert>

#undef assert
#define assert(_e_) \
  if(!(_e_)) throw std::runtime_error(#_e_);


const std::string column_pre("thrift-client-test");
const int num_columns = 5;
const int num_cells = 10;
const int num_fractions = 26; // a=1 and z=26
const size_t total_cells = num_columns*num_cells*num_fractions;
size_t batches = 1; // changed on test



namespace  SWC { namespace Thrift {
namespace Test {


/* INPUTS */

int64_t now_ns() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string column_name(int c) {
  return column_pre+"-"+std::to_string(c);
}

std::string key(int i, int f) {
  std::string key;
  for(uint8_t n=97;n<97+f;++n) {
    key += char(n);
    key.append(std::to_string(i));
    if(n != 96+f)
      key += ',';
  }
  return key;
}

void key(int i, int f, Key& key) {
  for(uint8_t n=97;n<97+f;++n) {
    std::string& fraction = key.emplace_back();
    fraction += char(n);
    fraction.append(std::to_string(i));
  }
}


template<ColumnType::type ColumnT>
std::string cell_value(int c, int i, int f, int batch);
template<>
std::string cell_value<ColumnType::type::PLAIN>(int c, int i, int f, int batch) {
  return "DATAOF:"
        + column_name(c)
        + "-"
        + std::to_string(i)
        + "/"
        + std::to_string(f)
        + "/"
        + std::to_string(batch)
        ;
}
template<>
std::string cell_value<ColumnType::type::COUNTER_I64>(int c, int i, int f, int batch) {
  (void)c;
  (void)i;
  (void)f;
  (void)batch;
  return "+1";
}
template<>
std::string cell_value<ColumnType::type::SERIAL>(int c, int i, int f, int batch) {
  return "[0:B:"+ column_name(c) + 
          ",0:I:" + std::to_string(i) +
          ",1:I:" + std::to_string(f) +
          ",2:I:" + std::to_string(batch) + "]";
}


/* OUTPUT */
void print(const Cells& container) {
  std::cout << "cells.size=" << container.plain_cells.size() << std::endl;
  container.printTo(std::cout << " ");
  std::cout << std::endl;
}

void print(const CCells& columns) {
  std::cout << "columns.size=" << columns.size() << std::endl;
  for(auto& col : columns) {
    std::cout << "column=" << col.first
              << " cells.size=" << col.second.cells.size()
              << " serial_cells.size=" << col.second.serial_cells.size()
              << std::endl;
    col.second.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}

void print(const KCells& keys) {
  std::cout << "keys.size=" << keys.size() << std::endl;
  for(auto& key_cells : keys) {
    key_cells.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}

void print(FCells* fcells, Key& key) {
  for(auto& f : fcells->f) {
    key.emplace_back(f.first);
    std::cout << "fraction='" << f.first << "'"
              << " cells.size=" << f.second.cells.size()
              << " serial_cells.size=" << f.second.serial_cells.size()
              << " key.size=" << key.size() << " ";
    for(auto& fraction : key)
      std::cout << "/" << fraction;
    std::cout << std::endl;
    f.second.printTo(std::cout << " ");
    std::cout << std::endl;
    print(&f.second, key);
    key.clear();
  }
}

void print(FCells& cells) {
  cells.printTo(std::cout);
  std::cout << std::endl;

  Key key;
  print(&cells, key);
}


/* SQL METHODS */

template<ColumnType::type ColumnT>
void sql_mng_and_list_column(Client& client) {
  std::cout << std::endl << "test: sql_mng_column: " << std::endl;

  Schemas schemas;
  try {
    client.sql_list_columns(
      schemas,
      "get schema col-test-create-1"
    );
    if(schemas.size()) {
      std::string sql_delete("delete column(name='col-test-create-1' cid=");
      sql_delete.append(std::to_string(schemas.front().cid));
      sql_delete.append(")");
      client.sql_mng_column(sql_delete);
    }
  } catch(Exception& e) {
    e.printTo(std::cout << "OK: ");
    std::cout << std::endl;
  }

  std::string sql_create = "create column(name='col-test-create-1' cell_ttl=123456";
  switch(ColumnT) {
    case ColumnType::type::PLAIN:
      sql_create += ')';
      break;
    case ColumnType::type::COUNTER_I64:
      sql_create.append(" type=COUNTER_I64");
      break;
    case ColumnType::type::SERIAL: 
      sql_create.append(" type=SERIAL");
      break;
  }
  sql_create += ')';

  client.sql_mng_column(sql_create);
  schemas.clear();
  client.sql_list_columns(
    schemas,
    "get schema col-test-create-1"
  );
  assert(schemas.size() == 1);

  std::string sql_modify = "modify column(name='col-test-create-1' cid=";
  sql_modify.append(std::to_string(schemas.back().cid));
  switch(ColumnT) {
    case ColumnType::type::PLAIN:
      sql_modify.append(" cell_ttl=123456789 type=PLAIN");
      break;
    case ColumnType::type::COUNTER_I64:
      sql_modify.append(" cell_ttl=123456789 type=COUNTER_I64");
      break;
    case ColumnType::type::SERIAL: 
      sql_modify.append(" cell_ttl=123456789 type=SERIAL");
      break;
  }
  sql_modify += ')';

  client.sql_mng_column(sql_modify);

  schemas.clear();
  client.sql_list_columns(
    schemas,
    "get schema col-test-create-1"
  );
  assert(schemas.size() == 1);
  assert(schemas.back().cell_ttl == 123456789);
  assert(schemas.back().col_type == ColumnT);

  std::string sql_delete("delete column(name='col-test-create-1' cid=");
  sql_delete.append(std::to_string(schemas.front().cid));
  sql_delete.append(")");
  client.sql_mng_column(sql_delete);

  try {
    client.sql_list_columns(
      schemas,
      "get schema col-test-create-1"
    );
    assert(false);
  } catch(Exception& e) {
    e.printTo(std::cout << "OK: ");
    std::cout << std::endl;
  }
}

template<ColumnType::type ColumnT>
void sql_list_columns_all(Client& client) {
  std::cout << std::endl << "test: sql_list_columns_all: " << std::endl;

  Schemas schemas;
  client.sql_list_columns(
    schemas,
    "list columns"
  );
  assert(schemas.size() >= 3);
  std::cout << std::endl << "schemas.size=" << schemas.size() << std::endl;
  for(auto& schema : schemas) {
    schema.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}

template<ColumnType::type ColumnT>
void sql_compact_columns(Client& client) {
  std::cout << std::endl << "test: sql_compact_columns all: " << std::endl;
  CompactResults res;

  bool err = false;
  size_t count = 0;
  do {
    res.clear();
    client.sql_compact_columns(res,  "compact columns ");
    assert(res.size() >= 2);
    std::cout << std::endl
              << "CompactResults.size=" << res.size()
              << " waited=" << count << "ms" << std::endl;
    for(auto& r : res) {
      r.printTo(std::cout << " ");
      std::cout << std::endl;
      if((err = r.err))
        break;
    }
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    ++count;
  } while(err);

  std::cout << std::endl << "test: sql_compact_columns test-columns: " << std::endl;
  res.clear();
  std::string sql("compact columns [");
  for(auto c=1; c <= num_columns; ++c) {
    sql.append(column_name(c));
    if(c != num_columns)
       sql += ',';
  }
  sql += ']';
  client.sql_compact_columns(res, sql);
  assert(res.size() == num_columns);

  std::cout << std::endl << "CompactResults.size=" << res.size() << std::endl;
  for(auto& r : res) {
    assert(!r.err);
    r.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}


template<ColumnType::type ColumnT>
void sql_delete_test_column(Client& client) {
  std::cout << std::endl << "test: sql_delete_test_column: " << std::endl;
  for(auto c=1; c <= num_columns; ++c) {
    try{
      Schemas schemas;
      client.sql_list_columns(
        schemas,
        "get schema " + column_name(c)
      );
      if(!schemas.empty())
        client.sql_mng_column(
          "delete column(cid="+std::to_string(schemas.back().cid)+" name='"+schemas.back().col_name+"')");
    } catch (...) {}

  }
}

template<ColumnType::type ColumnT>
void sql_create_test_column(Client& client, uint32_t cell_versions=1) {
  sql_delete_test_column<ColumnT>(client);
  std::cout << std::endl << "test: sql_create_test_column: " << std::endl;
  std::string sql;
  for(auto c=1; c <= num_columns; ++c) {
    switch(ColumnT) {
      case ColumnType::type::PLAIN:
        sql = "create column(name='"+column_name(c)+"' "
                    "cell_versions="+std::to_string(cell_versions)+
                    " tags=[" + std::to_string(c % 2) + "]" +
                    ")";
        break;
      case ColumnType::type::COUNTER_I64:
        sql = "create column(name='"+column_name(c)+"' "
                    "cell_versions="+std::to_string(cell_versions)+
                    " tags=[" + std::to_string(c % 2) + "]" +
                    " type=COUNTER_I64)";
        break;
      case ColumnType::type::SERIAL: 
        sql = "create column(name='"+column_name(c)+"' "
                    "cell_versions="+std::to_string(cell_versions)+
                    " tags=[" + std::to_string(c % 2) + "]" +
                    " type=SERIAL)";
        break;
    }
    client.sql_mng_column(sql);
  }
}

template<ColumnType::type ColumnT>
void sql_update(Client& client, size_t updater_id=0, int batch=0) {
  std::cout << std::endl << "test: sql_update";
  if(updater_id)
    std::cout << " updater_id=" << updater_id;
  std::cout << ": " << std::endl;

  // UPDATE cell(FLAG, CID|NAME, KEY, TS_ORDER, TIMESTAMP, VALUE), cell(..)
  std::string cells("UPDATE ");
  for(auto c=1; c <= num_columns; ++c) {
    for(auto i=1; i <= num_cells; ++i) {
      for(auto f=1; f <= num_fractions; ++f) {
        cells.append("cell(");
        cells.append("INSERT,");

        cells.append(column_name(c));

        cells.append(",[");
        cells.append(key(i, f));
        cells.append("],");

        cells.append("''");
        //cells.append("'DESC or ASC',");
        //cells.append(std::to_string(now_ns()));
        cells.append(", ");

        cells.append(cell_value<ColumnT>(c, i, f, batch));
        cells.append(")");
        if(i != num_cells || c != num_columns || f != num_fractions)
          cells.append(",");
      }
    }
  }
  //std::cout << cells << " (" << batch << ")\n";
  client.sql_update(cells, updater_id);
}

template<ColumnType::type ColumnT>
void sql_update_with_id(Client& client) {
  std::cout << std::endl << "test: sql_update_with_id: " << std::endl;

  size_t updater_id = client.updater_create(4096);
  std::cout << "test: updater_create: " << std::endl;
  std::cout << " updater_id="<< updater_id << std::endl;
  assert(updater_id);

  for(size_t b=1; b<=batches;++b) {
    sql_update<ColumnT>(client, updater_id, b);
  }
  client.updater_close(updater_id);
}


std::string select_sql() {
  std::string sql("select where col(");
  for(auto c=1; c <= num_columns; ++c) {
    sql.append(column_name(c));
    if(c != num_columns)
      sql.append(",");
  }
  sql.append(")=(cells=())");
  return sql;
}

template<ColumnType::type ColumnT>
void sql_select(Client& client) {
  std::cout << std::endl << "test: sql_select: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  Cells container;
  client.sql_select(container, sql);

  print(container);

  switch(ColumnT) {
    case ColumnType::type::PLAIN:
      std::cout << " cells.size()=" << container.plain_cells.size()
                << " expected=" << total_cells*batches << "\n";
      assert(container.plain_cells.size() == total_cells*batches);
      break;
    case ColumnType::type::COUNTER_I64:
      std::cout << " counter_cells.size()=" << container.counter_cells.size()
                << " expected=" << total_cells * 1 << "\n";
      assert(container.counter_cells.size() == total_cells * 1);
      break;
    case ColumnType::type::SERIAL: 
      std::cout << " serial_cells.size()=" << container.serial_cells.size()
                << " expected=" << total_cells*batches << "\n";
      assert(container.serial_cells.size() == total_cells*batches);
      break;
  }
}

template<ColumnType::type ColumnT>
void sql_select_rslt_on_column(Client& client) {
  std::cout << std::endl << "test: sql_select_rslt_on_column: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  CCells cells;
  client.sql_select_rslt_on_column(cells, sql);

  print(cells);
  assert(cells.size() == num_columns);
}

template<ColumnType::type ColumnT>
void sql_select_rslt_on_key(Client& client) {
  std::cout << std::endl << "test: sql_select_rslt_on_key: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  KCells cells;
  client.sql_select_rslt_on_key(cells, sql);

  print(cells);
  assert(cells.size() == num_cells*num_fractions);
}

template<ColumnType::type ColumnT>
void sql_select_rslt_on_fraction(Client& client) {
  std::cout << std::endl << "test: sql_select_rslt_on_fraction: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  FCells cells;
  client.sql_select_rslt_on_fraction(cells, sql);

  print(cells);
  assert(cells.f.size() == num_cells);
}

template<ColumnType::type ColumnT>
void sql_query(Client& client, CellsResult::type rslt) {
  std::cout << std::endl << "test: sql_query, " << rslt << ": " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  CellsGroup group;
  client.sql_query(group, sql, rslt);

  std::cout << std::endl;
  group.printTo(std::cout);
  std::cout << std::endl;

  switch(rslt) {
    case CellsResult::ON_COLUMN : {
      print(group.ccells);
      assert(group.ccells.size() == num_columns);
      break;
    }
    case CellsResult::ON_KEY : {
      print(group.kcells);
      assert(group.kcells.size() == num_cells*num_fractions);
      break;
    }
    case CellsResult::ON_FRACTION : {
      print(group.fcells);
      assert(group.fcells.f.size() == num_cells);
      break;
    }
    default : {
      print(group.cells);
      switch(ColumnT) {
        case ColumnType::type::PLAIN:
          assert(group.cells.plain_cells.size() == total_cells);
          break;
        case ColumnType::type::COUNTER_I64:
          assert(group.cells.counter_cells.size() == total_cells);
          break;
        case ColumnType::type::SERIAL: 
          assert(group.cells.serial_cells.size() == total_cells);
          break;
      }
    }
  }
}



/* SPECS METHODS */

template<ColumnType::type ColumnT>
void spec_delete_test_column(Client& client) {
  std::cout << std::endl << "test: spec_delete_test_column: " << std::endl;


  for(auto c=1; c <= num_columns; ++c) {
    try {
      Schemas schemas;
      SpecSchemas spec;
      spec.names.emplace_back(column_name(c));
      client.list_columns(schemas, spec);

      if(!schemas.empty()) {
        client.mng_column(SchemaFunc::REMOVE, schemas.back());
        schemas.clear();
        client.list_columns(schemas, spec);
        assert(schemas.empty());
      }
    } catch (...) {}
  }
}

template<ColumnType::type ColumnT>
void spec_create_test_column(Client& client, uint32_t cell_versions=1) {
  spec_delete_test_column<ColumnT>(client);
  std::cout << std::endl << "test: spec_create_test_column: " << std::endl;
  Schema schema;
  for(auto c=1; c <= num_columns; ++c) {
    schema.__set_col_name(column_name(c));
    schema.__set_col_tags({std::to_string(c % 2)});
    schema.__set_cell_versions(cell_versions);
    schema.__set_col_type(ColumnT);
    std::cout << " create: " << schema << std::endl;
    client.mng_column(SchemaFunc::CREATE, schema);
  }
}

template<ColumnType::type ColumnT>
void spec_compact_test_column(Client& client) {
  std::cout << std::endl << "test: spec_compact_test_column: " << std::endl;

  SpecSchemas spec;
  for(auto c=1; c <= num_columns; ++c)
    spec.names.emplace_back(column_name(c));

  CompactResults res;
  client.compact_columns(res, spec);

  assert(res.size() == num_columns);
  for(auto& r : res)
    std::cout << " " << r << std::endl;
}

template<ColumnType::type ColumnT>
void spec_list_columns(Client& client) {
  std::cout << std::endl << "test: spec_list_columns: " << std::endl;

  SpecSchemas spec;
  auto& pattern = spec.patterns.names.emplace_back();
  pattern.value = column_pre;
  pattern.comp = Comp::PF;

  Schemas schemas;
  client.list_columns(schemas, spec);

  assert(schemas.size() == num_columns);
  std::cout << std::endl << "schemas.size=" << schemas.size() << std::endl;
  for(auto& schema : schemas) {
    schema.printTo(std::cout << " ");
    std::cout << std::endl;
  }

  spec.patterns.names.clear();
  spec.patterns.tags.comp = Comp::SBS;
  auto& tmp = spec.patterns.tags.values.emplace_back();
  tmp.value = "0";
  tmp.comp = Comp::EQ;

  schemas.clear();
  client.list_columns(schemas, spec);

  assert(schemas.size() == num_columns / 2);
  std::cout << std::endl << "schemas.size=" << schemas.size() << std::endl;
  for(auto& schema : schemas) {
    schema.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}

template<ColumnType::type ColumnT>
void spec_update(Client& client, size_t updater_id=0, int batch=0) {
  std::cout << std::endl << "test: spec_update";
  if(updater_id)
    std::cout << " updater_id=" << updater_id;
  std::cout << ": " << std::endl;

  Schemas schemas;
  std::string sql("get schema ");
  for(auto c=1; c <= num_columns; ++c) {
    sql.append(column_name(c));
    if(c != num_columns)
      sql += ',';
  }
  client.sql_list_columns(schemas, sql);

  switch(ColumnT) {
    case ColumnType::type::PLAIN: {
      UCCellsPlain cells;
      auto c=0;
      for(auto& schema : schemas) {
        ++c;
        auto& col = cells[schema.cid];
        for(auto i=1; i <= num_cells; ++i) {
          for(auto f=1; f <= num_fractions; ++f) {
            auto& cell = col.emplace_back();
            cell.f = Flag::INSERT;
            key(i, f, cell.k);
            cell.__set_ts(i*f*c);//now_ns());
            cell.__set_ts_desc(true);
            cell.__set_v(cell_value<ColumnT>(c, i, f, batch));
          }
        }
      }
      //std::cout << cells << "\n";
      client.update_plain(cells, updater_id);
      break;
    }
    case ColumnType::type::COUNTER_I64: {
      UCCellsCounter cells;
      auto c=0;
      for(auto& schema : schemas) {
        ++c;
        auto& col = cells[schema.cid];
        for(auto i=1; i <= num_cells; ++i) {
          for(auto f=1; f <= num_fractions; ++f) {
            auto& cell = col.emplace_back();
            cell.f = Flag::INSERT;
            key(i, f, cell.k);
            cell.__set_ts(i*f*c);//now_ns());
            cell.__set_ts_desc(true);
            cell.__set_v(c * i * f * batch);
          }
        }
      }
      //std::cout << cells << "\n";
      client.update_counter(cells, updater_id);
      break;
    }
    case ColumnType::type::SERIAL: {
      UCCellsSerial cells;
      auto c=0;
      for(auto& schema : schemas) {
        ++c;
        auto& col = cells[schema.cid];
        for(auto i=1; i <= num_cells; ++i) {
          for(auto f=1; f <= num_fractions; ++f) {
            auto& cell = col.emplace_back();
            cell.f = Flag::INSERT;
            key(i, f, cell.k);
            cell.__set_ts(i*f*c);//now_ns());
            cell.__set_ts_desc(true);
            cell.__set_encoder(EncodingType::SNAPPY);
            cell.v.resize(10);
            for(int fid=0; fid<10; ++fid) {
              auto& fields = cell.v[fid];
              fields.__set_field_id(fid);
              fields.__set_v_int64(f);
              fields.__set_v_double(f);
              fields.__set_v_bytes(apache::thrift::to_string(cell.k));
              fields.__set_v_key(cell.k);
              fields.__set_v_li({c, i, f, batch});
              fields.__set_v_lb({
                column_name(c),
                std::to_string(c),
                std::to_string(i),
                std::to_string(f),
                std::to_string(batch)
              });
            }
          }
        }
      }
      //std::cout << cells << "\n";
      client.update_serial(cells, updater_id);
      break;
    }
  }
}

template<ColumnType::type ColumnT>
SpecScan select_specs(Client& client) {
  SpecScan ss;
  for(auto c=1; c <= num_columns; ++c) {
    Schemas schemas;
    client.sql_list_columns(
      schemas,
      "get schema " + column_name(c)
    );
    assert(!schemas.empty());

    SpecKeyInterval* key_intval_ptr = nullptr;
    switch (ColumnT) {
      case ColumnType::type::PLAIN: {
        auto& col = ss.columns_plain.emplace_back();
        col.__set_cid(schemas.back().cid);
        auto& intval = col.intervals.emplace_back();
        intval.key_intervals.emplace_back();
        key_intval_ptr = &intval.key_intervals.back();
        break;
      }
      case ColumnType::type::COUNTER_I64: {
        auto& col = ss.columns_counter.emplace_back();
        col.__set_cid(schemas.back().cid);
        auto& intval = col.intervals.emplace_back();
        intval.key_intervals.emplace_back();
        key_intval_ptr = &intval.key_intervals.back();
        break;
      }
      case ColumnType::type::SERIAL: {
        auto& col = ss.columns_serial.emplace_back();
        col.__set_cid(schemas.back().cid);
        auto& intval = col.intervals.emplace_back();
        intval.key_intervals.emplace_back();
        key_intval_ptr = &intval.key_intervals.back();
        break;
      }
    }

    auto& key_intval = *key_intval_ptr;
    key_intval.start.resize(2);
    key_intval.start[0].__set_comp(Comp::EQ);
    key_intval.start[0].__set_f("a1");
    key_intval.start[1].__set_comp(Comp::FIP);
    key_intval.start[1].__set_f("");
  }
  return ss;
}

template<ColumnType::type ColumnT>
void spec_select(Client& client) {
  std::cout << std::endl << "test: spec_select: " << std::endl;

  SpecScan specs = select_specs<ColumnT>(client);
  specs.printTo(std::cout << " spec_select='");
  std::cout << "'\n";

  Cells container;
  client.scan(container, specs);

  print(container);
  switch (ColumnT) {
    case ColumnType::type::PLAIN: {
      std::cout << " plain_cells.size()=" << container.plain_cells.size()
                << " expected=" << num_fractions*num_columns << "\n";
      assert(container.plain_cells.size() == num_fractions * num_columns);
      break;
    }
    case ColumnType::type::COUNTER_I64: {
      std::cout << " counter_cells.size()=" << container.counter_cells.size()
                << " expected=" << num_fractions*num_columns << "\n";
      assert(container.counter_cells.size() == num_fractions * num_columns);
      break;
    }
    case ColumnType::type::SERIAL: {
      std::cout << " serial_cells.size()=" << container.serial_cells.size()
                << " expected=" << num_fractions*num_columns << "\n";
      assert(container.serial_cells.size() == num_fractions * num_columns);
      break;
    }
  }

  for(auto rslt_typ : {
    CellsResult::IN_LIST,
    CellsResult::ON_COLUMN,
    CellsResult::ON_KEY,
    CellsResult::ON_FRACTION}) {

    CellsGroup gcells;
    client.scan_rslt_on(gcells, specs, rslt_typ);

    gcells.printTo(std::cout);
    std::cout << "\n";
    switch(rslt_typ) {
      case CellsResult::ON_COLUMN: {
        std::cout << rslt_typ << " ccells.size()=" << gcells.ccells.size()
                  << " expected=" << num_columns << "\n";
        assert(gcells.ccells.size() == num_columns);
        for(auto& col : gcells.ccells) {
          for(auto& cell : col.second.serial_cells) {
            int f = cell.k.size();
            for(int fid=0; fid<10; ++fid) {
              auto& fields = cell.v[fid];
              assert(fields.field_id == fid);
              assert(fields.v_int64 == f);
              assert(fields.v_double == double(f));
              assert(fields.v_key == cell.k);
              assert(!fields.v_bytes.compare(apache::thrift::to_string(cell.k)));
              assert(fields.v_li[2] == f);
              assert(!fields.v_lb[3].compare(std::to_string(f)));
            }
          }
        }
        break;
      }
      case CellsResult::ON_FRACTION: {
        std::cout << rslt_typ << " fcells.f.size()=" << gcells.fcells.f.size()
                  << " expected=" << 1 << "\n";
        assert(gcells.fcells.f.size() == 1);
        break;
      }
      case CellsResult::ON_KEY: {
        std::cout << rslt_typ << " kcells.size()=" << gcells.kcells.size()
                  << " expected=" << num_fractions << "\n";
        assert(gcells.kcells.size() == num_fractions);
        break;
      }
      default: { // IN_LIST
        switch (ColumnT) {
          case ColumnType::type::PLAIN: {
            std::cout<< rslt_typ << " plain_cells.size()=" << gcells.cells.plain_cells.size()
                      << " expected=" << num_fractions*num_columns << "\n";
            assert(gcells.cells.plain_cells.size() == num_fractions * num_columns);
            break;
          }
          case ColumnType::type::COUNTER_I64: {
            std::cout<< rslt_typ << " counter_cells.size()=" << gcells.cells.counter_cells.size()
                      << " expected=" << num_fractions*num_columns << "\n";
            assert(gcells.cells.counter_cells.size() == num_fractions * num_columns);
            break;
          }
          case ColumnType::type::SERIAL: {
            std::cout<< rslt_typ << " serial_cells.size()=" << gcells.cells.serial_cells.size()
                      << " expected=" << num_fractions*num_columns << "\n";
            assert(gcells.cells.serial_cells.size() == num_fractions * num_columns);
            break;
          }
        }
      }
    }
  }
}


template<ColumnType::type ColumnT>
void test() {
  batches = 1;
  //auto client = Client::make("localhost", 18000);

  Client client("localhost", 18000);

  /** SQL **/
  sql_list_columns_all<ColumnT>(client);
  sql_mng_and_list_column<ColumnT>(client);

  sql_create_test_column<ColumnT>(client);
  sql_compact_columns<ColumnT>(client);

  sql_update<ColumnT>(client);

  sql_select<ColumnT>(client);
  sql_select_rslt_on_column<ColumnT>(client);
  sql_select_rslt_on_key<ColumnT>(client);
  sql_select_rslt_on_fraction<ColumnT>(client);

  sql_query<ColumnT>(client, CellsResult::IN_LIST);
  sql_query<ColumnT>(client, CellsResult::ON_COLUMN);
  sql_query<ColumnT>(client, CellsResult::ON_KEY);
  sql_query<ColumnT>(client, CellsResult::ON_FRACTION);

  sql_delete_test_column<ColumnT>(client);

  // with updater_id
  sql_create_test_column<ColumnT>(client, batches *= 10);
  sql_update_with_id<ColumnT>(client);
  sql_select<ColumnT>(client);
  sql_delete_test_column<ColumnT>(client);

  batches = 1;

  /** SPECS **/
  spec_create_test_column<ColumnT>(client);
  spec_update<ColumnT>(client);
  spec_select<ColumnT>(client);
  spec_compact_test_column<ColumnT>(client);
  spec_select<ColumnT>(client);

  spec_list_columns<ColumnT>(client);
  spec_delete_test_column<ColumnT>(client);

  client.close();

  std::cout << std::endl << " # OK! #" << std::endl;
}

}
}}

int main() {
  SWC::Thrift::Test::test<SWC::Thrift::ColumnType::type::PLAIN>();
  SWC::Thrift::Test::test<SWC::Thrift::ColumnType::type::COUNTER_I64>();
  SWC::Thrift::Test::test<SWC::Thrift::ColumnType::type::SERIAL>();
}