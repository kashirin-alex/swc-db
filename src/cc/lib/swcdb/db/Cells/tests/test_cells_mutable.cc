/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include <thread>
#include <iostream>
#include <cstring>
#include <vector>

#include "swcdb/core/Time.h"
#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/db/client/Stats/Stat.h"

#include "swcdb/core/config/Settings.h"
namespace SWC { namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}}


namespace Cells = SWC::DB::Cells;

void op(Cells::Mutable::Ptr cells_mutable, int& truclations, int64_t& ts_total, std::shared_ptr<SWC::Stats::Stat> latency_mutable,
        int num_revs, bool reverse, int num_cells, bool gen_historic, Cells::Flag flag, SWC::Types::Column typ, bool time_order_desc) {

  Cells::Cell cell;

  int64_t rev;
  int64_t took;
  std::string cell_number;
  
  for(auto r=1;r<=(flag==Cells::DELETE ? 1 : num_revs);r++){
  for(auto i=(reverse?num_cells:1);(reverse?i>=1:i<=num_cells);(reverse?i--:i++)){
      cell_number = std::to_string(i);
      
      rev = SWC::Time::now_ns()-(gen_historic?r*1000000:0);
      cell.flag = flag;
      cell.set_timestamp(rev-1);
      //cell.set_revision(rev);
      cell.set_time_order_desc(time_order_desc);

      cell.key.free();
      for(uint8_t chr=97; chr<=122;chr++)
        cell.key.add(((char)chr)+cell_number);

      if(cell.flag == Cells::INSERT) {
        if(SWC::Types::is_counter(typ)) {
          if(r == num_revs-2) {
            cell.set_counter(Cells::OP_EQUAL, num_revs, typ);
            truclations++;
          } else
            cell.set_counter(0, 1);
      
        } else {
          cell.set_value("V_OF: "+cell_number);
        }
      }

      
      took = SWC::Time::now_ns();

      //cells_mutable->insert(0, cell);
      cells_mutable->add_raw(cell);
      //cells_mutable->push_back(cell);

      took = SWC::Time::now_ns()-took;
      ts_total += took;
      latency_mutable->add(took); 

      if(i % 1000000 == 0) {
        //std::cout << "v:took=" << took_vector  << " " << r << "/" << i << " = " << cells.size() << "\n";
        std::cout << "   m:took=" << took << " " << r << "/" << i << " = " << cells_mutable->size() << "\n";
      }
  }
  }
}



