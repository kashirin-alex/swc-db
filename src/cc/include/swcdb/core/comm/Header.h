/* 
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_Header_h
#define swcdb_core_comm_Header_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Encoder.h"

namespace SWC { namespace Comm {

class Header final {

  public:

  static const uint8_t PROTOCOL_VERSION = 1;
  static const uint8_t PREFIX_LENGTH    = 2;
  static const uint8_t FIXED_LENGTH     = PREFIX_LENGTH + 16;


  struct Buffer final {

    Buffer();
    
    void reset();

    size_t encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void encode(Core::Encoder::Type encoder, StaticBuffer& data);
    
    void decode(int& err, StaticBuffer& data) const;

    void print(std::ostream& out) const;
  
    uint32_t            size;        //!< Buffer size
    Core::Encoder::Type encoder;     //!< Buffer Encoder
    uint32_t            size_plain;  //!< Buffer set if Encoder not PLAIN
    uint32_t            chksum;      //!< Buffer checksum
  
  } __attribute__((packed));


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


  Header(uint64_t cmd=0, uint32_t timeout=0);

  ~Header();

  void set(uint64_t cmd=0, uint32_t timeout=0);

  size_t encoded_length();

  void encode(uint8_t** bufp) const;

  void decode_prefix(const uint8_t** bufp, size_t* remainp);

  void decode(const uint8_t** bufp, size_t* remainp);

  void initialize_from_request_header(const Header &req_header);

  void print(std::ostream& out) const;

  uint8_t  version;         //!< Protocol version
  uint8_t  header_len;      //!< Length of header

  uint8_t  flags;           //!< Flags
  uint32_t id;              //!< Request ID
  uint32_t timeout_ms;      //!< Request timeout
  uint16_t command;         //!< Request command number
  uint8_t  buffers;         //!< number of buffers from 0 to 2 (data+data_ext) 

  Buffer  data;             //!< Data Buffer
  Buffer  data_ext;         //!< Data Extended Buffer

  uint32_t checksum;        //!< Header checksum (excl. it self)

} __attribute__((packed));


}} //namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Header.cc"
#endif 

#endif // swcdb_core_comm_Header_h
