/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <iostream>

#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/db/Cells/Interval.h"
#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Files/CellStore.h"


namespace DB = SWC::DB;
namespace Files = SWC::Files;
namespace Condition = SWC::Condition;

int main() {
  
  DB::Cells::Interval expected_expanded;
  int n_cs = 9999;
  Files::CellStore::ReadersPtr cellstores = std::make_shared<Files::CellStore::Readers>();
  
  for(int n=1; n<=n_cs;n++){
    cellstores->push_back(
      std::make_shared<Files::CellStore::Read>(
        n, 
        std::make_shared<DB::RangeBase>(1,1), 
        DB::Cells::Interval()
      )
    );
  }

  for(const auto& cs : *cellstores.get()){
      auto s = std::to_string(n_cs-cs->id);
      DB::Cell::Key key;
      key.add("11");
      key.add(std::string("a")+s);
      key.add(std::string("b")+s);
      key.add(std::string("c")+s);
      key.add(std::string("d")+s);
      cs->interval.set_key_begin(key);
      if(!key.equal(cs->interval.key_begin)) {
        std::cerr << "!key.equal(cs->interval.key_begin): ERROR\n";
        exit(1);
      }
      if(n_cs-cs->id == 0)
        expected_expanded.set_key_begin(key);
      key.free();

      s = std::to_string(cs->id);
      key.add("11");
      key.add(std::string("a")+s);
      key.add(std::string("b")+s);
      key.add(std::string("c")+s);
      key.add(std::string("d")+s);
      cs->interval.set_key_end(key);
      if(!key.equal(cs->interval.key_end)) {
        std::cerr << "!key.equal(cs->interval.key_end): ERROR\n";
        exit(1);
      }
      if(cs->id == n_cs)
        expected_expanded.set_key_end(key);
      
      DB::Specs::Timestamp ts(n_cs-cs->id, Condition::GE);
      cs->interval.set_ts_earliest(ts);
      if(!ts.equal(cs->interval.ts_earliest)) {
        std::cerr << "!ts.equal(cs->interval.ts_earliest): ERROR\n";
        exit(1);
      }
      if(ts.value == 0)
        expected_expanded.set_ts_earliest(ts);

      ts.set(cs->id, Condition::LE);
      cs->interval.set_ts_latest(ts);
      if(!ts.equal(cs->interval.ts_latest)) {
        std::cerr << "!ts.equal(cs->interval.ts_latest): ERROR\n";
        exit(1);
      }
      if(ts.value == n_cs)
        expected_expanded.set_ts_latest(ts);

  }
  std::cout << " init OK\n";
  std::cout << "\n";

  DB::Cells::Interval intval;
  std::cout << "\n";

  for(const auto& cs : *cellstores.get()) {
    //std::cout << cs->to_string() << "\n";
    intval.expand(cs->interval);
  }
  if(!expected_expanded.equal(intval)){
    std::cout << " expected,  " << expected_expanded.to_string() << "\n";
    std::cout << " resulting, " << intval.to_string() << "\n";
    std::cerr << "!expected_expanded.equal(intval)): ERROR\n";
    exit(1);
  }
  exit(0);
}