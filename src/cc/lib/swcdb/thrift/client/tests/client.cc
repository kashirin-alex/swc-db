/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>
#include "swcdb/thrift/client/Client.h"



int main() {
  //auto client = SWC::Thrift::Client::make("localhost", 18000);

  for(auto t=0;t<1;t++) {
  std::thread([t](){
  SWC::Thrift::Client client("localhost", 18000);
  try {
    for(auto i =0; i<1; i++) {
      SWC::Thrift::Cells cells;

      client.select_sql(cells, "select where col(col-test-1)=(cells=(offset=10000 limit=10 ONLY_KEYS))");
      
      std::cout << "client.select_sql(): sz=" << cells.size() << std::endl;
      for(auto& cell : cells) {
        cell.printTo(std::cout);
        std::cout << std::endl;
      }

    }
  } catch (SWC::thrift::TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
  }).detach();
  }

  std::this_thread::sleep_for(std::chrono::seconds(3));
}