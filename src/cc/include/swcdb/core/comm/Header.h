/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_Header_h
#define swcdb_core_comm_Header_h


#include "swcdb/core/comm/HeaderBufferInfo.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"


namespace SWC { namespace Comm {


struct Header final {

  static const uint8_t PROTOCOL_VERSION = 1;
  static const uint8_t PREFIX_LENGTH    = 2;
  static const uint8_t FIXED_LENGTH     = PREFIX_LENGTH + 16;
  static const uint8_t MAX_LENGTH       = FIXED_LENGTH + 15 * 2;

  static const uint8_t FLAG_REQUEST_BIT           = 0x01;
  static const uint8_t FLAG_REQUEST_MASK          = 0xFE;

  static const uint8_t FLAG_RESPONSE_IGNORE_BIT   = 0x02;
  static const uint8_t FLAG_RESPONSE_PARTIAL_BIT  = 0x04;


  Header(uint64_t cmd=0, uint32_t timeout=0) noexcept
        : version(1), header_len(0), flags(0), buffers(0),
          id(0), timeout_ms(timeout), checksum(0), command(cmd) {
  }

  explicit Header(const Header& init_from_req_header) noexcept
                  : version(1), header_len(0),
                    flags(init_from_req_header.flags), buffers(0),
                    id(init_from_req_header.id),
                    timeout_ms(0), checksum(0),
                    command(init_from_req_header.command) {
  }

  //~Header() { }

  void reset() noexcept;

  void set(uint64_t cmd=0, uint32_t timeout=0) noexcept {
    command     = cmd;
    timeout_ms  = timeout;
  }

  uint8_t encoded_length() noexcept;

  void encode(uint8_t** bufp) const;

  void decode_prefix(const uint8_t** bufp, size_t* remainp);

  void decode(const uint8_t** bufp, size_t* remainp);

  void initialize_from_response(const Header& header);

  void initialize_from_request(const Header& header);

  void print(std::ostream& out) const;

  uint8_t     version;      //!< Protocol version
  uint8_t     header_len;   //!< Length of header

  uint8_t     flags;        //!< Flags
  uint8_t     buffers;      //!< number of buffers from 0 to 2 (data+data_ext)
  uint32_t    id;           //!< Request ID
  uint32_t    timeout_ms;   //!< Request timeout
  uint32_t    checksum;     //!< Header checksum (excl. it self)
  uint16_t    command;      //!< Request command number

  BufferInfo  data;         //!< Data Buffer
  BufferInfo  data_ext;     //!< Data Extended Buffer

} __attribute__((packed));



SWC_CAN_INLINE
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

SWC_CAN_INLINE
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

SWC_CAN_INLINE
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

SWC_CAN_INLINE
void Header::decode_prefix(const uint8_t** bufp, size_t* remainp) {
  if (*remainp < PREFIX_LENGTH)
    SWC_THROWF(Error::COMM_BAD_HEADER,
              "Header size %lu is less than the fixed length %d",
              *remainp, PREFIX_LENGTH);

  version = Serialization::decode_i8(bufp, remainp);
  header_len = Serialization::decode_i8(bufp, remainp);
}

SWC_CAN_INLINE
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

SWC_CAN_INLINE
void Header::initialize_from_response(const Header& header) {
  flags = header.flags;
  id = header.id;
  command = header.command;
  buffers = 0;
  data.reset();
  data_ext.reset();
}

SWC_CAN_INLINE
void Header::initialize_from_request(const Header& header) {
  flags = header.flags;
  id = header.id;
  command = header.command;
  buffers = 0;
  data.reset();
  data_ext.reset();
}



}} //namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Header.cc"
#endif

#endif // swcdb_core_comm_Header_h
