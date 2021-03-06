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

std::string cell_value(int c, int i, int f, int batch) {
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



/* OUTPUT */
void print(const Cells& container) {
  std::cout << "cells.size=" << container.cells.size() << std::endl;
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

  client.sql_mng_column(
    "create column(name='col-test-create-1' cell_ttl=123456)"
  );
  schemas.clear();
  client.sql_list_columns(
    schemas,
    "get schema col-test-create-1"
  );
  assert(schemas.size() == 1);

  std::string sql("modify column(cid=");
  sql.append(std::to_string(schemas.back().cid));
  sql.append(" name='col-test-create-1' cell_ttl=123456789)");
  client.sql_mng_column(sql);

  schemas.clear();
  client.sql_list_columns(
    schemas,
    "get schema col-test-create-1"
  );
  assert(schemas.size() == 1);
  assert(schemas.back().cell_ttl == 123456789);

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

void sql_create_test_column(Client& client, uint32_t cell_versions=1) {
  sql_delete_test_column(client);
  std::cout << std::endl << "test: sql_create_test_column: " << std::endl;
  for(auto c=1; c <= num_columns; ++c) {
    client.sql_mng_column(
      "create column(name='"+column_name(c)+"' "
                    "cell_versions="+std::to_string(cell_versions)+
                    ")");
  }
}

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

        cells.append(cell_value(c, i, f, batch));
        cells.append(")");
        if(i != num_cells || c != num_columns || f != num_fractions)
          cells.append(",");
      }
    }
  }
  //std::cout << cells << " (" << batch << ")\n";
  client.sql_update(cells, updater_id);
}

void sql_update_with_id(Client& client) {
  std::cout << std::endl << "test: sql_update_with_id: " << std::endl;

  size_t updater_id = client.updater_create(4096);
  std::cout << "test: updater_create: " << std::endl;
  std::cout << " updater_id="<< updater_id << std::endl;
  assert(updater_id);

  for(size_t b=1; b<=batches;++b) {
    sql_update(client, updater_id, b);
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

void sql_select(Client& client) {
  std::cout << std::endl << "test: sql_select: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  Cells container;
  client.sql_select(container, sql);

  print(container);

  std::cout << " cells.size()=" << container.cells.size()
            << " expected=" << total_cells*batches << "\n";
  assert(container.cells.size() == total_cells*batches);
}

void sql_select_rslt_on_column(Client& client) {
  std::cout << std::endl << "test: sql_select_rslt_on_column: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  CCells cells;
  client.sql_select_rslt_on_column(cells, sql);

  print(cells);
  assert(cells.size() == num_columns);
}

void sql_select_rslt_on_key(Client& client) {
  std::cout << std::endl << "test: sql_select_rslt_on_key: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  KCells cells;
  client.sql_select_rslt_on_key(cells, sql);

  print(cells);
  assert(cells.size() == num_cells*num_fractions);
}

void sql_select_rslt_on_fraction(Client& client) {
  std::cout << std::endl << "test: sql_select_rslt_on_fraction: " << std::endl;

  std::string sql = select_sql();
  std::cout << " SQL='" << sql << "'\n";

  FCells cells;
  client.sql_select_rslt_on_fraction(cells, sql);

  print(cells);
  assert(cells.f.size() == num_cells);
}

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
      assert(group.cells.cells.size() == total_cells);
      break;
    }
  }
}



/* SPECS METHODS */

void spec_delete_test_column(Client& client) {
  std::cout << std::endl << "test: spec_delete_test_column: " << std::endl;


  for(auto c=1; c <= num_columns; ++c) {
    try {
      Schemas schemas;
      SpecSchemas spec;
      spec.names.emplace_back(column_name(c));
      client.list_columns(schemas, spec);

      if(!schemas.empty()) {
        client.mng_column(SchemaFunc::DELETE, schemas.back());
        schemas.clear();
        client.list_columns(schemas, spec);
        assert(schemas.empty());
      }
    } catch (...) {}
  }
}

void spec_create_test_column(Client& client, uint32_t cell_versions=1,
                             bool serial=false) {
  spec_delete_test_column(client);
  std::cout << std::endl << "test: spec_create_test_column: " << std::endl;
  Schema schema;
  for(auto c=1; c <= num_columns; ++c) {
    schema.__set_col_name(column_name(c));
    schema.__set_cell_versions(cell_versions);
    if(serial)
      schema.__set_col_type(ColumnType::type::SERIAL);

    std::cout << " create: " << schema << std::endl;
    client.mng_column(SchemaFunc::CREATE, schema);
  }
}

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

