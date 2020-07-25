/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/CommBuf.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"

namespace SWC {

namespace {
static const uint16_t BUFFER_CHUNK_SZ = 4096;
}

SWC_SHOULD_INLINE
CommBuf::Ptr CommBuf::make(uint32_t reserve) {
  return std::make_shared<CommBuf>(reserve);
}

SWC_SHOULD_INLINE
CommBuf::Ptr CommBuf::make(const Serializable& params, uint32_t reserve) {
  return std::make_shared<CommBuf>(params, reserve);
}

SWC_SHOULD_INLINE
CommBuf::Ptr CommBuf::make(const Serializable& params, StaticBuffer& buffer, 
                           uint32_t reserve) {
  return std::make_shared<CommBuf>(params, buffer, reserve);
}

SWC_SHOULD_INLINE
CommBuf::Ptr CommBuf::make(StaticBuffer& buffer, uint32_t reserve) {
  return std::make_shared<CommBuf>(buffer, reserve);
}

SWC_SHOULD_INLINE
CommBuf::Ptr 
CommBuf::create_error_message(int error, const char *msg, uint16_t len) {
  auto cbp = CommBuf::make(4 + Serialization::encoded_length_bytes(len));
  Serialization::encode_i32(&cbp->data_ptr, error);
  Serialization::encode_bytes(&cbp->data_ptr, msg, len);
  return cbp;
}

CommBuf::CommBuf(uint32_t reserve) {
  if(reserve)
    set_data(reserve);
}

CommBuf::CommBuf(const Serializable& params, uint32_t reserve) {
  set_data(params, reserve);
}

CommBuf::CommBuf(const Serializable& params, StaticBuffer& buffer, 
                uint32_t reserve) : buf_ext(buffer) {
  set_data(params, reserve);
}

CommBuf::CommBuf(StaticBuffer& buffer, uint32_t reserve) 
                : buf_ext(buffer) {
  if(reserve)
    set_data(reserve);
}

CommBuf::~CommBuf() { }

void CommBuf::set_data(uint32_t sz) {
  buf_data.reallocate(sz);
  data_ptr = buf_data.base; 
}

void CommBuf::set_data(const Serializable& params, uint32_t reserve) {
  set_data(reserve + params.encoded_length());

  data_ptr = buf_data.base + reserve;
  params.encode(&data_ptr);
  data_ptr = buf_data.base;
}

SWC_SHOULD_INLINE
void CommBuf::write_header() {
  if(buf_data.size) {
    header.data_size   = buf_data.size;
    header.data_chksum = checksum32(buf_data.base, buf_data.size);
  }
  if(buf_ext.size) {  
    header.data_ext_size   = buf_ext.size;
    header.data_ext_chksum = checksum32(buf_ext.base, buf_ext.size);
  }
  buf_header.reallocate(header.encoded_length());
  uint8_t *buf = buf_header.base;
  header.encode(&buf);
}

std::vector<asio::const_buffer> CommBuf::get_buffers() {
  write_header();

  std::vector<asio::const_buffer> buffers;
  buffers.reserve(
    3 + 
    buf_data.size/BUFFER_CHUNK_SZ + 
    buf_ext.size/BUFFER_CHUNK_SZ
  );
  buffers.emplace_back(buf_header.base, buf_header.size);

  if(buf_data.size) for(size_t i=0; ; i += BUFFER_CHUNK_SZ) {
    if(buf_data.size > i + BUFFER_CHUNK_SZ) {
      buffers.emplace_back(buf_data.base + i, BUFFER_CHUNK_SZ);
    } else {
      buffers.emplace_back(buf_data.base + i, buf_data.size - i);
      break;
    }
  }
  if(buf_ext.size) for(size_t i=0; ; i += BUFFER_CHUNK_SZ) {
    if(buf_ext.size > i + BUFFER_CHUNK_SZ) {
      buffers.emplace_back(buf_ext.base + i, BUFFER_CHUNK_SZ);
    } else {
      buffers.emplace_back(buf_ext.base + i, buf_ext.size - i);
      break;
    }
  }
  return buffers;
}

SWC_SHOULD_INLINE
void CommBuf::append_i32(uint32_t ival) {
  Serialization::encode_i32(&data_ptr, ival);
}


} // namespace SWC
