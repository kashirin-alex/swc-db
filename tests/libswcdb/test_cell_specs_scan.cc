/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */
#include <iostream>

#include "swcdb/db/Cells/SpecsScan.h"

#include "swcdb/core/config/Settings.h"
namespace SWC { namespace Config {
void Settings::init_app_options() {}
void Settings::init_post_cmd_args() {}
}}


namespace Specs = SWC::DB::Specs;
namespace Condition = SWC::Condition;


void test_encode_decode(const Specs::Scan& ss){
  
  size_t len = ss.encoded_length_internal();
  
  uint8_t* base = new uint8_t[len];
  const uint8_t* mark1 = base;
  uint8_t* ptr = base;

  ss.encode_internal(&ptr);
  if(ptr-base != len) {
    std::cout << "\n Encode/Decode \n";
    std::cout << "ERROR, encode wrote less than expected\n";
    exit(1);    
  }

  size_t remain = len;
  const uint8_t* ptr1 = (const uint8_t*)base;
  Specs::Scan ss_decoded(&ptr1, &remain);
  if(remain > 0 || ptr1 != base+len) {
    std::cout << "\n Encode/Decode \n";
    std::cout << "ERROR, remain-len remain=" << remain 
              << " ptr~=" << ptr1-(base+len) << "\n";
    exit(1);
  }
        
  if(!ss_decoded.equal(ss)) {
    std::cout << "\n Encode/Decode \n";
    std::cout << "!ss.equal(ss_decoded): ERROR\n";
    exit(1);
  }

  //col_decoded.cid = 444;
        
  len = ss_decoded.encoded_length_internal();
  uint8_t* base2 = new uint8_t[len];
  const uint8_t* mark2 = base2;
  uint8_t* ptr2 = base2;
  ss_decoded.encode_internal(&ptr2);
        
  if(memcmp(mark1, mark2, len) != 0){
    std::cout << "\nERROR, encoding mismatch (memcmp) \n";
    std::cout << Specs::Scan(&mark2, &len).to_string() << "\n\n";
    std::cout << "data:\"" << std::string((const char*)mark1, len) << "\"\n";
    std::cout << "!==\n";
    std::cout << "data:\"" << std::string((const char*)mark2, len) << "\"\n";
    exit(1);
  }

  delete [] base;
  delete [] base2;
        
}



