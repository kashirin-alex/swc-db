/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/Interval.h"
#include "swcdb/core/config/Settings.h"


namespace SWC { namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}}


namespace DB = SWC::DB;
namespace Condition = SWC::Condition;

int main() {
  int num_keys = 9999;
  DB::Cells::Interval expected_expanded(SWC::DB::Types::KeySeq::VOLUME);
  DB::Cells::Interval interval(SWC::DB::Types::KeySeq::VOLUME);
  DB::Cells::Cell cell; 

  for (int n = 0; n<=num_keys ;++n) {
      cell.key.free();

      auto s = std::to_string(num_keys-n);
      cell.key.add("11");
      cell.key.add(std::string("a")+s);
      cell.key.add(std::string("b")+s);
      cell.key.add(std::string("c")+s);
      cell.key.add(std::string("d")+s);
      cell.timestamp = n;

      interval.expand(cell);    
      interval.expand(cell.timestamp);  
      interval.align(cell.key);


      if(!n) {
        expected_expanded.set_key_end(cell.key);
        expected_expanded.set_ts_earliest(DB::Specs::Timestamp(n, Condition::GE));
        DB::Cell::KeyVec k;
        cell.key.convert_to(k);
        expected_expanded.set_aligned_max(k);
      }
      if(n == num_keys) {
        expected_expanded.set_key_begin(cell.key);
        expected_expanded.set_ts_latest(DB::Specs::Timestamp(n, Condition::LE));
        DB::Cell::KeyVec k;
        cell.key.convert_to(k);
        expected_expanded.set_aligned_min(k);
      }
  }
  std::cout << " init OK\n";
  std::cout << "\n";

  if(!expected_expanded.equal(interval)){
    std::cout << " expected,  " << expected_expanded.to_string() << "\n";
    std::cout << " resulting, " << interval.to_string() << "\n";
    std::cerr << "!expected_expanded.equal(interval)): ERROR\n";
    exit(1);
  }
  exit(0);
}