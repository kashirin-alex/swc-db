/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Buffers.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"

namespace SWC { namespace Comm {

namespace {
static const uint16_t BUFFER_CHUNK_SZ = 4096;
}

SWC_SHOULD_INLINE
Buffers::Ptr Buffers::make(uint32_t reserve) {
  return std::make_shared<Buffers>(reserve);
}

SWC_SHOULD_INLINE
Buffers::Ptr Buffers::make(const Serializable& params, uint32_t reserve) {
  return std::make_shared<Buffers>(params, reserve);
}

SWC_SHOULD_INLINE
Buffers::Ptr Buffers::make(const Serializable& params, StaticBuffer& buffer, 
                           uint32_t reserve) {
  return std::make_shared<Buffers>(params, buffer, reserve);
}

SWC_SHOULD_INLINE
Buffers::Ptr Buffers::make(StaticBuffer& buffer, uint32_t reserve) {
  return std::make_shared<Buffers>(buffer, reserve);
}

SWC_SHOULD_INLINE
Buffers::Ptr 
Buffers::create_error_message(int error, const char *msg, uint16_t len) {
  auto cbp = Buffers::make(4 + Serialization::encoded_length_bytes(len));
  Serialization::encode_i32(&cbp->data_ptr, error);
  Serialization::encode_bytes(&cbp->data_ptr, msg, len);
  return cbp;
}

Buffers::Buffers(uint32_t reserve) {
  if(reserve)
    set_data(reserve);
}

Buffers::Buffers(const Serializable& params, uint32_t reserve) {
  set_data(params, reserve);
}

Buffers::Buffers(const Serializable& params, StaticBuffer& buffer, 
                 uint32_t reserve) : buf_ext(buffer) {
  set_data(params, reserve);
}

Buffers::Buffers(StaticBuffer& buffer, uint32_t reserve) 
                : buf_ext(buffer) {
  if(reserve)
    set_data(reserve);
}

Buffers::~Buffers() { }

void Buffers::set_data(uint32_t sz) {
  if(buf_data.size)
    header.data.reset();

  buf_data.reallocate(sz);
  data_ptr = buf_data.base; 
}

void Buffers::set_data(const Serializable& params, uint32_t reserve) {
  set_data(reserve + params.encoded_length());

  data_ptr = buf_data.base + reserve;
  params.encode(&data_ptr);
  data_ptr = buf_data.base;
}

void Buffers::prepare(Core::Encoder::Type encoder) {
  if(buf_data.size) {
    header.data.encode(encoder, buf_data);
    if(buf_ext.size)
      header.data_ext.encode(encoder, buf_ext);
  }
}

SWC_SHOULD_INLINE
void Buffers::write_header() {
  buf_header.reallocate(header.encoded_length());
  uint8_t *buf = buf_header.base;
  header.encode(&buf);
}

std::vector<asio::const_buffer> Buffers::get_buffers() {
  write_header();

  size_t sz = 1;

  size_t buf_data_chunks;
  bool buf_data_not_aligned;
  if(buf_data.size) {
    double chunks = double(buf_data.size)/BUFFER_CHUNK_SZ;
    buf_data_chunks = chunks;
    buf_data_not_aligned = double(buf_data_chunks) != chunks;
    sz += buf_data_chunks;
    sz += buf_data_not_aligned;
  } else {
    buf_data_chunks = 0;
    buf_data_not_aligned = false;
  }

  size_t buf_ext_chunks;
  bool buf_ext_not_aligned;
  if(buf_ext.size) {
    double chunks = double(buf_ext.size)/BUFFER_CHUNK_SZ;
    buf_ext_chunks = chunks;
    buf_ext_not_aligned = double(buf_ext_chunks) != chunks;
    sz += buf_ext_chunks;
    sz += buf_ext_not_aligned;
  } else {
    buf_ext_chunks = 0;
    buf_ext_not_aligned = false;
  }

  std::vector<asio::const_buffer> buffers(sz);
  auto it = buffers.begin();
  *it = asio::const_buffer(buf_header.base, buf_header.size);

  if(buf_data.size) {
    auto p = buf_data.base;
    for(size_t i = 0; i<buf_data_chunks; ++i, p+=BUFFER_CHUNK_SZ)
      *++it = asio::const_buffer(p, BUFFER_CHUNK_SZ);
    if(buf_data_not_aligned)
      *++it = asio::const_buffer(p, (buf_data.base + buf_data.size) - p);
  }
  if(buf_ext.size) {
    auto p = buf_ext.base;
    for(size_t i = 0; i<buf_ext_chunks; ++i, p+=BUFFER_CHUNK_SZ)
      *++it = asio::const_buffer(p, BUFFER_CHUNK_SZ);
    if(buf_ext_not_aligned)
      *++it = asio::const_buffer(p, (buf_ext.base + buf_ext.size) - p);
  }
  return buffers;
}

SWC_SHOULD_INLINE
void Buffers::append_i8(uint8_t ival) {
  Serialization::encode_i8(&data_ptr, ival);
}

SWC_SHOULD_INLINE
void Buffers::append_i32(uint32_t ival) {
  Serialization::encode_i32(&data_ptr, ival);
}


}} // namespace SWC::Comm
