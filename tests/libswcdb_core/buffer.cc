/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Buffer.h"

#include <cstring>
#include <string>


namespace {


void test_static_own_and_borrow() {
  const char raw[] = "hello-buffer";
  const size_t len = sizeof(raw) - 1;

  {
    SWC::StaticBuffer buf(len);
    SWC_ASSERT(buf.own);
    SWC_ASSERT(buf.size == len);
    SWC_ASSERT(buf.base != nullptr);
    std::memcpy(buf.base, raw, len);
  }

  {
    uint8_t* owned = new uint8_t[len];
    std::memcpy(owned, raw, len);
    SWC::StaticBuffer buf(owned, len, true);
    SWC_ASSERT(buf.own);
    SWC_ASSERT(buf.size == len);
    SWC_ASSERT(!std::memcmp(buf.base, raw, len));
  }

  {
    uint8_t stack[16];
    std::memcpy(stack, raw, len);
    SWC::StaticBuffer buf(stack, len, false);
    SWC_ASSERT(!buf.own);
    SWC_ASSERT(buf.size == len);
    SWC_ASSERT(buf.base == stack);
    buf.free();
    SWC_ASSERT(buf.base == nullptr);
    SWC_ASSERT(buf.size == 0);
  }
}


void test_assign_reallocate_move() {
  const uint8_t a[] = { 1, 2, 3, 4, 5 };
  SWC::StaticBuffer buf;
  buf.assign(a, sizeof(a));
  SWC_ASSERT(buf.own);
  SWC_ASSERT(buf.size == sizeof(a));
  SWC_ASSERT(!std::memcmp(buf.base, a, sizeof(a)));

  buf.reallocate(2);
  SWC_ASSERT(buf.size == 2);
  buf.base[0] = 9;
  buf.base[1] = 8;

  SWC::StaticBuffer moved(std::move(buf));
  SWC_ASSERT(moved.own);
  SWC_ASSERT(moved.size == 2);
  SWC_ASSERT(moved.base[0] == 9);
  SWC_ASSERT(moved.base[1] == 8);
  SWC_ASSERT(!buf.own);
  SWC_ASSERT(buf.base == nullptr);

  buf.reallocate(0);
  SWC_ASSERT(buf.size == 0);
  SWC_ASSERT(buf.base == nullptr);
}


void test_allocate_zero_throws() {
  bool threw = false;
  try {
    (void)SWC::StaticBuffer::allocate(0);
  } catch(const SWC::Error::Exception& e) {
    threw = true;
    SWC_ASSERT(e.code() == SWC::Error::BAD_MEMORY_ALLOCATION);
  }
  SWC_ASSERT(threw);
}


void test_dynamic_grow_and_static_take() {
  SWC::DynamicBuffer dyn;
  SWC_ASSERT(dyn.empty());
  SWC_ASSERT(dyn.fill() == 0);

  const std::string chunk1 = "abcdef";
  const std::string chunk2 = "0123456789";
  dyn.add(chunk1);
  SWC_ASSERT(dyn.fill() == chunk1.size());
  SWC_ASSERT(!dyn.empty());
  dyn.add(reinterpret_cast<const uint8_t*>(chunk2.data()), chunk2.size());
  SWC_ASSERT(dyn.fill() == chunk1.size() + chunk2.size());

  dyn.ensure(1024);
  SWC_ASSERT(dyn.remaining() >= 1024);
  dyn.add(uint8_t('Z'));
  SWC_ASSERT(dyn.fill() == chunk1.size() + chunk2.size() + 1);

  SWC::StaticBuffer taken(dyn);
  SWC_ASSERT(taken.size == chunk1.size() + chunk2.size() + 1);
  SWC_ASSERT(taken.own);
  SWC_ASSERT(dyn.base == nullptr);
  SWC_ASSERT(dyn.fill() == 0);

  SWC::DynamicBuffer dyn2(32);
  dyn2.add(chunk1);
  SWC::StaticBuffer set_into;
  set_into.set(dyn2);
  SWC_ASSERT(set_into.size == chunk1.size());
  SWC_ASSERT(!std::memcmp(set_into.base, chunk1.data(), chunk1.size()));
  SWC_ASSERT(dyn2.base == nullptr);
}


} // namespace


int main() {
  test_static_own_and_borrow();
  test_assign_reallocate_move();
  test_allocate_zero_throws();
  test_dynamic_grow_and_static_take();

  std::cout << "buffer OK\n";
  return 0;
}