void check(SWC::Types::Column typ, size_t num_cells = 1, int num_revs = 1, int max_versions = 1, 
           bool reverse=false, bool time_order_desc=false, bool gen_historic=false) {
  std::cout << "\nchecking with type=" << SWC::Types::to_string(typ) 
                           << " cells=" << num_cells 
                           << " revs=" << num_revs 
                           << " reverse=" << reverse
                           << " max_versions=" << max_versions 
                           << " time_order_desc=" << time_order_desc 
                           << " gen_historic=" << gen_historic 
                           << "\n";
  int truclations = 0;

  std::shared_ptr<SWC::Stats::Stat> latency_mutable(std::make_shared<SWC::Stats::Stat>());
  int64_t ts_total = 0;

  Cells::Mutable::Ptr cells_mutable(Cells::Mutable::make(max_versions, 0, typ));

  op(cells_mutable, truclations, ts_total, latency_mutable,
     num_revs, reverse, num_cells, gen_historic, Cells::INSERT, typ, time_order_desc);
  /// 

  int expected_sz = num_cells;
  if(SWC::Types::is_counter(typ))
   expected_sz *= (truclations?2:1);
  else 
   expected_sz *= (max_versions > num_revs?num_revs:max_versions);


  if(cells_mutable->size() != expected_sz) {
    std::cerr << "INSERT SIZE NOT AS EXPECTED, "
              << "expected(" << expected_sz << ") != result(" << cells_mutable->size()  << ")\n";
    exit(1);
  }

  std::cout << " mutable (add)"
            << " num="  << latency_mutable->count()
            << " counted=" << cells_mutable->size()
            << " avg="    << latency_mutable->avg()
            << " min="    << latency_mutable->min()
            << " max="    << latency_mutable->max()
            << " median=" << ts_total / num_cells
            << " took="   << ts_total / 1000000 << "ms"
            << "\n"; 



  SWC::DB::Specs::Interval specs;
  //specs.flags.limit = 5;
  specs.flags.offset = max_versions;
  /*
  specs.key_start.add("F2-1", SWC::Condition::GE);
  specs.key_start.add("", SWC::Condition::NONE);
  specs.key_start.add("", SWC::Condition::NONE);
  specs.key_start.add("F6", SWC::Condition::PF);
  */
  //specs.key_start.add(".*-2$", SWC::Condition::RE);
  //specs.key_start.add("", SWC::Condition::NONE);
  if(SWC::Types::is_counter(typ))
    specs.value.set(10,  SWC::Condition::EQ);
  else
    specs.value.set("V_OF: "+std::to_string(num_cells-3),  SWC::Condition::GE);
  specs.flags.max_versions=max_versions;

  Cells::Cell cell;
  SWC::DynamicBuffer result;
  size_t count = 0; 
  size_t skips = 0;
      
  int64_t took = SWC::Time::now_ns();
  cells_mutable->scan_test_use(specs, result, count, skips);
  took = SWC::Time::now_ns()-took;

  size_t remain = result.fill();
  const uint8_t* bptr = result.base;
  int counted = 0;
  while(remain) {
    cell.read(&bptr, &remain);
    //std::cout << cell.to_string() <<"\n";
    counted++;
  }
  if(count != 3*max_versions) {
    std::cout << " skips=" << skips << " count="<< count << " counted=" << counted << " \n";
    std::cerr << "\n" << cells_mutable->to_string(true);
    std::cerr << "\nBad scan, expected=" << 3*max_versions << " result=" << count << "\n";
    std::cerr << specs.to_string() <<"\n";
    exit(1);
  }
  std::cout << " mutable (scan)"
            << " num="  << cells_mutable->size()
            << " avg="  << took / cells_mutable->size()
            << " took=" << took / 1000000 << "ms"
            << "\n"; 


  latency_mutable = std::make_shared<SWC::Stats::Stat>();
  ts_total = 0;
  truclations = 0;
  op(cells_mutable, truclations, ts_total, latency_mutable,
     num_revs, reverse, num_cells, gen_historic, Cells::DELETE, typ, time_order_desc);
  /// 

  if(cells_mutable->size() != num_cells) {
    std::cerr << "\n" << cells_mutable->to_string(true);
    std::cerr << "\nDELETE SIZE NOT AS EXPECTED, "
              << "expected(" << num_cells << ") != result(" << cells_mutable->size()  << ")\n";
    exit(1);
  }
  
  Cells::Vector results(max_versions, 0, typ);
  size_t cell_offset = 0;
  size_t cell_skips = 0;
  auto spec = SWC::DB::Specs::Interval();
  cells_mutable->scan(
    spec, 
    results, cell_offset, 
    [](){return false;}, 
    cell_skips, 
    [spec, typ](const SWC::DB::Cells::Cell& cell, bool& stop) {
                return spec.is_matching(cell, typ);}
    );

  if(results.size() != 0) {
    std::cerr << "AFTER DELETE SIZE NOT AS EXPECTED, "
              << "expected(" << 0 << ") != result(" << results.size()  << ")\n";
    exit(1);
  }
  std::cout << " mutable (del)"
            << " num="  << latency_mutable->count()
            << " counted=" << cells_mutable->size()
            << " avg="    << latency_mutable->avg()
            << " min="    << latency_mutable->min()
            << " max="    << latency_mutable->max()
            << " median=" << ts_total / num_cells
            << " took="   << ts_total / 1000000 << "ms"
            << "\n"; 

  std::cout << " clearing, ";
  
  //cells_mutable->free();
  cells_mutable = nullptr;
  
  std::cout << " cleared check with cells=" << num_cells << "\n";

}
  








int main(int argc, char** argv) {
  /*
  check(SWC::Types::Column::PLAIN, 10, 1);
  check(SWC::Types::Column::PLAIN, 10, 3, 2);

  check(SWC::Types::Column::PLAIN, 100000, 1);
  check(SWC::Types::Column::PLAIN, 100000, 3, 2);
  */
  check(SWC::Types::Column::PLAIN, 1000, 1);
  check(SWC::Types::Column::PLAIN, 1000, 3, 2);
  check(SWC::Types::Column::PLAIN, 1000, 10, 8);

  check(SWC::Types::Column::PLAIN, 10000, 1);
  check(SWC::Types::Column::PLAIN, 10000, 3, 2);
  check(SWC::Types::Column::PLAIN, 10000, 10, 8);

  check(SWC::Types::Column::PLAIN, 100000, 1);
  check(SWC::Types::Column::PLAIN, 100000, 3, 2);
  check(SWC::Types::Column::PLAIN, 100000, 10, 8);

  check(SWC::Types::Column::PLAIN, 200000, 1);
  check(SWC::Types::Column::PLAIN, 200000, 3, 2);
  check(SWC::Types::Column::PLAIN, 200000, 10, 8);
  
  //check(SWC::Types::Column::PLAIN, 1000000, 1);
  //check(SWC::Types::Column::PLAIN, 1000000, 3, 2);

  //check(SWC::Types::Column::PLAIN, 2000000, 1);
  //check(SWC::Types::Column::PLAIN, 2000000, 3, 2);

  //check(SWC::Types::Column::PLAIN, 10000000, 1);
  //check(SWC::Types::Column::PLAIN, 20000000, 3);


  /*
  check(SWC::Types::Column::PLAIN, 11, 3, true);
  check(SWC::Types::Column::PLAIN, 11, 3, true, true);
  check(SWC::Types::Column::PLAIN, 11, 3, true, 2, false);
  check(SWC::Types::Column::PLAIN, 11, 3, true, 2, true, true);
  check(SWC::Types::Column::PLAIN, 11, 10000, true, 2, true);

  check(SWC::Types::Column::COUNTER_I64, 10, 10, true, 2, true, false);
  check(SWC::Types::Column::COUNTER_I64, 10, 10, true, 2, true, true);
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