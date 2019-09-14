/* -*- c++ -*-
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** @file
 * Declarations for CommHeader.
 * This file contains type declarations for CommHeader, a class that manages
 * construction, serialization, and deserialization of an AsyncComm message
 * header.
 */

#ifndef swc_core_comm_CommHeader_h
#define swc_core_comm_CommHeader_h

#include "swcdb/lib/core/Compat.h"

namespace SWC {

  /** @addtogroup AsyncComm
   *  @{
   */

  /** Header for messages transmitted via AsyncComm.
   */
  class CommHeader {

  public:

    static const uint8_t PROTOCOL_VERSION = 1;
 
    static const uint8_t FIXED_LENGTH = 25;

    /** Enumeration constants for bits in flags field
     */
    enum Flags {
      FLAGS_BIT_REQUEST          = 0x1, //!< Request message
      FLAGS_BIT_IGNORE_RESPONSE  = 0x2, //!< Response should be ignored
      FLAGS_BIT_URGENT           = 0x4, //!< Request is urgent
    };

    /** Enumeration constants for flags field bitmaks
     */
    enum FlagMask {
      FLAGS_MASK_REQUEST          = 0xE, //!< Request message bit
      FLAGS_MASK_IGNORE_RESPONSE  = 0xD, //!< Response should be ignored bit
      FLAGS_MASK_URGENT           = 0xB, //!< Request is urgent bit
    };

    /** Default constructor.
     */
    CommHeader()
      : version(1), header_len(FIXED_LENGTH), flags(0),
        gid(0), id(0), timeout_ms(0), command(0), 
        total_len(0), header_checksum(0) {  }

    /** Constructor taking command number and optional timeout.
     * @param cmd Command number
     * @param timeout Request timeout
     */
    CommHeader(uint64_t cmd, uint32_t timeout=0)
      : version(1), header_len(FIXED_LENGTH), flags(0),
        gid(0), id(0), timeout_ms(timeout), command(cmd), 
        total_len(0), header_checksum(0) {  }

    /** Returns fixed length of header.
     * @return Fixed length of header
     */
    size_t fixed_length() const { return FIXED_LENGTH; }

    /** Returns encoded length of header.
     * @return Encoded length of header
     */
    size_t encoded_length() const { return FIXED_LENGTH; }

    /** Encode header to memory pointed to by <code>*bufp</code>.
     * The <code>bufp</code> pointer is advanced to address immediately
     * following the encoded header.
     * @param bufp Address of memory pointer to where header is to be encoded.
     */
    void encode(uint8_t **bufp);
    
    /** Decode serialized header at <code>*bufp</code>
     * The <code>bufp</code> pointer is advanced to the address immediately
     * following the decoded header and <code>remainp</code> is decremented
     * by the length of the serialized header.
     * @param bufp Address of memory pointer to where header is to be encoded.
     * @param remainp Pointer to valid bytes remaining in buffer (decremented
     *                by call)
     * @throws Error::COMM_BAD_HEADER If fixed header size is less than
     * <code>*remainp</code>.
     * @throws Error::COMM_HEADER_CHECKSUM_MISMATCH If computed checksum does
     * not match checksum field
     */
    void decode(const uint8_t **bufp, size_t *remainp);

    /** Set total length of message (header + payload).
     * @param len Total length of message (header + payload)
     */
    void set_total_length(uint32_t len) { total_len = len; }

    /** Initializes header from <code>req_header</code>.
     * This method is typically used to initialize a response header
     * from a corresponding request header.
     * @param req_header Request header from which to initialize
     */
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

    uint8_t version;     //!< Protocol version
    uint8_t header_len;  //!< Length of header
    uint8_t flags;      //!< Flags
    uint32_t gid;        //!< Group ID (see ApplicationQueue)
    uint32_t id;         //!< Request ID
    uint32_t timeout_ms; //!< Request timeout
    uint16_t command;    //!< Request command number
    uint32_t total_len;  //!< Total length of message including header
    uint32_t header_checksum; //!< Header checksum (computed with this member 0)
  };
  /** @}*/
}

#endif // swc_core_comm_CommHeader_h
