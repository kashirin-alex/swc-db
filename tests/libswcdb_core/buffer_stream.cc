/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/BufferStream.h"
#include "swcdb/core/Encoder.h"

#include <cstring>
#include <string>
#include <vector>


namespace {


std::vector<uint8_t> make_bytes(size_t len) {
  std::vector<uint8_t> data(len);
  for(size_t i = 0; i < len; ++i)
    data[i] = uint8_t('a' + (i % 26));
  return data;
}


void test_plain_stream() {
  const size_t pre_alloc = 64;
  const size_t commit_size = 32;
  SWC::Core::BufferStreamOut stream(pre_alloc, commit_size);
  SWC_ASSERT(stream.error == SWC::Error::OK);
  SWC_ASSERT(stream.empty());

  auto part1 = make_bytes(20);
  stream.add(part1.data(), part1.size());
  SWC_ASSERT(!stream.full());
  SWC_ASSERT(stream.available() == part1.size());

  auto part2 = make_bytes(20);
  stream.add(part2.data(), part2.size());
  SWC_ASSERT(stream.full());
  SWC_ASSERT(stream.available() == part1.size() + part2.size());

  SWC::StaticBuffer out1;
  stream.get(out1);
  SWC_ASSERT(out1.size == part1.size() + part2.size());
  SWC_ASSERT(!std::memcmp(out1.base, part1.data(), part1.size()));
  SWC_ASSERT(!std::memcmp(out1.base + part1.size(), part2.data(), part2.size()));
  SWC_ASSERT(stream.empty());

  auto part3 = make_bytes(10);
  stream.add(part3.data(), part3.size());
  SWC::StaticBuffer out2;
  stream.get(out2);
  SWC_ASSERT(out2.size == part3.size());
  SWC_ASSERT(!std::memcmp(out2.base, part3.data(), part3.size()));
  SWC_ASSERT(stream.empty());
}


void test_zstd_on_add_stream() {
  const size_t pre_alloc = 256;
  const size_t commit_size = 128;
  SWC::Core::BufferStreamOut_ZSTD_OnAdd stream(0, pre_alloc, commit_size);
  SWC_ASSERT(stream.error == SWC::Error::OK);

  auto plain = make_bytes(512);
  stream.add(plain.data(), plain.size());
  SWC_ASSERT(stream.error == SWC::Error::OK);
  SWC_ASSERT(stream.full());

  SWC::StaticBuffer enc;
  stream.get(enc);
  SWC_ASSERT(stream.error == SWC::Error::OK);
  SWC_ASSERT(enc.size > 0);
  SWC_ASSERT(stream.empty());

  std::vector<uint8_t> decoded(plain.size());
  int err = SWC::Error::OK;
  SWC::Core::Encoder::decode(
    err, SWC::Core::Encoder::Type::ZSTD,
    enc.base, enc.size, decoded.data(), decoded.size());
  SWC_ASSERT(err == SWC::Error::OK);
  SWC_ASSERT(!std::memcmp(decoded.data(), plain.data(), plain.size()));
}


void test_stream_in_plain() {
  SWC::Core::BufferStreamIn stream;
  SWC_ASSERT(stream.error == SWC::Error::OK);
  SWC_ASSERT(stream.empty());

  auto part1 = make_bytes(12);
  auto part2 = make_bytes(8);
  SWC::StaticBuffer chunk1(part1.size());
  std::memcpy(chunk1.base, part1.data(), part1.size());
  SWC::StaticBuffer chunk2(part2.size());
  std::memcpy(chunk2.base, part2.data(), part2.size());

  stream.add(chunk1);
  SWC_ASSERT(!stream.empty());
  stream.add(chunk2);

  auto putback = make_bytes(4);
  stream.put_back(putback.data(), putback.size());

  SWC::StaticBuffer out;
  SWC_ASSERT(stream.get(out));
  SWC_ASSERT(out.size == part1.size() + part2.size() + putback.size());
  SWC_ASSERT(!std::memcmp(out.base, part1.data(), part1.size()));
  SWC_ASSERT(!std::memcmp(out.base + part1.size(), part2.data(), part2.size()));
  SWC_ASSERT(!std::memcmp(
    out.base + part1.size() + part2.size(), putback.data(), putback.size()));
  SWC_ASSERT(stream.empty());
  SWC_ASSERT(!stream.get(out));
}


void test_zstd_out_in_roundtrip() {
  const size_t pre_alloc = 256;
  const size_t commit_size = 128;
  auto plain = make_bytes(400);

  SWC::Core::BufferStreamOut_ZSTD out_stream(0, pre_alloc, commit_size);
  SWC_ASSERT(out_stream.error == SWC::Error::OK);
  out_stream.add(plain.data(), plain.size());
  SWC::StaticBuffer enc;
  out_stream.get(enc);
  SWC_ASSERT(out_stream.error == SWC::Error::OK);
  SWC_ASSERT(enc.size > 0);

  SWC::Core::BufferStreamIn_ZSTD in_stream;
  SWC_ASSERT(in_stream.error == SWC::Error::OK);
  in_stream.add(enc);

  SWC::StaticBuffer decoded;
  SWC_ASSERT(in_stream.get(decoded));
  SWC_ASSERT(decoded.size == plain.size());
  SWC_ASSERT(!std::memcmp(decoded.base, plain.data(), plain.size()));
  SWC_ASSERT(in_stream.empty());
}


void test_encoder_stream_out(SWC::Core::Encoder::Type encoder) {
  const size_t pre_alloc = 256;
  const size_t commit_size = 128;
  auto plain = make_bytes(512);

  SWC::Core::BufferStreamOut_ENCODER stream(encoder, pre_alloc, commit_size);
  SWC_ASSERT(stream.error == SWC::Error::OK);
  stream.add(plain.data(), plain.size());

  SWC::StaticBuffer enc;
  stream.get(enc);
  SWC_ASSERT(stream.error == SWC::Error::OK);
  SWC_ASSERT(enc.size > 0);
  SWC_ASSERT(stream.empty());

  std::vector<uint8_t> decoded(plain.size());
  int err = SWC::Error::OK;
  SWC::Core::Encoder::decode(
    err, encoder, enc.base, enc.size, decoded.data(), decoded.size());
  SWC_ASSERT(err == SWC::Error::OK);
  SWC_ASSERT(!std::memcmp(decoded.data(), plain.data(), plain.size()));
}


} // namespace


int main() {
  test_plain_stream();
  test_zstd_on_add_stream();
  test_stream_in_plain();
  test_zstd_out_in_roundtrip();
  test_encoder_stream_out(SWC::Core::Encoder::Type::ZSTD);
  test_encoder_stream_out(SWC::Core::Encoder::Type::SNAPPY);

  std::cout << "buffer_stream OK\n";
  return 0;
}