void spec_list_columns(Client& client) {
  std::cout << std::endl << "test: spec_list_columns: " << std::endl;

  SpecSchemas spec;
  auto& pattern = spec.patterns.emplace_back();
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
}

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

  UCCells cells;
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
        cell.__set_v(cell_value(c, i, f, batch));
      }
    }
  }
  //std::cout << cells << "\n";
  client.update(cells, updater_id);
}

void spec_update_serial(Client& client, size_t updater_id=0, int batch=0) {
  std::cout << std::endl << "test: spec_update_serial";
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
}

SpecScan select_specs(Client& client) {
  SpecScan ss;
  for(auto c=1; c <= num_columns; ++c) {
    Schemas schemas;
    client.sql_list_columns(
      schemas,
      "get schema " + column_name(c)
    );
    assert(!schemas.empty());

    auto& col = ss.columns.emplace_back();
    col.__set_cid(schemas.back().cid);
    auto& intval = col.intervals.emplace_back();

    intval.key_intervals.emplace_back();
    auto& key_intval = intval.key_intervals.back();

    key_intval.start.resize(2);
    key_intval.start[0].__set_comp(Comp::EQ);
    key_intval.start[0].__set_f("a1");
    key_intval.start[1].__set_comp(Comp::GE);
    key_intval.start[1].__set_f("");
  }
  return ss;
}

void spec_select(Client& client) {
  std::cout << std::endl << "test: spec_select: " << std::endl;

  SpecScan specs = select_specs(client);
  specs.printTo(std::cout << " spec_select='");
  std::cout << "'\n";

  Cells container;
  client.scan(container, specs);

  print(container);
  std::cout << " cells.size()=" << container.cells.size()
            << " serial_cells.size()=" << container.serial_cells.size()
            << " expected=" << num_fractions*num_columns << "\n";
  assert(container.cells.size() + container.serial_cells.size()
         == num_fractions*num_columns);

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
        std::cout << rslt_typ
                  << " cells.size()=" << gcells.cells.cells.size()
                  << " serial_cells.size()=" << gcells.cells.serial_cells.size()
                  << " expected=" << num_fractions*num_columns << "\n";
        assert(gcells.cells.cells.size() + gcells.cells.serial_cells.size()
                == num_fractions*num_columns);
        break;
      }
    }
  }
}



}
}}

namespace Test = SWC::Thrift::Test;

int main() {
  //auto client = SWC::Thrift::Client::make("localhost", 18000);

  SWC::Thrift::Client client("localhost", 18000);

  /** SQL **/
  Test::sql_list_columns_all(client);
  Test::sql_mng_and_list_column(client);

  Test::sql_create_test_column(client);
  Test::sql_compact_columns(client);

  Test::sql_update(client);

  Test::sql_select(client);
  Test::sql_select_rslt_on_column(client);
  Test::sql_select_rslt_on_key(client);
  Test::sql_select_rslt_on_fraction(client);

  Test::sql_query(client, SWC::Thrift::CellsResult::IN_LIST);
  Test::sql_query(client, SWC::Thrift::CellsResult::ON_COLUMN);
  Test::sql_query(client, SWC::Thrift::CellsResult::ON_KEY);
  Test::sql_query(client, SWC::Thrift::CellsResult::ON_FRACTION);

  Test::sql_delete_test_column(client);

  // with updater_id
  Test::sql_create_test_column(client, batches *= 10);
  Test::sql_update_with_id(client);
  Test::sql_select(client);
  Test::sql_delete_test_column(client);

  batches = 1;

  /** SPECS **/
  Test::spec_create_test_column(client);
  Test::spec_update(client);
  Test::spec_select(client);
  Test::spec_compact_test_column(client);
  Test::spec_select(client);

  Test::spec_list_columns(client);
  Test::spec_delete_test_column(client);

  /** SPECS Serial Value **/
  Test::spec_create_test_column(client, 1, true);
  Test::spec_update_serial(client);
  Test::spec_select(client);
  Test::spec_delete_test_column(client);


  client.close();

  std::cout << std::endl << " # OK! #" << std::endl;
}