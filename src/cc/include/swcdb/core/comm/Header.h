/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_Header_h
#define swcdb_core_comm_Header_h


#include "swcdb/core/comm/HeaderBufferInfo.h"


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


}} //namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Header.cc"
#endif

#endif // swcdb_core_comm_Header_h
