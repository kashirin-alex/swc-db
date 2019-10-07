/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include <thread>
#include <iostream>
#include <cstring>
#include <vector>

#include "swcdb/lib/db/Cells/Mutable.h"
#include "swcdb/lib/db/Cells/Chain.h"
#include "swcdb/lib/db/Stats/Stat.h"


namespace Cells = SWC::DB::Cells;

void check(size_t num_cells) {
  std::cout << " checking with cells=" << num_cells << "\n";
      

  //std::cout << "\n--------Init Cells-------------\n";
  std::shared_ptr<SWC::Stats::Stat> latency_vector = std::make_shared<SWC::Stats::Stat>();
  int64_t ts_total_vector = 0;
  int64_t took_vector;

  int64_t rev;
  std::vector<Cells::Cell*> cells;
  std::string value;
  Cells::Cell cell;
  
  /*
  std::shared_ptr<SWC::Stats::Stat> latency = std::make_shared<SWC::Stats::Stat>();
  int64_t ts_total = 0;
  int64_t took;
  Cells::Chain::Ptr cells_chain  = std::make_shared<Cells::Chain>(1);
  */

  std::shared_ptr<SWC::Stats::Stat> latency_mutable = std::make_shared<SWC::Stats::Stat>();
  int64_t ts_total_mutable = 0;
  int64_t took_mutable;
  Cells::Mutable::Ptr cells_mutable = Cells::Mutable::make(1, 2, 0, SWC::Types::Column::COUNTER_I64);
  
  int num_revs = 5;
  for(auto r=1;r<=num_revs;++r){
  for(auto i=0;i<num_cells;++i){
      //std::cout << "Initial Cell-"<< i << ":\n";
      std::string n = std::to_string(i);
      
      rev = SWC::Time::now_ns();
      cell.flag = Cells::INSERT;
      cell.set_timestamp(rev-1);
      cell.set_revision(rev);
      cell.set_time_order_desc(false);

      cell.key.free();
      cell.key.add("aKey1");
      cell.key.add("aKey2");
      cell.key.add(n);
      cell.key.add("aKey3");
      cell.key.add("aKey4");
      cell.key.add("aKey5");
      if(num_revs == r)
        cell.set_value(Cells::OP::EQUAL, 0);
      else
        cell.set_value(Cells::OP::PLUS, 1);
      //value = std::string("A-Data-Value-1234567890-"+n);
      //cell.set_value(value.data(), value.length());

      took_vector = SWC::Time::now_ns();

      //cells.insert(cells.begin(), new Cells::Cell(cell));
      cells.push_back(new Cells::Cell(cell));

      took_vector = SWC::Time::now_ns()-took_vector;
      ts_total_vector += took_vector;
      latency_vector->add(took_vector); 
      
      /*
      took = SWC::Time::now_ns();

      cells_chain->add(cell);

      took = SWC::Time::now_ns()-took;
      ts_total += took;
      latency->add(took); 
      */
      
      took_mutable = SWC::Time::now_ns();

      //cells_mutable->insert(0, cell);
      //cells_mutable->add(cell);
      cells_mutable->push_back(cell);

      took_mutable = SWC::Time::now_ns()-took_mutable;
      ts_total_mutable += took_mutable;
      latency_mutable->add(took_mutable); 

      if(i % 100000 == 0)
        std::cout << "took=" << took_mutable << " " << r << "/" << i << " = " << cells_mutable->size() << "\n";

      //std::cout << cells.back()->to_string() << "\n\n";
  }
  }

  std::cout << " vector "
            << " avg="    << latency_vector->avg()
            << " min="    << latency_vector->min()
            << " max="    << latency_vector->max()
            << " count="  << latency_vector->count()
            << " median=" << ts_total_vector/num_cells
            << " took="   << ts_total_vector / 1000000
            << " counting, ";
  took_vector = SWC::Time::now_ns();
  int64_t tmpc = 0;
  for(auto& cell: cells){
    if(cell->vlen >= 0)
      tmpc++;
  }
  std::cout << " counted=" << tmpc
            << " took=" << SWC::Time::now_ns()-took_vector << "\n"; 
  
  /// 
  /*
  std::cout << " chained"
            << " avg="    << latency->avg()
            << " min="    << latency->min()
            << " max="    << latency->max()
            << " count="  << latency->count()
            << " median=" << ts_total/latency->count()
            << " counting, ";
  took = SWC::Time::now_ns();
  std::cout << " counted=" << cells_chain->count()
            << " took=" << SWC::Time::now_ns()-took << "\n"; 

  cells_chain =  nullptr;
  */
  /// 

  std::cout << " mutable"
            << " avg="    << latency_mutable->avg()
            << " min="    << latency_mutable->min()
            << " max="    << latency_mutable->max()
            << " count="  << latency_mutable->count()
            << " median=" << ts_total_mutable / num_cells
            << " took="   << ts_total_mutable / 1000000
            << " counting, ";
  took_mutable = SWC::Time::now_ns();
  std::cout << " counted=" << cells_mutable->size()
            << " took=" << SWC::Time::now_ns()-took_mutable<< "\n"; 

  //cells_mutable->free();
  
  
  std::cout << " clearing, ";
  
  cells_mutable = nullptr;

  for(auto & cell : cells)
    delete cell;
  cells.clear();

  std::cout << " cleared check with cells=" << num_cells << "\n";

}
  
int main(int argc, char** argv) {
  
  check(1);
  check(10);
  check(100);
  check(1000);
  check(10000);

  check(100000);
  check(100000);
  check(100000);

  check(1000000);

  for(auto i=1; i<=100; i++) {
    check(1000000);
  //  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }


  //for(auto i=1; i<=10000000; i*=10)
  //  check(i);

  exit(0);
}