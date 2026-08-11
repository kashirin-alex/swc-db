/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Checksum.h"
#include "swcdb/core/comm/HeaderBufferInfo.h"

#include <cstring>
#include <string>
#include <vector>


namespace {


std::vector<uint8_t> make_bytes(size_t len, bool compressible) {
  std::vector<uint8_t> data(len);
  if(compressible) {
    for(size_t i = 0; i < len; ++i)
      data[i] = uint8_t('A' + (i % 7));
  } else {
    for(size_t i = 0; i < len; ++i)
      data[i] = uint8_t((i * 37 + 11) & 0xff);
  }
  return data;
}


void test_field_serde() {
  SWC::Comm::BufferInfo src;
  src.size = 100;
  src.size_plain = 200;
  src.chksum = 0xabcdef01u;
  src.encoder = SWC::Core::Encoder::Type::ZSTD;

  uint8_t len = src.encoded_length();
  std::vector<uint8_t> buf(len);
  uint8_t* p = buf.data();
  src.encode(&p);
  SWC_ASSERT(size_t(p - buf.data()) == len);

  SWC::Comm::BufferInfo dst;
  const uint8_t* rp = buf.data();
  size_t remain = len;
  dst.decode(&rp, &remain);
  SWC_ASSERT(remain == 0);
  SWC_ASSERT(dst.size == src.size);
  SWC_ASSERT(dst.size_plain == src.size_plain);
  SWC_ASSERT(dst.chksum == src.chksum);
  SWC_ASSERT(dst.encoder == src.encoder);
}


void test_plain_small_stays_plain() {
  auto bytes = make_bytes(16, true);
  SWC::StaticBuffer data(bytes.size());
  std::memcpy(data.base, bytes.data(), bytes.size());

  SWC::Comm::BufferInfo info;
  info.encode(SWC::Core::Encoder::Type::ZSTD, data);

  SWC_ASSERT(info.encoder == SWC::Core::Encoder::Type::PLAIN);
  SWC_ASSERT(info.size_plain == 0);
  SWC_ASSERT(info.size == bytes.size());
  SWC_ASSERT(info.chksum == SWC::Core::checksum32(data.base, data.size));
  SWC_ASSERT(!std::memcmp(data.base, bytes.data(), bytes.size()));
}


void test_compress_roundtrip(SWC::Core::Encoder::Type encoder) {
  auto bytes = make_bytes(256, true);
  SWC::StaticBuffer data(bytes.size());
  std::memcpy(data.base, bytes.data(), bytes.size());

  SWC::Comm::BufferInfo info;
  info.encode(encoder, data);

  SWC_ASSERT(info.encoder == encoder);
  SWC_ASSERT(info.size_plain == bytes.size());
  SWC_ASSERT(info.size == data.size);
  SWC_ASSERT(data.size < bytes.size());
  SWC_ASSERT(info.chksum == SWC::Core::checksum32(data.base, data.size));
  SWC_ASSERT(SWC::Core::checksum_i32_chk(info.chksum, data.base, data.size));

  int err = SWC::Error::OK;
  info.decode(err, data);
  SWC_ASSERT(err == SWC::Error::OK);
  SWC_ASSERT(data.size == bytes.size());
  SWC_ASSERT(!std::memcmp(data.base, bytes.data(), bytes.size()));
}


void test_corrupt_decode() {
  auto bytes = make_bytes(256, true);
  SWC::StaticBuffer data(bytes.size());
  std::memcpy(data.base, bytes.data(), bytes.size());

  SWC::Comm::BufferInfo info;
  info.encode(SWC::Core::Encoder::Type::ZSTD, data);
  SWC_ASSERT(info.size_plain == bytes.size());

  if(data.size)
    data.base[0] ^= 0xff;

  int err = SWC::Error::OK;
  info.decode(err, data);
  SWC_ASSERT(err == SWC::Error::ENCODER_DECODE);
}


} // namespace


int main() {
  test_field_serde();
  test_plain_small_stays_plain();
  test_compress_roundtrip(SWC::Core::Encoder::Type::ZSTD);
  test_compress_roundtrip(SWC::Core::Encoder::Type::SNAPPY);
  test_corrupt_decode();

  std::cout << "comm_buffer_info OK\n";
  return 0;
}