void test(int chk) {

    Specs::Scan ss = Specs::Scan();
    ss.flags.limit = 111;
    ss.flags.offset = 222;
    ss.flags.max_versions = 2;
    
    const char * a1 = "a1";
    const char * a2 = "a2";
    const char * a3 = "a3";
    const char * a4 = "a4";
    const char * b1 = "b1";
    const char * b2 = "b2";
    const char * b3 = "b3";
    const char * b4 = "b4";
    std::string a_string_v = std::string("A-Value1");
    const char * va1 = a_string_v.c_str();
    const char * va2 = "A-Value2";
    const char * vb1 = "B-Value1";
    const char * vb2 = "B-Value2";

    int64_t ts1 = 111;
    int64_t ts2 = 112;
    int64_t ts3 = 113;
    int64_t ts4 = 114;
    int64_t ts5 = 115;
    int64_t ts6 = 116;
    int64_t ts7 = 117;
    int64_t ts8 = 118;
    
    Specs::Key key_start;
    key_start.add(a1, Condition::EQ);
    key_start.add(a2, 2, Condition::EQ);
    //std::cout << "- " << key_start.to_string() << "\n";
    
    Specs::Key key_finish;
    key_finish.add(a3, Condition::EQ);
    key_finish.add(a4, Condition::EQ);
    //std::cout << "- " << key_finish.to_string() << "\n";

    Specs::Column::Ptr cs_is_1 = Specs::Column::make_ptr(5555, 1);
    cs_is_1->intervals.push_back(
      Specs::Interval::make_ptr(
        key_start,
        key_finish,
        Specs::Value(va1, Condition::EQ),
        Specs::Timestamp(ts1, Condition::EQ),
        Specs::Timestamp(ts2, Condition::EQ),
        ss.flags
      )
    );
    //std::cout << "- " << cs_is_1->to_string() << "\n";
    
    key_start.add(a3, Condition::EQ);
    key_start.add(a4, Condition::EQ);
    key_finish.add(a1, 2, Condition::EQ);
    key_finish.add(a2, 2, Condition::EQ);
    cs_is_1->intervals.push_back(
      Specs::Interval::make_ptr(
        key_start,
        key_finish,
        Specs::Value(va2, Condition::EQ),
        Specs::Timestamp(ts3, Condition::EQ),
        Specs::Timestamp(ts4, Condition::EQ),
        ss.flags
      )
    );
    //std::cout << "- " << cs_is_1->to_string() << "\n";

    ss.columns.push_back(cs_is_1);
    //std::cout << "- " << ss.to_string() << "\n";
    
    Specs::Column::Ptr cs_is_2 = Specs::Column::make_ptr(11111, 1);
    cs_is_2->intervals.push_back(
      Specs::Interval::make_ptr(
        key_start,
        key_finish,
        Specs::Value(vb1, Condition::EQ),
        Specs::Timestamp(ts5, Condition::EQ),
        Specs::Timestamp(ts6, Condition::EQ)
      )
    );
    //std::cout << "- " << cs_is_2->to_string() << "\n\n";
        
    Specs::Key key_start2;
    key_start2.add(b1, Condition::EQ);
    key_start2.add(b2, 2, Condition::EQ);

    Specs::Key key_finish2;
    key_start2.add(b3, Condition::EQ);
    key_start2.add(b4, Condition::EQ);

    cs_is_2->intervals.push_back(
      Specs::Interval::make_ptr(
        key_start2,
        key_finish2,
        Specs::Value(vb2, Condition::EQ),
        Specs::Timestamp(ts7, Condition::EQ),
        Specs::Timestamp(ts8, Condition::EQ)
      )
    );
    ss.columns.push_back(cs_is_2);

    //std::cout << "- " << ss.to_string() << "\n\n";

    
    if(!ss.equal(ss)) {
      std::cout << "\n!ss.equal(ss): ERROR\n";
      exit(1);
    }
    Specs::Scan ss_copy;
    ss_copy.copy(ss);
    if(!ss.equal(ss_copy)) {
      std::cout << "\nInit from\n";
      std::cout << "!ss.equal(ss_copy): ERROR\n\n";
      std::cout << " old - " << ss.to_string() << "\n";
      std::cout << " new - " << ss_copy.to_string() << "\n";
      exit(1);
    }
    if(&ss.columns[0]->intervals[0]->key_start 
        == &ss_copy.columns[0]->intervals[0]->key_start) {
      std::cout << "\ncopy key.data ptr equal: ERROR\n";
      exit(1);
    }
    //std::cout << "\n OK \n";


    Specs::Scan passed_ss2;
    passed_ss2 = ss; // the same ptr instances

    if(!ss.equal(passed_ss2)) {
      std::cout << "\nAssign from\n";
      std::cout << " old - " << ss.to_string() << "\n";
      std::cout << " new - " << passed_ss2.to_string() << "\n";
      std::cout << "!ss.equal(passed_ss2): ERROR\n";
      exit(1);
    }
    if(&ss.columns[0]->intervals[0]->key_start
        != &passed_ss2.columns[0]->intervals[0]->key_start) {
      std::cout << "\nassign key.data ptr not equal: ERROR\n";
      exit(1);
    }

    // sanity - no changes happened at 'equal'
    if(!ss_copy.equal(passed_ss2)) {
      std::cout << "\n!ss_copy.equal(passed_ss2): ERROR\n";
      exit(1);
    }
    if(!ss_copy.equal(ss)) {
      std::cout << "\n!ss_copy.equal(passed_ss2): ERROR\n";
      exit(1);
    }

    //std::cout << "\n OK \n";
    



    for(size_t i=chk;i>0;--i){
      //std::cout << "try:" << i << "\n\n";
      //test_encode_decode(ss);
    }

    std::cout << " chk=" << chk;
    
}

int main() {
  for(size_t i=10000;i>0;--i)
    test(i);

  std::cout << "\n OK \n";
  exit(0);
}