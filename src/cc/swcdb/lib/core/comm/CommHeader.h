/* 
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_comm_CommHeader_h
#define swc_core_comm_CommHeader_h

#include "swcdb/lib/core/Compat.h"
#include "swcdb/lib/core/Checksum.h"

namespace SWC {

class CommHeader {

  public:

  static const uint8_t PROTOCOL_VERSION = 1;
 
  static const uint8_t FIXED_LENGTH = 25;

  enum Flags {
    FLAGS_BIT_REQUEST          = 0x1, //!< Request message
    FLAGS_BIT_IGNORE_RESPONSE  = 0x2, //!< Response should be ignored
    FLAGS_BIT_URGENT           = 0x4, //!< Request is urgent
  };
  
  enum FlagMask {
    FLAGS_MASK_REQUEST          = 0xE, //!< Request message bit
    FLAGS_MASK_IGNORE_RESPONSE  = 0xD, //!< Response should be ignored bit
    FLAGS_MASK_URGENT           = 0xB, //!< Request is urgent bit
  };

  CommHeader(): version(1), header_len(FIXED_LENGTH), flags(0),
                gid(0), id(0), timeout_ms(0), command(0), 
                total_len(0), header_checksum(0) {  }

  CommHeader(uint64_t cmd, uint32_t timeout=0)
            : version(1), header_len(FIXED_LENGTH), flags(0),
              gid(0), id(0), timeout_ms(timeout), command(cmd), 
              total_len(0), header_checksum(0) {  }
  
  size_t encoded_length() const { return FIXED_LENGTH; }


  void encode(uint8_t **bufp){
    uint8_t *base = *bufp;
    Serialization::encode_i8(bufp, version);
    Serialization::encode_i8(bufp, header_len);
    Serialization::encode_i8(bufp, flags);
    Serialization::encode_i32(bufp, gid);
    Serialization::encode_i32(bufp, id);
    Serialization::encode_i32(bufp, timeout_ms);
    Serialization::encode_i16(bufp, command);
  
    Serialization::encode_i32(bufp, total_len);
    checksum_i32(base, *bufp, bufp);
  }

  void decode(const uint8_t **bufp, size_t *remainp){
    const uint8_t *base = *bufp;
    if (*remainp < FIXED_LENGTH)
      HT_THROWF(Error::COMM_BAD_HEADER,
                "Header size %d is less than the minumum fixed length %d",
                (int)*remainp, (int)FIXED_LENGTH);
    
    HT_TRY("decoding comm header",
      version = Serialization::decode_i8(bufp, remainp);
      header_len = Serialization::decode_i8(bufp, remainp);
      flags = Serialization::decode_i8(bufp, remainp);
      gid = Serialization::decode_i32(bufp, remainp);
      id = Serialization::decode_i32(bufp, remainp);
      timeout_ms = Serialization::decode_i32(bufp, remainp);
      command = Serialization::decode_i16(bufp, remainp);
      total_len = Serialization::decode_i32(bufp, remainp);
      header_checksum = Serialization::decode_i32(bufp, remainp)
    );

    if(!checksum_i32_chk(header_checksum, base, *bufp-base-4))
      HT_THROWF(Error::COMM_HEADER_CHECKSUM_MISMATCH, 
                "header-checksum decoded-len=%d", *bufp-base);
  }
  
  void set_total_length(uint32_t len) { total_len = len; }

  void initialize_from_request_header(CommHeader &req_header) {
    flags = req_header.flags;
    gid = req_header.gid;
    id = req_header.id;
    command = req_header.command;
    total_len = 0;
  }

  const std::string to_string() const {
    std::string s = " version=" + std::to_string((int)version);
    s += " header_len=" + std::to_string((int)header_len);
    s += " flags=" + std::to_string((int)flags);
    s += " gid=" + std::to_string((int)gid);
    s += " id=" + std::to_string((int)id);
    s += " timeout_ms=" + std::to_string((int)timeout_ms);
    s += " command=" + std::to_string((int)command);
    s += " total_len=" + std::to_string((int)total_len);
    s += " header_checksum=" + std::to_string((int)header_checksum);
    return s;
  }

  uint8_t version;      //!< Protocol version
  uint8_t header_len;   //!< Length of header
  uint8_t flags;        //!< Flags
  uint32_t gid;         //!< Group ID (base of Req.ID)
  uint32_t id;          //!< Request ID
  uint32_t timeout_ms;  //!< Request timeout
  uint16_t command;     //!< Request command number
  uint32_t total_len;   //!< Total length of message including header
  uint32_t header_checksum; //!< Header checksum (excl. at computation)
};
  
}

#endif // swc_core_comm_CommHeader_h
