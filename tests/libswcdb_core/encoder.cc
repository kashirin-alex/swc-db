/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Encoder.h"

#include <cstring>
#include <string>
#include <vector>


namespace {


std::vector<uint8_t> make_payload(size_t len, bool compressible) {
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


void check_names() {
  using SWC::Core::Encoder::Type;
  using SWC::Core::Encoder::encoding_from;
  using SWC::Core::Encoder::to_string;

  SWC_ASSERT(encoding_from("DEFAULT") == Type::DEFAULT);
  SWC_ASSERT(encoding_from("PLAIN") == Type::PLAIN);
  SWC_ASSERT(encoding_from("ZLIB") == Type::ZLIB);
  SWC_ASSERT(encoding_from("SNAPPY") == Type::SNAPPY);
  SWC_ASSERT(encoding_from("ZSTD") == Type::ZSTD);
  SWC_ASSERT(encoding_from("zlib") == Type::ZLIB);
  SWC_ASSERT(encoding_from("0") == Type::DEFAULT);
  SWC_ASSERT(encoding_from("1") == Type::PLAIN);
  SWC_ASSERT(encoding_from("2") == Type::ZLIB);
  SWC_ASSERT(encoding_from("3") == Type::SNAPPY);
  SWC_ASSERT(encoding_from("4") == Type::ZSTD);
  SWC_ASSERT(encoding_from("nope") == Type::UNKNOWN);
  SWC_ASSERT(encoding_from("") == Type::UNKNOWN);

  SWC_ASSERT(std::string(to_string(Type::DEFAULT)) == "DEFAULT");
  SWC_ASSERT(std::string(to_string(Type::PLAIN)) == "PLAIN");
  SWC_ASSERT(std::string(to_string(Type::ZLIB)) == "ZLIB");
  SWC_ASSERT(std::string(to_string(Type::SNAPPY)) == "SNAPPY");
  SWC_ASSERT(std::string(to_string(Type::ZSTD)) == "ZSTD");
  SWC_ASSERT(std::string(to_string(Type::UNKNOWN)) == "UNKNOWN");
}


void roundtrip_plain(const std::vector<uint8_t>& src) {
  int err = SWC::Error::OK;
  size_t sz_enc = 0;
  SWC::DynamicBuffer output;
  SWC::Core::Encoder::encode(
    err, SWC::Core::Encoder::Type::PLAIN,
    src.data(), src.size(), &sz_enc, output, 0);
  SWC_ASSERT(err == SWC::Error::OK);
  SWC_ASSERT(sz_enc == 0);
  SWC_ASSERT(output.fill() == src.size());
  if(src.size())
    SWC_ASSERT(!std::memcmp(output.base, src.data(), src.size()));
}


void roundtrip_codec(SWC::Core::Encoder::Type encoder,
                     const std::vector<uint8_t>& src) {
  int err = SWC::Error::OK;
  size_t sz_enc = 0;
  SWC::DynamicBuffer output;
  SWC::Core::Encoder::encode(
    err, encoder, src.data(), src.size(), &sz_enc, output, 0, false, false);
  SWC_ASSERT(err == SWC::Error::OK);
  SWC_ASSERT(sz_enc > 0);
  SWC_ASSERT(sz_enc == output.fill());
  SWC_ASSERT(sz_enc < src.size());

  std::vector<uint8_t> dst(src.size());
  SWC::Core::Encoder::decode(
    err, encoder, output.base, sz_enc, dst.data(), dst.size());
  SWC_ASSERT(err == SWC::Error::OK);
  SWC_ASSERT(!std::memcmp(dst.data(), src.data(), src.size()));
}


void corrupt_decode(SWC::Core::Encoder::Type encoder) {
  const uint8_t junk[] = { 0x00, 0xff, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
  uint8_t dst[64];
  int err = SWC::Error::OK;
  SWC::Core::Encoder::decode(
    err, encoder, junk, sizeof(junk), dst, sizeof(dst));
  SWC_ASSERT(err == SWC::Error::ENCODER_DECODE);
}


} // namespace


int main() {
  check_names();

  roundtrip_plain(make_payload(0, true));
  roundtrip_plain(make_payload(16, false));
  roundtrip_plain(make_payload(4096, true));

  auto compressible = make_payload(64 * 1024, true);
  roundtrip_codec(SWC::Core::Encoder::Type::ZLIB, compressible);
  roundtrip_codec(SWC::Core::Encoder::Type::SNAPPY, compressible);
  roundtrip_codec(SWC::Core::Encoder::Type::ZSTD, compressible);

  corrupt_decode(SWC::Core::Encoder::Type::ZLIB);
  corrupt_decode(SWC::Core::Encoder::Type::SNAPPY);
  corrupt_decode(SWC::Core::Encoder::Type::ZSTD);

  std::cout << "encoder OK\n";
  return 0;
}
