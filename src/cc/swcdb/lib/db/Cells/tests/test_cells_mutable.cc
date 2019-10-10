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

void check(SWC::Types::Column typ, size_t num_cells = 1, int num_revs = 1, bool reverse=false,
          int max_versions = 1, bool time_order_desc=false, bool gen_historic=false) {
  std::cout << " checking with type=" << SWC::Types::to_string(typ) 
                           << " cells=" << num_cells 
                           << " revs=" << num_revs 
                           << " reverse=" << reverse
                           << " max_versions=" << max_versions 
                           << " time_order_desc=" << time_order_desc 
                           << " gen_historic=" << gen_historic 
                           << "\n";
  bool counter = typ == SWC::Types::Column::COUNTER_I64;
  int64_t rev;
  std::string value;
  Cells::Cell cell;

  //std::cout << "\n--------Init Cells-------------\n";
  /*
  std::shared_ptr<SWC::Stats::Stat> latency_vector = std::make_shared<SWC::Stats::Stat>();
  int64_t ts_total_vector = 0;
  int64_t took_vector;
  std::vector<Cells::Cell*> cells;
  
  std::shared_ptr<SWC::Stats::Stat> latency = std::make_shared<SWC::Stats::Stat>();
  int64_t ts_total = 0;
  int64_t took;
  Cells::Chain::Ptr cells_chain  = std::make_shared<Cells::Chain>(1);
  */

  std::shared_ptr<SWC::Stats::Stat> latency_mutable = std::make_shared<SWC::Stats::Stat>();
  int64_t ts_total_mutable = 0;
  int64_t took_mutable;
  Cells::Mutable::Ptr cells_mutable = Cells::Mutable::make(
    1, max_versions, 0, typ);

  int truclations = 0;
  for(auto r=1;r<=num_revs;r++){
  for(auto i=(reverse?num_cells:1);(reverse?i>=1:i<=num_cells);(reverse?i--:i++)){
    
      std::string n = std::to_string(i);
      
      rev = SWC::Time::now_ns()-(gen_historic?r*1000000:0);
      cell.flag = Cells::INSERT;
      cell.set_timestamp(rev-1);
      cell.set_revision(rev);
      cell.set_time_order_desc(time_order_desc);

      cell.key.free();
      for(int f=1;f<=4;f++)
        cell.key.add("F"+n+"-"+std::to_string(f));
      if(counter) {
        if(r == num_revs-2) {
          cell.set_value(Cells::OP::EQUAL, num_revs);
          truclations++;
        } else
          cell.set_value(Cells::OP::PLUS, 1);
      
      } else {
        value = std::string("A-Data-Value-1234567890-"+n);
        cell.set_value(value.data(), value.length());
      }

      /*
      took_vector = SWC::Time::now_ns();

      //cells.insert(cells.begin(), new Cells::Cell(cell));
      cells.push_back(new Cells::Cell(cell));

      took_vector = SWC::Time::now_ns()-took_vector;
      ts_total_vector += took_vector;
      latency_vector->add(took_vector); 
      
      ///
      took = SWC::Time::now_ns();

      cells_chain->add(cell);

      took = SWC::Time::now_ns()-took;
      ts_total += took;
      latency->add(took); 
      */
      
      took_mutable = SWC::Time::now_ns();

      //cells_mutable->insert(0, cell);
      cells_mutable->add(cell);
      //cells_mutable->push_back(cell);

      took_mutable = SWC::Time::now_ns()-took_mutable;
      ts_total_mutable += took_mutable;
      latency_mutable->add(took_mutable); 

      if(i % 1000000 == 0) {
        //std::cout << "v:took=" << took_vector  << " " << r << "/" << i << " = " << cells.size() << "\n";
        std::cout << "m:took=" << took_mutable << " " << r << "/" << i << " = " << cells_mutable->size() << "\n";
      }
  }
  }

  /*
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

  int expected_sz = num_cells;
  if(counter)
   expected_sz *= (truclations?2:1);
  else 
   expected_sz *= (max_versions > num_revs?num_revs:max_versions);


  if(cells_mutable->size() != expected_sz) {
    std::cerr << "SIZE NOT AS EXPECTED, "
              << "expected(" << expected_sz << ") != result(" << cells_mutable->size()  << ")\n";
    exit(1);
  }

  SWC::DB::Specs::Interval specs;
  //specs.flags.limit = 5;
  specs.flags.offset = 3;
  /*
  specs.key_start.add("F2-1", SWC::Condition::GE);
  specs.key_start.add("", SWC::Condition::NONE);
  specs.key_start.add("", SWC::Condition::NONE);
  specs.key_start.add("F6", SWC::Condition::PF);
  */
  //specs.key_start.add(".*-2$", SWC::Condition::RE);
  //specs.key_start.add("", SWC::Condition::NONE);
  if(counter)
    specs.value.set(10,  SWC::Condition::EQ);
  else
    specs.value.set("A-Data-Value-1234567890-9",  SWC::Condition::GT);

  SWC::DynamicBuffer result;
  size_t count=0; 
  size_t skips = 0;
  cells_mutable->scan(specs, result, count, skips);

  size_t remain = result.fill();
  const uint8_t* bptr = result.base;
  int counted = 0;
  while(remain) {
    cell.read(&bptr, &remain);
    if(++counted <= 20)
      std::cout << cell.to_string(typ) << "\n";
  }
  std::cout << " skips=" << skips << " count="<< count << "\n";


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


  std::cout << " clearing, ";
  
  //cells_mutable->free();
  cells_mutable = nullptr;
  
  /*
  for(auto & cell : cells)
    delete cell;
  cells.clear();
  */
  std::cout << " cleared check with cells=" << num_cells << "\n";

}
  
int main(int argc, char** argv) {
  
  //check(1);

  check(SWC::Types::Column::PLAIN, 11, 3);

  check(SWC::Types::Column::PLAIN, 11, 3, true);
  check(SWC::Types::Column::PLAIN, 11, 3, true, true);
  check(SWC::Types::Column::PLAIN, 11, 3, true, 2, false);
  check(SWC::Types::Column::PLAIN, 11, 3, true, 2, true, true);
  check(SWC::Types::Column::PLAIN, 11, 10000, true, 2, true);

  check(SWC::Types::Column::COUNTER_I64, 10, 10, true, 2, true, false);
  check(SWC::Types::Column::COUNTER_I64, 10, 10, true, 2, true, true);
  /*
  check(100);
  check(1000);
  check(10000);

  check(100000);
  check(100000);
  check(100000);
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  
  check(1000000);
  std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  
  for(auto i=1; i<=10; i++) {
    check(100000000);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  }
  */

  //for(auto i=1; i<=10000000; i*=10)
  //  check(i);

  exit(0);
}