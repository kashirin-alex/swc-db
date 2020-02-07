/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>
#include <cassert>
#include "swcdb/thrift/client/Client.h"

namespace  SWC { namespace Thrift {
namespace Test {

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

void print(const Cells& cells) {
  std::cout << "cells.size=" << cells.size() << std::endl;
  for(auto& cell : cells) {
    cell.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}

void sql_select(Client& client) {
  std::cout << std::endl << "test: sql_select: " << std::endl;

  Cells cells;
  client.sql_select(
    cells, 
    "select where col(col-test-1)=(cells=(offset=10000 limit=10 ONLY_KEYS))"
  );
  print(cells);
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

  CCells cells;
  client.sql_select_rslt_on_column(
    cells, 
    "select where col(col-test-1)=(cells=(offset=10000 limit=10 ONLY_KEYS))"
  );
  print(cells);
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

  KCells cells;
  client.sql_select_rslt_on_key(
    cells, 
    "select where" 
    "col(col-test-1)=(cells=(offset=200000 max_versions=1 limit=10 ONLY_KEYS))"
      " and " 
    "col(col-test-2)=(cells=(offset=10000 limit=10 ONLY_KEYS))"
  );
  print(cells);
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

  FCells cells;
  client.sql_select_rslt_on_fraction(
    cells, 
    "select where" 
    "col(col-test-1)=(cells=(offset=20280 max_versions=1 limit=52 ONLY_KEYS))"
      " and " 
    "col(col-test-2)=(cells=(offset=1014 limit=52 ONLY_KEYS))"
  );

  print(cells);
}


void sql_exec_query(Client& client, CellsResult::type rslt) {
  std::cout << std::endl << "test: sql_exec_query, " << rslt << ": " << std::endl;

  CellsGroup group;
  client.sql_exec_query(
    group, 
    "select where" 
    "col(col-test-1)=(cells=(offset=200000 max_versions=1 limit=10 ONLY_KEYS))"
      " and " 
    "col(col-test-2)=(cells=(offset=10000 limit=10 ONLY_KEYS))",
    rslt
  );

  std::cout << std::endl;
  group.printTo(std::cout);
  std::cout << std::endl;

  switch(rslt) {
    case CellsResult::ON_COLUMN : {
      print(group.ccells);
      break;
    }
    case CellsResult::ON_KEY : {
      print(group.kcells);
      break;
    }
    case CellsResult::ON_FRACTION : {
      print(group.fcells);
      break;
    }
    default : {
      print(group.cells);
      break;
    }
  }
}


}
}}

namespace Test = SWC::Thrift::Test;

int main() {
  //auto client = SWC::Thrift::Client::make("localhost", 18000);

  SWC::Thrift::Client client("localhost", 18000);
  
  Test::sql_list_columns_all(client);
  Test::sql_mng_and_list_column(client);

  Test::sql_select(client);
  Test::sql_select_rslt_on_column(client);
  Test::sql_select_rslt_on_key(client);
  Test::sql_select_rslt_on_fraction(client);

  Test::sql_exec_query(client, SWC::Thrift::CellsResult::IN_LIST);
  Test::sql_exec_query(client, SWC::Thrift::CellsResult::ON_COLUMN);
  Test::sql_exec_query(client, SWC::Thrift::CellsResult::ON_KEY);
  Test::sql_exec_query(client, SWC::Thrift::CellsResult::ON_FRACTION);

  client.close();

}