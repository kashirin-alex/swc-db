/**
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/comm/Header.h"

#include <cstring>
#include <vector>


namespace {


void fill_header(SWC::Comm::Header& h, bool with_ext) {
  h.set(42, 1500);
  h.flags = SWC::Comm::Header::FLAG_REQUEST_BIT;
  h.id = 0xabcdu;
  h.data.size = 128;
  h.data.size_plain = 0;
  h.data.chksum = 0x11111111u;
  h.data.encoder = SWC::Core::Encoder::Type::PLAIN;
  if(with_ext) {
    h.data_ext.size = 256;
    h.data_ext.size_plain = 512;
    h.data_ext.chksum = 0x22222222u;
    h.data_ext.encoder = SWC::Core::Encoder::Type::ZSTD;
  } else {
    h.data_ext.reset();
  }
}


void roundtrip(bool with_ext) {
  SWC::Comm::Header src;
  fill_header(src, with_ext);

  uint8_t header_len = src.encoded_length();
  SWC_ASSERT(header_len >= SWC::Comm::Header::FIXED_LENGTH);
  SWC_ASSERT(header_len <= SWC::Comm::Header::MAX_LENGTH);
  if(with_ext) {
    SWC_ASSERT(src.buffers == 2);
  } else {
    SWC_ASSERT(src.buffers == 1);
  }

  std::vector<uint8_t> buf(header_len);
  uint8_t* p = buf.data();
  src.encode(&p);
  SWC_ASSERT(size_t(p - buf.data()) == header_len);

  SWC::Comm::Header dst;
  {
    const uint8_t* rp = buf.data();
    size_t remain = SWC::Comm::Header::PREFIX_LENGTH;
    dst.decode_prefix(&rp, &remain);
    SWC_ASSERT(remain == 0);
    SWC_ASSERT(dst.version == src.version);
    SWC_ASSERT(dst.header_len == header_len);
  }
  {
    const uint8_t* rp = buf.data();
    size_t remain = header_len;
    dst.decode(&rp, &remain);
    SWC_ASSERT(size_t(rp - buf.data()) == header_len);
  }

  SWC_ASSERT(dst.flags == src.flags);
  SWC_ASSERT(dst.id == src.id);
  SWC_ASSERT(dst.timeout_ms == src.timeout_ms);
  SWC_ASSERT(dst.command == src.command);
  SWC_ASSERT(dst.buffers == src.buffers);
  SWC_ASSERT(dst.data.size == src.data.size);
  SWC_ASSERT(dst.data.encoder == src.data.encoder);
  SWC_ASSERT(dst.data.chksum == src.data.chksum);
  if(with_ext) {
    SWC_ASSERT(dst.data_ext.size == src.data_ext.size);
    SWC_ASSERT(dst.data_ext.size_plain == src.data_ext.size_plain);
    SWC_ASSERT(dst.data_ext.encoder == src.data_ext.encoder);
    SWC_ASSERT(dst.data_ext.chksum == src.data_ext.chksum);
  }
}


void checksum_mismatch() {
  SWC::Comm::Header src;
  fill_header(src, false);
  uint8_t header_len = src.encoded_length();
  std::vector<uint8_t> buf(header_len);
  uint8_t* p = buf.data();
  src.encode(&p);

  // Flip a byte inside the checksummed region (before trailing checksum).
  buf[4] ^= 0xff;

  SWC::Comm::Header dst;
  {
    const uint8_t* rp = buf.data();
    size_t remain = SWC::Comm::Header::PREFIX_LENGTH;
    dst.decode_prefix(&rp, &remain);
  }

  bool threw = false;
  try {
    const uint8_t* rp = buf.data();
    size_t remain = header_len;
    dst.decode(&rp, &remain);
  } catch(const SWC::Error::Exception& e) {
    threw = true;
    SWC_ASSERT(e.code() == SWC::Error::COMM_HEADER_CHECKSUM_MISMATCH);
  }
  SWC_ASSERT(threw);
}


void bad_prefix() {
  uint8_t tiny[1] = { 0x01 };
  SWC::Comm::Header h;
  bool threw = false;
  try {
    const uint8_t* rp = tiny;
    size_t remain = 1;
    h.decode_prefix(&rp, &remain);
  } catch(const SWC::Error::Exception& e) {
    threw = true;
    SWC_ASSERT(e.code() == SWC::Error::COMM_BAD_HEADER);
  }
  SWC_ASSERT(threw);
}


} // namespace


int main() {
  roundtrip(false);
  roundtrip(true);
  checksum_mismatch();
  bad_prefix();

  std::cout << "comm_header OK\n";
  return 0;
}
