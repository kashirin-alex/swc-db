/* 
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/Header.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"

namespace SWC { namespace Comm {


Header::Buffer::Buffer()
  : size(0), 
    encoder(Core::Encoder::Type::PLAIN), 
    size_plain(0), 
    chksum(0) {
}

SWC_SHOULD_INLINE
void Header::Buffer::reset() {
  size = 0;
  encoder = Core::Encoder::Type::PLAIN;
  size_plain = 0;
  chksum = 0;
}

size_t Header::Buffer::encoded_length() const {
  size_t sz = Serialization::encoded_length_vi32(size) + 5;
  if(encoder != Core::Encoder::Type::PLAIN)
    sz += Serialization::encoded_length_vi32(size_plain);
  return sz;
}

void Header::Buffer::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, size);
  Serialization::encode_i8(bufp, (uint8_t)encoder);
  if(encoder != Core::Encoder::Type::PLAIN)
    Serialization::encode_vi32(bufp, size_plain); 
  Serialization::encode_i32(bufp, chksum);
}

void Header::Buffer::decode(const uint8_t** bufp, size_t* remainp) {
  size = Serialization::decode_vi32(bufp, remainp);
  encoder = (Core::Encoder::Type)Serialization::decode_i8(bufp, remainp);
  if(encoder != Core::Encoder::Type::PLAIN)
    size_plain = Serialization::decode_vi32(bufp, remainp);
  chksum = Serialization::decode_i32(bufp, remainp);
}

void Header::Buffer::encode(Core::Encoder::Type _enc, StaticBuffer& data) {
  if(_enc != Core::Encoder::Type::PLAIN && 
     encoder == Core::Encoder::Type::PLAIN &&
     data.size > 32) { // at least size if encoder not encrypt-type
    
    int err = Error::OK;
    size_t len_enc = 0;
    DynamicBuffer output;
    Core::Encoder::encode(err, _enc, data.base, data.size, 
                          &len_enc, output, 0);
    if(len_enc) {
      encoder = _enc;
      size_plain = data.size;
      data.set(output);
    }
  }

  size   = data.size;
  chksum = Core::checksum32(data.base, data.size);
}

void Header::Buffer::decode(int& err, StaticBuffer& data) const {
  if(size_plain) {
    StaticBuffer decoded_buf((size_t)size_plain);
    Core::Encoder::decode(
      err, encoder, 
      data.base, data.size, 
      decoded_buf.base, size_plain
    );
    if(!err)
      data.set(decoded_buf);
  }
}

void Header::Buffer::print(std::ostream& out) const {
  out << " Buffer(sz=" << size
      << " enc=" << Core::Encoder::to_string(encoder)
      << " szplain=" << size_plain
      << " chk=" << chksum << ')';
}



SWC_SHOULD_INLINE
Header::Header(uint64_t cmd, uint32_t timeout)
              : version(1), header_len(0), flags(0),
                id(0), timeout_ms(timeout), command(cmd),
                buffers(0), checksum(0) {
}
  
Header::~Header() { }

SWC_SHOULD_INLINE
void Header::set(uint64_t cmd, uint32_t timeout) {
  command     = cmd;
  timeout_ms  = timeout;
}

SWC_SHOULD_INLINE
size_t Header::encoded_length() { 
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
              "Header size %d is less than the fixed length %d",
              (int)*remainp, PREFIX_LENGTH);

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
              "header-checksum decoded-len=%lu", *bufp-base);
}

void Header::initialize_from_request_header(const Header &req_header) {
  flags = req_header.flags;
  id = req_header.id;
  command = req_header.command;
  buffers = 0;
  data.reset();
  data_ext.reset();
}

void Header::print(std::ostream& out) const {
  out << "version="     << (int)version
      << " header_len=" << (int)header_len
      << " flags="      << (int)flags
      << " id="         << (int)id
      << " timeout_ms=" << (int)timeout_ms
      << " command="    << (int)command
      << " buffers="    << (int)buffers;
  if(buffers) {
    data.print(out);
    data_ext.print(out);
  }
  out << " checksum="   << (int)checksum;
}


}}
