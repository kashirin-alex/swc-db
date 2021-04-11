/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"
#include "swcdb/core/comm/Header.h"


namespace SWC { namespace Comm {


SWC_SHOULD_INLINE
Header::Header(uint64_t cmd, uint32_t timeout) noexcept
              : version(1), header_len(0), flags(0), buffers(0),
                id(0), timeout_ms(timeout), checksum(0), command(cmd) {
}
SWC_SHOULD_INLINE
Header::Header(const Header& init_from_req_header) noexcept
              : version(1), header_len(0),
                flags(init_from_req_header.flags), buffers(0),
                id(init_from_req_header.id),
                timeout_ms(0), checksum(0),
                command(init_from_req_header.command) {
}

SWC_SHOULD_INLINE
void Header::reset() noexcept {
  version = 0;
  header_len = 0;
  flags = 0;
  id = 0;
  timeout_ms = 0;
  command = 0;
  buffers = 0;
  data.reset();
  data_ext.reset();
  checksum = 0;
}

SWC_SHOULD_INLINE
void Header::set(uint64_t cmd, uint32_t timeout) noexcept {
  command     = cmd;
  timeout_ms  = timeout;
}

SWC_SHOULD_INLINE
uint8_t Header::encoded_length() noexcept {
  header_len = FIXED_LENGTH;
  buffers = 0;
  if(data.size) {
    ++buffers;
    header_len += data.encoded_length();
    if(data_ext.size) {
      ++buffers;
      header_len += data_ext.encoded_length();
    }
  }
  return header_len;
}

SWC_SHOULD_INLINE
void Header::encode(uint8_t** bufp) const {
  uint8_t *base = *bufp;
  Serialization::encode_i8(bufp,  version);
  Serialization::encode_i8(bufp,  header_len);
  Serialization::encode_i8(bufp,  flags);
  Serialization::encode_i32(bufp, id);
  Serialization::encode_i32(bufp, timeout_ms);
  Serialization::encode_i16(bufp, command);
  Serialization::encode_i8(bufp,  buffers);

  if(data.size) {
    data.encode(bufp);
    if(data_ext.size)
      data_ext.encode(bufp);
  }

  Core::checksum_i32(base, header_len-4, bufp);
}

SWC_SHOULD_INLINE
void Header::decode_prefix(const uint8_t** bufp, size_t* remainp) {
  if (*remainp < PREFIX_LENGTH)
    SWC_THROWF(Error::COMM_BAD_HEADER,
              "Header size %lu is less than the fixed length %d",
              *remainp, PREFIX_LENGTH);

  version = Serialization::decode_i8(bufp, remainp);
  header_len = Serialization::decode_i8(bufp, remainp);
}

SWC_SHOULD_INLINE
void Header::decode(const uint8_t** bufp, size_t* remainp) {
  const uint8_t *base = *bufp;
  *bufp += PREFIX_LENGTH;

  flags = Serialization::decode_i8(bufp, remainp);
  id = Serialization::decode_i32(bufp, remainp);
  timeout_ms = Serialization::decode_i32(bufp, remainp);
  command = Serialization::decode_i16(bufp, remainp);

  if((buffers = Serialization::decode_i8(bufp, remainp))) {
    data.decode(bufp, remainp);
    if(buffers > 1)
      data_ext.decode(bufp, remainp);
  }

  checksum = Serialization::decode_i32(bufp, remainp);
  if(!Core::checksum_i32_chk(checksum, base, header_len-4))
    SWC_THROWF(Error::COMM_HEADER_CHECKSUM_MISMATCH,
              "header-checksum decoded-len=%ld", *bufp-base);
}

void Header::initialize_from_response(const Header& header) {
  flags = header.flags;
  id = header.id;
  command = header.command;
  buffers = 0;
  data.reset();
  data_ext.reset();
}

void Header::initialize_from_request(const Header& header) {
  flags = header.flags;
  id = header.id;
  command = header.command;
  buffers = 0;
  data.reset();
  data_ext.reset();
}

void Header::print(std::ostream& out) const {
  out << "version="     << int(version)
      << " header_len=" << int(header_len)
      << " flags="      << int(flags)
      << " id="         << int(id)
      << " timeout_ms=" << int(timeout_ms)
      << " command="    << int(command)
      << " buffers="    << int(buffers);
  if(buffers) {
    data.print(out);
    data_ext.print(out);
  }
  out << " checksum="   << int(checksum);
}


}}
