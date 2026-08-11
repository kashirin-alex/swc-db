/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Array.h"
#include "swcdb/core/ArraysArray.h"

#include <cstdint>


namespace {


void test_array_basic() {
  using Arr = SWC::Core::Array<uint32_t, uint8_t, 8>;
  Arr a;
  SWC_ASSERT(a.empty());
  SWC_ASSERT(!a.full());
  SWC_ASSERT(a.size() == 0);

  for(uint32_t n = 1; n <= 8; ++n)
    a.push_back(n);
  SWC_ASSERT(a.full());
  SWC_ASSERT(a.size() == 8);
  SWC_ASSERT(a.front() == 1);
  SWC_ASSERT(a.back() == 8);
  for(uint32_t i = 0; i < a.size(); ++i)
    SWC_ASSERT(a[i] == i + 1);

  a.erase(a.begin() + 2);
  SWC_ASSERT(a.size() == 7);
  SWC_ASSERT(a[2] == 4);

  a.insert(a.begin() + 2, uint32_t(3));
  SWC_ASSERT(a.size() == 8);
  SWC_ASSERT(a[2] == 3);
  SWC_ASSERT(a[3] == 4);
}


void test_array_move_split() {
  using Arr = SWC::Core::Array<uint32_t, uint8_t, 8>;
  Arr src;
  for(uint32_t n = 1; n <= 8; ++n)
    src.push_back(n);

  auto it = src.begin() + 5;
  Arr split(std::move(src), it);
  SWC_ASSERT(split.size() == 3);
  SWC_ASSERT(split[0] == 6);
  SWC_ASSERT(split[1] == 7);
  SWC_ASSERT(split[2] == 8);
  SWC_ASSERT(src.size() == 5);
  SWC_ASSERT(src.back() == 5);
}


void test_arrays_array() {
  using ItemArr = SWC::Core::Array<uint32_t, uint8_t, 4>;
  using Nested = SWC::Core::ArraysArray<ItemArr, uint8_t, 8>;

  Nested nested;
  SWC_ASSERT(nested.empty());
  SWC_ASSERT(nested.count() == 0);

  for(uint32_t n = 1; n <= 10; ++n)
    nested.push_back(n);

  SWC_ASSERT(nested.count() == 10);
  SWC_ASSERT(nested.size() >= 2);

  uint32_t expect = 1;
  for(auto it = nested.GetConstIterator(); it; ++it, ++expect)
    SWC_ASSERT(it.item() == expect);
  SWC_ASSERT(expect == 11);
}


} // namespace


int main() {
  test_array_basic();
  test_array_move_split();
  test_arrays_array();

  std::cout << "array OK\n";
  return 0;
}
