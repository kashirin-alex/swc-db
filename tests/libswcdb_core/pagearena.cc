/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/PageArena.h"
#include <vector>
#include <iostream>


int main() {
  size_t num_items = 1000;
  size_t num_uses = 1000;
  auto& arena = SWC::Env::PageArena;


  std::cout << "sizeof:" << "\n";
  std::cout << " Arena=" << sizeof(SWC::Core::Mem::Arena) << "\n";
  std::cout << " arena=" << sizeof(arena) << "\n";
  std::cout << " Item=" << sizeof(SWC::Core::Mem::Item) << "\n";
  std::cout << " ItemPtr=" << sizeof(SWC::Core::Mem::ItemPtr) << "\n";

  std::vector<SWC::Core::Mem::ItemPtr> data;

  for(size_t n=0; n<num_uses;++n) {
    std::cout << " set-"<< n << " begin\n";
    for(size_t i = num_items; i>0;--i) {
      std::string s(std::to_string(i));
      data.push_back(
        SWC::Core::Mem::ItemPtr(
          reinterpret_cast<const uint8_t*>(s.c_str()), s.length()));
    }
    std::cout << " set-"<< n << " end\n";
  }

  std::cout << " num_items="<< num_items  << std::flush
            << " arena.count()= " << arena.count()  << std::flush
            << " data.size()= " << data.size() << "\n";

  SWC_ASSERT(arena.count() == num_items);

  for(auto idx=arena.pages() ; idx;) {
    for(auto item : arena.page(--idx)) {
      std::cout << item->to_string() << "=" << item->count << "\n";
      std::cout << "num_uses=" << num_uses << " p->count=" << item->count << "\n";
      SWC_ASSERT(item->count == num_uses);
    }
  }
  SWC_ASSERT(arena.count() == num_items);
  std::cout << " arena.count="<< arena.count() << "\n";
  std::cout << "\n";


  for(auto idx=arena.pages() ;idx;) {
    auto c = arena.page(--idx).count();
    if(c)
      std::cout << "set on page-idx=" << idx << " count="<< arena.page(idx).count() << "\n";
  }

  std::cout << " release  begin\n";
  for(auto arr : data) {
    arr.release();
  }
  std::cout << " release  end\n";


  SWC_ASSERT(!arena.count());

  std::cout << " OK! \n";
  return 0;
}
