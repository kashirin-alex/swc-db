/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/core/comm/CommBuf.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"

namespace SWC {

CommBuf::Ptr CommBuf::make(uint32_t reserve) {
  return std::make_shared<CommBuf>(reserve);
}

CommBuf::Ptr CommBuf::make(const Serializable& params, uint32_t reserve) {
  return std::make_shared<CommBuf>(params, reserve);
}

CommBuf::Ptr CommBuf::make(const Serializable& params, StaticBuffer& buffer, 
                  uint32_t reserve) {
  return std::make_shared<CommBuf>(params, buffer, reserve);
}

CommBuf::Ptr CommBuf::make(StaticBuffer& buffer, uint32_t reserve) {
  return std::make_shared<CommBuf>(buffer, reserve);
}

CommBuf::Ptr CommBuf::create_error_message(int error, const char *msg) {
  auto cbp = CommBuf::make(4 + Serialization::encoded_length_str16(msg));
  cbp->append_i32(error);
  cbp->append_str16(msg);
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

void CommBuf::write_header() {
  if(buf_data.size) {
    header.data_size   = buf_data.size;
    header.data_chksum = fletcher32(buf_data.base, buf_data.size);
  }
  if(buf_ext.size) {  
    header.data_ext_size   = buf_ext.size;
    header.data_ext_chksum = fletcher32(buf_ext.base, buf_ext.size);
  }
  buf_header.reallocate(header.encoded_length());
  uint8_t *buf = buf_header.base;
  header.encode(&buf);
}

void CommBuf::get(std::vector<asio::const_buffer>& buffers) {
  write_header();
  buffers.emplace_back(buf_header.base, buf_header.size);

  if(buf_data.size) 
    buffers.emplace_back(buf_data.base, buf_data.size);
  if(buf_ext.size) 
    buffers.emplace_back(buf_ext.base, buf_ext.size);
}

void* CommBuf::get_data_ptr() { 
  return data_ptr; 
}

uint8_t** CommBuf::get_data_ptr_address() { 
  return &data_ptr; 
}

void* CommBuf::advance_data_ptr(size_t len) { 
  data_ptr += len; 
  return data_ptr; 
}

void CommBuf::append_bool(bool bval) { 
  Serialization::encode_bool(&data_ptr, bval); 
}

void CommBuf::append_byte(uint8_t bval) { 
  *data_ptr++ = bval; 
}

void CommBuf::append_bytes(const uint8_t *bytes, uint32_t len) {
  memcpy(data_ptr, bytes, len);
  data_ptr += len;
}

void CommBuf::append_str16(const char *str) {
  Serialization::encode_str16(&data_ptr, str);
}

void CommBuf::append_str16(const std::string &str) {
  Serialization::encode_str16(&data_ptr, str);
}

void CommBuf::append_i16(uint16_t sval) {
  Serialization::encode_i16(&data_ptr, sval);
}

void CommBuf::append_i32(uint32_t ival) {
  Serialization::encode_i32(&data_ptr, ival);
}

void CommBuf::append_i64(uint64_t lval) {
  Serialization::encode_i64(&data_ptr, lval);
}

void CommBuf::append_vstr(const char *str) {
  Serialization::encode_vstr(&data_ptr, str);
}

void CommBuf::append_vstr(const std::string &str) {
  Serialization::encode_vstr(&data_ptr, str);
}

void CommBuf::append_vstr(const void *str, uint32_t len) {
  Serialization::encode_vstr(&data_ptr, str, len);
}


} // namespace SWC
