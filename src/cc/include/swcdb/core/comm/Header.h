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

  enum Flags : uint8_t {
    FLAGS_BIT_REQUEST          = 0x1, //!< Request message
    FLAGS_BIT_IGNORE_RESPONSE  = 0x2, //!< Response should be ignored
    FLAGS_BIT_URGENT           = 0x4, //!< Request is urgent
  };

  enum FlagMask : uint8_t {
    FLAGS_MASK_REQUEST          = 0xE, //!< Request message bit
    FLAGS_MASK_IGNORE_RESPONSE  = 0xD, //!< Response should be ignored bit
    FLAGS_MASK_URGENT           = 0xB, //!< Request is urgent bit
  };


  Header(uint64_t cmd=0, uint32_t timeout=0) noexcept;

  explicit Header(const Header& init_from_req_header) noexcept;

  // ~Header() { }

  void reset() noexcept;

  void set(uint64_t cmd=0, uint32_t timeout=0) noexcept;

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
