/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>
#include <cassert>
#include "swcdb/thrift/client/Client.h"

const std::string column_pre("thrift-client-test");
const int num_columns = 5;
const int num_cells = 10;
const int num_fractions = 26; // a=1 and z=26
const int total_cells = num_columns*num_cells*num_fractions;
int batches = 1; // changed on test



namespace  SWC { namespace Thrift {
namespace Test {

const int64_t now_ns() {
  return (int64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string column_name(int c) {
  return column_pre+"-"+std::to_string(c);
}
std::string key(int i, int f) {
  std::string key;
  for(uint8_t n=97;n<97+f;++n) {
    key += (char)n;
    key.append(std::to_string(i));
    if(n != 96+f)
      key += ',';
  }
  return key;
}
void key(int i, int f, Key& key) {
  std::string fraction;
  for(uint8_t n=97;n<97+f;++n) {
    fraction += (char)n;
    fraction.append(std::to_string(i));
    key.emplace_back(fraction);
    fraction.clear();
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


void sql_mng_and_list_column(Client& client) {
  std::cout << std::endl << "test: sql_mng_column: " << std::endl;

  client.sql_mng_column(
    "create column(name='col-test-create-1' cell_ttl=123456)"
  );

  Schemas schemas;
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
  
  // UPDATE cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE), cell(..)
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
        //cells.append(std::to_string(now_ns()));
        cells.append(", ");

        cells.append(cell_value(c, i, f, batch));
        cells.append(")");
        if(i != num_cells || c != num_columns || f != num_fractions) 
          cells.append(",");
      }
    }
  }
  //std::cout << cells << "\n";
  client.sql_update(cells, updater_id);
}

void sql_update_with_id(Client& client) {
  std::cout << std::endl << "test: sql_update_with_id: " << std::endl;
  
  size_t updater_id = client.updater_create(4096);
  std::cout << "test: updater_create: " << std::endl;
  std::cout << " updater_id="<< updater_id << std::endl;
  assert(updater_id);

  for(int b=1; b<=batches;++b) {
    sql_update(client, updater_id, b);
  }
  client.updater_close(updater_id);
}


void print(const Cells& cells) {
  std::cout << "cells.size=" << cells.size() << std::endl;
  for(auto& cell : cells) {
    cell.printTo(std::cout << " ");
    std::cout << std::endl;
  }
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

  Cells cells;
  client.sql_select(cells, sql);

  print(cells);
  
  std::cout << " cells.size()=" << cells.size() 
            << " expected=" << total_cells*batches << "\n";
  assert(cells.size() == total_cells*batches);
}

void print(const CCells& cells) {
  std::cout << "columns.size=" << cells.size() << std::endl;
  for(auto& col : cells) {
    std::cout << "column=" << col.first 
              << " cells.size=" << col.second.size() << std::endl;
    for(auto& cell : col.second) {
      cell.printTo(std::cout << " ");
      std::cout << std::endl;
    }
  }
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

void print(const KCells& cells) {
  std::cout << "keys.size=" << cells.size() << std::endl;
  for(auto& key_cells : cells) {
    key_cells.printTo(std::cout << " ");
    std::cout << std::endl;
  }
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

void print(FCells* fcells, Key& key) {
  for(auto& f : fcells->f) {
    key.emplace_back(f.first);
    std::cout << "fraction='" << f.first << "'"
              << " cells.size=" << f.second.cells.size() 
              << " key.size=" << key.size() << " ";
    for(auto& fraction : key)
      std::cout << "/" << fraction;
    std::cout << std::endl;
    for(auto& cell : f.second.cells) {
      std::cout << " " << cell << "\n";
    }
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
      assert(group.cells.size() == total_cells);
      break;
    }
  }
}


void update(Client& client, size_t updater_id=0, int batch=0) {
  std::cout << std::endl << "test: update";
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
        cell.__set_v(cell_value(c, i, f, batch));
      }
    }
  }
  //std::cout << cells << "\n";
  client.update(cells, updater_id);
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
  
  Test::sql_create_test_column(client);
  Test::update(client);
  Test::sql_select(client);
  Test::sql_delete_test_column(client);

  /** SPECS **/


  client.close();

}