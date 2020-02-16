/**
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include <vector>
#include <cassert>
#include <iostream>

#include "swcdb/core/PageArena.h"

int main() {
  size_t num_items = 1000;
  size_t num_uses = 1000;
  auto& page = SWC::Env::PageArena;
    
  std::cout << "sizeof:" << "\n";
  std::cout << " Arena=" << sizeof(SWC::Mem::Arena) << "\n";
  std::cout << " page=" << sizeof(page) << "\n";
  std::cout << " Item=" << sizeof(SWC::Mem::Item) << "\n";
  std::cout << " ItemPtr=" << sizeof(SWC::Mem::ItemPtr) << "\n";
    
  std::vector<SWC::Mem::ItemPtr> data;
    
  for(size_t n=0; n<num_uses;++n) {
    std::cout << " set-"<< n << " begin\n";
    for(size_t i = num_items; i>0;--i) {
      std::string s(std::to_string(i));
      data.push_back(SWC::Mem::ItemPtr((const uint8_t*)s.data(), s.length()));
    }
    std::cout << " set-"<< n << " end\n";
  }

  std::cout << " num_items="<< num_items  
            << " page.size()= " << page.size()
            << " data.size()= " << data.size() << "\n";
    
  assert(page.size() == num_items);

  for(const auto& p : page) {
    std::cout << p->to_string() << "=" << p->count << "\n";
    std::cout << "num_uses=" << num_uses << " p->count=" << p->count << "\n";
    assert(p->count == num_uses);
  }

  std::cout << " release  begin\n";
  for(auto arr : data) {
    arr.release();
  }
  std::cout << " release  end\n";
        
        
  assert(page.empty());

  return 0;
}
