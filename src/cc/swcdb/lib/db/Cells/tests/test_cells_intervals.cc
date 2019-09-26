/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>

#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/db/Cells/Intervals.h"
#include "swcdb/lib/db/Files/CellStore.h"


namespace DB = SWC::DB;
namespace Files = SWC::Files;
namespace Condition = SWC::Condition;

int main() {
  
  DB::Cells::Intervals expected_expanded;
  int n_cs = 9999;
  Files::CellStores     cellstores;
  for(int n=1; n<=n_cs;n++)
    cellstores.push_back(std::make_shared<Files::CellStore>(n));

  for(const auto& cs : cellstores){
      auto s = std::to_string(n_cs-cs->cs_id);
      DB::Specs::Key key;
      key.add("11", Condition::GE);
      key.add(std::string("a")+s, Condition::GE);
      key.add(std::string("b")+s, Condition::GE);
      key.add(std::string("c")+s, Condition::GE);
      key.add(std::string("d")+s, Condition::GE);
      cs->intervals->key_begin(key);
      if(!key.equal(cs->intervals->get_key_begin())) {
        std::cerr << "!key.equal(cs->intervals->get_key_begin()): ERROR\n";
        exit(1);
      }
      if(n_cs-cs->cs_id == 0)
        expected_expanded.key_begin(key);
      key.free();

      s = std::to_string(cs->cs_id);
      key.add("11", Condition::LE);
      key.add(std::string("a")+s, Condition::LE);
      key.add(std::string("b")+s, Condition::LE);
      key.add(std::string("c")+s, Condition::LE);
      key.add(std::string("d")+s, Condition::LE);
      cs->intervals->key_end(key);
      if(!key.equal(cs->intervals->get_key_end())) {
        std::cerr << "!key.equal(cs->intervals->get_key_end()): ERROR\n";
        exit(1);
      }
      if(cs->cs_id == n_cs)
        expected_expanded.key_end(key);
      
      DB::Specs::Timestamp ts;
      ts.comp = Condition::GE;
      ts.value = n_cs-cs->cs_id;
      cs->intervals->ts_earliest(ts);
      if(!ts.equal(cs->intervals->get_ts_earliest())) {
        std::cerr << "!ts.equal(cs->intervals->get_ts_earliest()): ERROR\n";
        exit(1);
      }
      if(ts.value == 0)
        expected_expanded.ts_earliest(ts);

      ts.comp = Condition::LE;
      ts.value = cs->cs_id;
      cs->intervals->ts_latest(ts);
      if(!ts.equal(cs->intervals->get_ts_latest())) {
        std::cerr << "!ts.equal(cs->intervals->get_ts_latest()): ERROR\n";
        exit(1);
      }
      if(ts.value == n_cs)
        expected_expanded.ts_latest(ts);

  }
  std::cout << " init OK\n";
  std::cout << "\n";

  DB::Cells::Intervals::Ptr intvals = std::make_shared<DB::Cells::Intervals>();
  std::cout << "\n";

  bool init = false;
  for(const auto& cs : cellstores) {
    intvals->expand(cs->intervals, init);
    init=true;
    //std::cout << cs->to_string() << "\n";
  }
  if(!expected_expanded.equal(intvals)){
    std::cout << " expected,  " << expected_expanded.to_string() << "\n";
    std::cout << " resulting, " << intvals->to_string() << "\n";
    std::cerr << "!expected_expanded.equal(intvals)): ERROR\n";
    exit(1);
  }
  exit(0);
}