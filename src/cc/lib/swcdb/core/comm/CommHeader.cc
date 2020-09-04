/* 
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/comm/CommHeader.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"

namespace SWC {


SWC_SHOULD_INLINE
CommHeader::CommHeader(uint64_t cmd, uint32_t timeout)
                      : version(1), header_len(0), flags(0),
                        id(0), timeout_ms(timeout), command(cmd), 
                        buffers(0), 
                        data_size(0), data_chksum(0), 
                        data_ext_size(0), data_ext_chksum(0), 
                        checksum(0) {
}
  
CommHeader::~CommHeader() { }

SWC_SHOULD_INLINE
void CommHeader::set(uint64_t cmd, uint32_t timeout) {
  command     = cmd;
  timeout_ms  = timeout;
}

SWC_SHOULD_INLINE
size_t CommHeader::encoded_length() { 
  header_len = FIXED_LENGTH;
  buffers = 0;
  if(data_size) {
    ++buffers;
    header_len += Serialization::encoded_length_vi32(data_size) + 4;
  }
  if(data_ext_size) {
    ++buffers;
    header_len += Serialization::encoded_length_vi32(data_ext_size) + 4;
  }
  return header_len;
}

SWC_SHOULD_INLINE
void CommHeader::encode(uint8_t** bufp) const {
  uint8_t *base = *bufp;
  Serialization::encode_i8(bufp,  version);
  Serialization::encode_i8(bufp,  header_len);
  Serialization::encode_i8(bufp,  flags);
  Serialization::encode_i32(bufp, id);
  Serialization::encode_i32(bufp, timeout_ms);
  Serialization::encode_i16(bufp, command);
  Serialization::encode_i8(bufp,  buffers);
    
  if(data_size) {
    Serialization::encode_vi32(bufp, data_size);
    Serialization::encode_i32(bufp, data_chksum);
  }
  if(data_ext_size) {
    Serialization::encode_vi32(bufp, data_ext_size);
    Serialization::encode_i32(bufp, data_ext_chksum); 
  }

  checksum_i32(base, header_len-4, bufp);
}

SWC_SHOULD_INLINE
void CommHeader::decode_prefix(const uint8_t** bufp, size_t* remainp) {
  if (*remainp < PREFIX_LENGTH)
    SWC_THROWF(Error::COMM_BAD_HEADER,
              "Header size %d is less than the fixed length %d",
              (int)*remainp, PREFIX_LENGTH);

  version = Serialization::decode_i8(bufp, remainp);
  header_len = Serialization::decode_i8(bufp, remainp);
}

SWC_SHOULD_INLINE
void CommHeader::decode(const uint8_t** bufp, size_t* remainp) {
  const uint8_t *base = *bufp;
  *bufp += PREFIX_LENGTH;
    
  flags = Serialization::decode_i8(bufp, remainp);
  id = Serialization::decode_i32(bufp, remainp);
  timeout_ms = Serialization::decode_i32(bufp, remainp);
  command = Serialization::decode_i16(bufp, remainp);
    
  if((buffers = Serialization::decode_i8(bufp, remainp))) {
    data_size = Serialization::decode_vi32(bufp, remainp);
    data_chksum = Serialization::decode_i32(bufp, remainp);    
    if(buffers > 1) {
      data_ext_size = Serialization::decode_vi32(bufp, remainp);
      data_ext_chksum = Serialization::decode_i32(bufp, remainp);
    }
  }

  checksum = Serialization::decode_i32(bufp, remainp);
  if(!checksum_i32_chk(checksum, base, header_len-4))
    SWC_THROWF(Error::COMM_HEADER_CHECKSUM_MISMATCH, 
              "header-checksum decoded-len=%lu", *bufp-base);
}

void CommHeader::initialize_from_request_header(const CommHeader &req_header) {
  flags = req_header.flags;
  id = req_header.id;
  command = req_header.command;
  buffers = 0;
  data_size = 0;
  data_chksum = 0;
  data_ext_size = 0;
  data_ext_chksum = 0;
}

void CommHeader::print(std::ostream& out) const {
  out << "version="     << (int)version
      << " header_len=" << (int)header_len
      << " flags="      << (int)flags
      << " id="         << (int)id
      << " timeout_ms=" << (int)timeout_ms
      << " command="    << (int)command
      << " buffers="    << (int)buffers
      << " data(sz=" << data_size     << " chk=" << data_chksum << ')'
      << " ext(sz="  << data_ext_size << " chk=" << data_ext_chksum << ')'
      << " checksum="   << (int)checksum ;
}
  

}