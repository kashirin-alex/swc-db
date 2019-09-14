/*
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
 * Definitions for CommHeader.
 * This file contains method definitions for CommHeader, a class that manages
 * construction, serialization, and deserialization of an AsyncComm message
 * header.
 */

#include "CommHeader.h"
#include "swcdb/lib/core/Error.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Logger.h"
#include "swcdb/lib/core/Checksum.h"


using namespace SWC;


void CommHeader::encode(uint8_t **bufp) {
  uint8_t *base = *bufp;
  Serialization::encode_i8(bufp, version);
  Serialization::encode_i8(bufp, header_len);
  Serialization::encode_i8(bufp, flags);
  Serialization::encode_i32(bufp, gid);
  Serialization::encode_i32(bufp, id);
  Serialization::encode_i32(bufp, timeout_ms);
  Serialization::encode_i16(bufp, command);
  
  Serialization::encode_i32(bufp, total_len);
  Serialization::encode_i32(bufp, 0);

  // compute and serialize header checksum
  header_checksum = fletcher32(base, (*bufp)-base);
  *bufp -= 4;
  Serialization::encode_i32(bufp, header_checksum);
}

void CommHeader::decode(const uint8_t **bufp, size_t *remainp) {
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
  memset((void *)(*bufp-4), 0, 4);
  uint32_t checksum = fletcher32(base, *bufp-base);
  if (checksum != header_checksum){
    HT_DEBUGF("checksum %u != %u", checksum, header_checksum);
    HT_THROWF(Error::COMM_HEADER_CHECKSUM_MISMATCH, 
              "checksum %u != %u", checksum, header_checksum);
  }
}
