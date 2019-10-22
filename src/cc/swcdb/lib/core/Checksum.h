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
 * along with Hypertable. If not, see <http://www.gnu.org/licenses/>
 */

/** @file
 * Implementation of checksum routines.
 * This file implements the fletcher32 checksum algorithm.
 */

#ifndef swc_core_Checksum_h
#define swc_core_Checksum_h

#include "Logger.h"
#include "Serialization.h"

namespace SWC {

/** Compute fletcher32 checksum for arbitary data. See
  * http://en.wikipedia.org/wiki/Fletcher%27s_checksum for more information
  * about the algorithm. Fletcher32 is the default checksum.
  *
  * @param data Pointer to the input data
  * @param len Input data length in bytes
  * @return The calculated checksum
*/
extern uint32_t fletcher32(const void *data, size_t len);


inline bool checksum_i32_chk(uint32_t checksum, 
                             const uint8_t *base, uint32_t len){
  uint32_t computed = fletcher32(base, len);
  if(checksum == computed)
    return true;
  HT_ERRORF("checksum_i32_chk, original(%u) != computed(%u)", 
            checksum, computed);
  return false;
}

inline bool checksum_i32_chk(uint32_t checksum, 
                             const uint8_t *base, uint32_t len, 
                             uint32_t offset){
  memset((void *)(base+offset), 0, 4);
  return checksum_i32_chk(checksum, base, len);
}

inline void checksum_i32(const uint8_t *start, const uint8_t *end, 
                        uint8_t **ptr){
  Serialization::encode_i32(ptr, fletcher32(start, end-start));
}

inline void checksum_i32(const uint8_t *start, const uint8_t *end, 
                        uint8_t **ptr, uint32_t& checksum){
  checksum = fletcher32(start, end-start);
  Serialization::encode_i32(ptr, checksum);
}

  /** @}*/

} // namespace SWC

#endif /* swc_core_Checksum_h */
