/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>
#include "swcdb/thrift/client/Client.h"

namespace  SWC { namespace Thrift {
namespace Test {

void sql_select_map(Client& client) {
  std::cout << std::endl << "test: sql_select_map: " << std::endl;

  ColumnsMapCells columns;
  client.sql_select_map(
    columns, 
    "select where col(col-test-1)=(cells=(offset=10000 limit=10 ONLY_KEYS))"
  );
      
  std::cout << "columns.size=" << columns.size() << std::endl;
  for(auto& col : columns) {
    std::cout << "column=" << col.first 
              << " cells.size=" << col.second.size() << std::endl;
    for(auto& cell : col.second) {
      cell.printTo(std::cout << " ");
      std::cout << std::endl;
    }
  }
}

void sql_select_list(Client& client) {
  std::cout << std::endl << "test: sql_select_list: " << std::endl;

  Cells cells;
  client.sql_select_list(
    cells, 
    "select where col(col-test-1)=(cells=(offset=10000 limit=10 ONLY_KEYS))"
  );
      
  std::cout << "cells.size=" << cells.size() << std::endl;
  for(auto& cell : cells) {
    cell.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}

void sql_select_keys(Client& client) {
  std::cout << std::endl << "test: sql_select_keys: " << std::endl;

  KeysCells keys_cells;
  client.sql_select_keys(
    keys_cells, 
    "select where" 
    "col(col-test-1)=(cells=(offset=200000 max_versions=1 limit=10 ONLY_KEYS))"
      "and" 
    "col(col-test-2)=(cells=(offset=10000 limit=10 ONLY_KEYS))"
  );
      
  std::cout << "keys_cells.size=" << keys_cells.size() << std::endl;
  for(auto& key_cells : keys_cells) {
    key_cells.printTo(std::cout << " ");
    std::cout << std::endl;
  }
}


}
}}

namespace Test = SWC::Thrift::Test;

int main() {
  //auto client = SWC::Thrift::Client::make("localhost", 18000);

  SWC::Thrift::Client client("localhost", 18000);
  
  
  Test::sql_select_map(client);
  Test::sql_select_list(client);
  Test::sql_select_keys(client);

  client.close();

}