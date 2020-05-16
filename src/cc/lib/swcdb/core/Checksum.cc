/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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


#include "swcdb/core/Logger.h"
#include "swcdb/core/Serialization.h"

namespace SWC {

 
#define SWC_F32_DO1(buf,i) \
  sum1 += ((uint16_t)buf[i] << 8) | buf[i + 1]; sum2 += sum1
#define SWC_F32_DO2(buf, i)  SWC_F32_DO1(buf, i); SWC_F32_DO1(buf, i + 2);
#define SWC_F32_DO4(buf, i)  SWC_F32_DO2(buf, i); SWC_F32_DO2(buf, i + 4);
#define SWC_F32_DO8(buf, i)  SWC_F32_DO4(buf, i); SWC_F32_DO4(buf, i + 8);
#define SWC_F32_DO16(buf, i) SWC_F32_DO8(buf, i); SWC_F32_DO8(buf, i + 16);

uint32_t fletcher32(const void *data8, size_t len8) {
  /* data may not be aligned properly and would segfault on
   * many systems if cast and used as 16-bit words
   */
  const uint8_t *data = (const uint8_t *)data8;
  uint32_t sum1 = 0xffff, sum2 = 0xffff;
  size_t len = len8 / 2; /* loop works on 16-bit words */

  uint16_t tlen;
  while (len) {
    /* 360 is the largest number of sums that can be
     * performed without integer overflow
     */
    len -= (tlen = len > 360 ? 360 : len);

     while (tlen >= 16) {
      SWC_F32_DO16(data, 0);
      data += 32;
      tlen -= 16;
    }

    if(tlen) do {
      SWC_F32_DO1(data, 0);
      data += 2;
    } while(--tlen);

    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  }

  /* Check for odd number of bytes */
  if (len8 & 1) {
    sum1 += ((uint16_t)*data) << 8;
    sum2 += sum1;
    sum1 = (sum1 & 0xffff) + (sum1 >> 16);
    sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  }

  /* Second reduction step to reduce sums to 16 bits */
  sum1 = (sum1 & 0xffff) + (sum1 >> 16);
  sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  return (sum2 << 16) | sum1;
}


SWC_SHOULD_INLINE
bool checksum_i32_chk(uint32_t checksum, const uint8_t *base, uint32_t len) {
  uint32_t computed = fletcher32(base, len);
  if(checksum == computed)
    return true;
  SWC_LOGF(LOG_ERROR, "checksum_i32_chk, original(%u) != computed(%u)", 
            checksum, computed);
  return false;
}

SWC_SHOULD_INLINE
bool checksum_i32_chk(uint32_t checksum, const uint8_t *base, uint32_t len, 
                      uint32_t offset) {
  memset((void *)(base+offset), 0, 4);
  return checksum_i32_chk(checksum, base, len);
}

SWC_SHOULD_INLINE
void checksum_i32(const uint8_t *start, size_t len, uint8_t **ptr) {
  Serialization::encode_i32(ptr, fletcher32(start, len));
}

SWC_SHOULD_INLINE
void checksum_i32(const uint8_t *start, const uint8_t *end, uint8_t **ptr) {
  checksum_i32(start, end-start, ptr);
}

SWC_SHOULD_INLINE
void checksum_i32(const uint8_t *start, const uint8_t *end, uint8_t **ptr, 
                  uint32_t& checksum) {
  checksum = fletcher32(start, end-start);
  Serialization::encode_i32(ptr, checksum);
}


} // namespace SWC
