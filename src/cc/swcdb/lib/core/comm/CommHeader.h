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
  static const uint8_t PREFIX_LENGTH    = 2;
  static const uint8_t FIXED_LENGTH     = PREFIX_LENGTH+20;

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

  CommHeader(uint64_t cmd=0, uint32_t timeout=0)
            : version(1), header_len(0), flags(0),
              gid(0), id(0), timeout_ms(timeout), command(cmd), 
              buffers(0), 
              data_size(0), data_chksum(0), 
              data_ext_size(0), data_ext_chksum(0), 
              checksum(0) { 
  }
  
  virtual ~CommHeader() { }

  void set(uint64_t cmd=0, uint32_t timeout=0) {
    command     = cmd;
    timeout_ms  = timeout;
  }

  const size_t encoded_length() { 
    header_len = FIXED_LENGTH;
    buffers = 0;
    if(data_size) {
      buffers++;
      header_len += Serialization::encoded_length_vi32(data_size) + 4;
    }
    if(data_ext_size) {
      buffers++;
      header_len += Serialization::encoded_length_vi32(data_ext_size) + 4;
    }
    return header_len; 
  }

  void encode(uint8_t **bufp) const {
    uint8_t *base = *bufp;
    Serialization::encode_i8(bufp,  version);
    Serialization::encode_i8(bufp,  header_len);
    Serialization::encode_i8(bufp,  flags);
    Serialization::encode_i32(bufp, gid);
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

  void decode_prefix(const uint8_t **bufp, size_t *remainp) {
    if (*remainp < 2)
      HT_THROWF(Error::COMM_BAD_HEADER,
                "Header size %d is less than the fixed length %d",
                (int)*remainp, 2);

    version = Serialization::decode_i8(bufp, remainp);
    header_len = Serialization::decode_i8(bufp, remainp);
  }

  void decode(const uint8_t **bufp, size_t *remainp) {
    const uint8_t *base = *bufp;
    *bufp += 2;
    
    flags = Serialization::decode_i8(bufp, remainp);
    gid = Serialization::decode_i32(bufp, remainp);
    id = Serialization::decode_i32(bufp, remainp);
    timeout_ms = Serialization::decode_i32(bufp, remainp);
    command = Serialization::decode_i16(bufp, remainp);
    
    if((buffers = Serialization::decode_i8(bufp, remainp)) > 0) {
      data_size = Serialization::decode_vi32(bufp, remainp);
      data_chksum = Serialization::decode_i32(bufp, remainp);    
      if(buffers > 1) {
        data_ext_size = Serialization::decode_vi32(bufp, remainp);
        data_ext_chksum = Serialization::decode_i32(bufp, remainp);
      }
    }

    checksum = Serialization::decode_i32(bufp, remainp);
    if(!checksum_i32_chk(checksum, base, header_len-4))
      HT_THROWF(Error::COMM_HEADER_CHECKSUM_MISMATCH, 
                "header-checksum decoded-len=%d", *bufp-base);
  }

  void initialize_from_request_header(CommHeader &req_header) {
    flags = req_header.flags;
    gid = req_header.gid;
    id = req_header.id;
    command = req_header.command;
    buffers = 0;
    data_size = 0;
    data_chksum = 0;
    data_ext_size = 0;
    data_ext_chksum = 0;
  }

  const std::string to_string() const {
    std::string s = " version=" + std::to_string((int)version);
    s += " header_len=" + std::to_string((int)header_len);
    s += " flags=" + std::to_string((int)flags);
    s += " gid=" + std::to_string((int)gid);
    s += " id=" + std::to_string((int)id);
    s += " timeout_ms=" + std::to_string((int)timeout_ms);
    s += " command=" + std::to_string((int)command);
    s += " buffers=" + std::to_string(buffers);
    s += " data(sz=" + std::to_string(data_size);
    s += " chk=" + std::to_string(data_chksum) + ")";
    s += " ext(sz=" + std::to_string(data_ext_size);
    s += " chk=" + std::to_string(data_ext_chksum) + ")";
    s += " checksum=" + std::to_string((int)checksum);
    return s;
  }

  uint8_t  version;         //!< Protocol version
  uint8_t  header_len;      //!< Length of header
  uint8_t  flags;           //!< Flags
  uint32_t gid;             //!< Group ID (base of Req.ID)
  uint32_t id;              //!< Request ID
  uint32_t timeout_ms;      //!< Request timeout
  uint16_t command;         //!< Request command number
  uint8_t  buffers;         //!< number of buffers from 0 to 2 (data+data_ext) 

  uint32_t data_size;       //!< Data size
  uint32_t data_chksum;     //!< Data checksum
  uint32_t data_ext_size;   //!< DataExt size
  uint32_t data_ext_chksum; //!< DataExt checksum

  uint32_t checksum;        //!< Header checksum (excl. it self)
     
};
  
}

#endif // swc_core_comm_CommHeader_h
