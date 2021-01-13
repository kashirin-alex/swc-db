/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include <thread>
#include <iostream>
#include <cstring>
#include <vector>

#include "swcdb/core/Time.h"
#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/common/Stats/Stat.h"

#include "swcdb/core/config/Settings.h"
namespace SWC { namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}}


namespace Cells = SWC::DB::Cells;

void op(Cells::Mutable::Ptr cells_mutable, 
        int& truclations, int64_t& ts_total, 
        std::shared_ptr<SWC::Common::Stats::Stat> latency_mutable,
        int num_revs, bool reverse, int num_cells, 
        bool gen_historic, Cells::Flag flag, 
        SWC::DB::Types::Column typ, bool time_order_desc) {

  Cells::Cell cell;

  int64_t rev;
  int64_t took;
  std::string cell_number;
  
  for(auto r=1;r<=(flag==Cells::DELETE ? 1 : num_revs);++r){
  for(auto i=(reverse?num_cells:1);(reverse?i>=1:i<=num_cells);(reverse?--i:++i)){
      cell_number = std::to_string(i);
      
      rev = SWC::Time::now_ns()-(gen_historic?r*1000000:0);
      cell.flag = flag;
      cell.set_timestamp(rev-1);
      //cell.set_revision(rev);
      cell.set_time_order_desc(time_order_desc);

      cell.key.free();
      for(uint8_t chr=97; chr<=122;++chr)
        cell.key.add(((char)chr)+cell_number);

      if(cell.flag == Cells::INSERT) {
        if(SWC::DB::Types::is_counter(typ)) {
          if(r == num_revs-2) {
            cell.set_counter(Cells::OP_EQUAL, num_revs, typ);
            ++truclations;
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

      if(!(i % 1000000)) {
        //std::cout << "v:took=" << took_vector  << " " << r << "/" << i << " = " << cells.size() << "\n";
        std::cout << "   m:took=" << took << " " << r << "/" << i 
                  << " = " << cells_mutable->size() << "\n";
      }
  }
  }
}



void check(SWC::DB::Types::KeySeq key_seq, SWC::DB::Types::Column typ, 
           size_t num_cells = 1, int num_revs = 1, int max_versions = 1, 
           bool reverse=false, bool time_order_desc=false, 
           bool gen_historic=false) {
  std::cout << "\nchecking with seq=" << SWC::DB::Types::to_string(key_seq) 
                           << " type=" << SWC::DB::Types::to_string(typ) 
                           << " cells=" << num_cells 
                           << " revs=" << num_revs 
                           << " reverse=" << reverse
                           << " max_versions=" << max_versions 
                           << " time_order_desc=" << time_order_desc 
                           << " gen_historic=" << gen_historic 
                           << "\n";
  int truclations = 0;

  auto latency_mutable(std::make_shared<SWC::Common::Stats::Stat>());
  int64_t ts_total = 0;

  Cells::Mutable::Ptr cells_mutable(
    Cells::Mutable::make(
      key_seq,
      max_versions, 0, typ));

  op(cells_mutable, truclations, ts_total, latency_mutable,
     num_revs, reverse, num_cells, gen_historic, 
     Cells::INSERT, typ, time_order_desc);
  /// 

  size_t expected_sz = num_cells;
  if(SWC::DB::Types::is_counter(typ))
   expected_sz *= 1;
  else 
   expected_sz *= (max_versions > num_revs ? num_revs : max_versions);


  if(cells_mutable->size() != expected_sz) {
    cells_mutable->print(std::cerr << "\n", true);
    std::cerr << "INSERT SIZE NOT AS EXPECTED, "
              << "expected(" << expected_sz 
              << ") != result(" << cells_mutable->size()  << ")\n";
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
  specs.value.col_type = typ;
  //specs.flags.limit = 5;
  specs.flags.offset = 0;
  /*
  specs.key_start.add("F2-1", SWC::Condition::GE);
  specs.key_start.add("", SWC::Condition::NONE);
  specs.key_start.add("", SWC::Condition::NONE);
  specs.key_start.add("F6", SWC::Condition::PF);
  */
  //specs.key_start.add(".*-2$", SWC::Condition::RE);
  //specs.key_start.add("", SWC::Condition::NONE);
  if(SWC::DB::Types::is_counter(typ)) {
    specs.value.set_counter(0,  SWC::Condition::EQ);
  } else {
    specs.value.set(
      "V_OF: "+std::to_string(num_cells-3),  SWC::Condition::VGT);
  }
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
    //std::cout << cell.to_string(typ) <<"\n";
    //std::cout << "v='" << std::string((const char*)cell.value, cell.vlen) <<"'\n";
    ++counted;
  }
  if(SWC::DB::Types::is_counter(typ) 
      ? count == num_cells : count != (size_t)3*max_versions ) {
    //cells_mutable->print(std::cerr << "\n", true);
    std::cout << " skips=" << skips 
              << " count="<< count << " counted=" << counted << " \n";
    std::cerr << "\nBad scan, expected=" << 3*max_versions 
              << " result=" << count << "\n";
    std::cerr << specs.to_string() <<"\n";
    exit(1);
  }
  std::cout << " mutable (scan)"
            << " num="  << cells_mutable->size()
            << " avg="  << took / cells_mutable->size()
            << " took=" << took / 1000000 << "ms"
            << "\n"; 


  latency_mutable = std::make_shared<SWC::Common::Stats::Stat>();
  ts_total = 0;
  truclations = 0;
  op(cells_mutable, truclations, ts_total, latency_mutable,
     num_revs, reverse, num_cells, gen_historic, 
     Cells::DELETE, typ, time_order_desc);
  /// 

  if(cells_mutable->size() != num_cells) {
    cells_mutable->print(std::cerr << "\n", true);
    std::cerr << "\nDELETE SIZE NOT AS EXPECTED, "
              << "expected(" << num_cells 
              << ") != result(" << cells_mutable->size()  << ")\n";
    exit(1);
  }
  
  SWC::DB::Cells::ReqScanTest req;
  req.spec.flags.max_versions = max_versions;
  req.cells.configure(max_versions, 0, typ);
  cells_mutable->scan(&req);
  req.profile.finished();

  if(req.cells.size()) {
    std::cerr << "AFTER DELETE SIZE NOT AS EXPECTED, "
              << "expected(" << 0 
              << ") != result(" <<req.cells.size()  << ")\n";
    exit(1);
  }
  std::cout << " mutable (del)"
            << " num="  << latency_mutable->count()
            << " counted=" << cells_mutable->size()
            << " avg="    << latency_mutable->avg()
            << " min="    << latency_mutable->min()
            << " max="    << latency_mutable->max()
            << " median=" << ts_total / num_cells
            << " took="   << ts_total / 1000000 << "ms ";       
  req.profile.print(std::cout); 
  std::cout << '\n'; 

  //std::cout << " clearing, ";
  
  //cells_mutable->free();
  cells_mutable = nullptr;
  
  //std::cout << " cleared check with cells=" << num_cells << "\n";

}
  








int main() {

  std::vector<size_t> cell_numbers = {
    1000,
    10000,
    100000,
    200000
  };

  std::vector<size_t> versions = {
    1,
    3,
    10
  };

  std::vector<size_t> max_versions = {
    1,
    2,
    8
  };

  std::vector< SWC::DB::Types::Column> column_types = {
    SWC::DB::Types::Column::PLAIN,
    SWC::DB::Types::Column::COUNTER_I64
  };

  std::vector<SWC::DB::Types::KeySeq> sequences = {
    SWC::DB::Types::KeySeq::LEXIC,
    SWC::DB::Types::KeySeq::VOLUME,
    SWC::DB::Types::KeySeq::FC_LEXIC,
    SWC::DB::Types::KeySeq::FC_VOLUME
  };


  for(auto column_type : column_types) {
    for(auto cell_number : cell_numbers) {
      for(auto version : versions) {
        for(auto max_version : max_versions) {
          if(SWC::DB::Types::is_counter(column_type) && max_version != 1)
            continue;
          else if(version < max_version)
            continue;
          for(auto seq : sequences) {
            check(seq, column_type, cell_number, version, max_version);
          }
        }
      }
    }
  }

  // ++ bool reverse=false, bool time_order_desc=false, bool gen_historic=false) 
  /*
  check(SWC::DB::Types::Column::PLAIN, 11, 3, true);
  check(SWC::DB::Types::Column::PLAIN, 11, 3, true, true);
  check(SWC::DB::Types::Column::PLAIN, 11, 3, true, 2, false);
  check(SWC::DB::Types::Column::PLAIN, 11, 3, true, 2, true, true);
  check(SWC::DB::Types::Column::PLAIN, 11, 10000, true, 2, true);
  */

  exit(0);
}