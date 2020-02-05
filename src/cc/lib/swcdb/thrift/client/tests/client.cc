/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>
#include "swcdb/thrift/client/Client.h"



int main() {
  //auto client = SWC::Thrift::Client::make("localhost", 18000);

  for(auto t=0;t<100;t++) {
  std::thread([t](){
  SWC::Thrift::Client client("localhost", 18000);
  try {
    for(auto i =0; i<1000; i++) {
      SWC::Thrift::Result res;
      std::string q(std::to_string(t));
      q.append("/ A SQL DATA /");
      q.append(std::to_string(i));

      client.select_sql(res, q);
      
      std::cout << "client.select_sql(): " << res << std::endl;
    }
  } catch (SWC::thrift::TException& tx) {
    std::cout << "ERROR: " << tx.what() << std::endl;
  }
  }).detach();
  }

  std::this_thread::sleep_for(std::chrono::seconds(30));
}