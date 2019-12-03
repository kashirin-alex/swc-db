/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_core_Checksum_h
#define swc_core_Checksum_h

namespace SWC {

uint32_t fletcher32(const void *data8, size_t len8);


bool checksum_i32_chk(uint32_t checksum, const uint8_t *base, uint32_t len);

bool checksum_i32_chk(uint32_t checksum, const uint8_t *base, uint32_t len, 
                      uint32_t offset);

void checksum_i32(const uint8_t *start, size_t len, uint8_t **ptr);

void checksum_i32(const uint8_t *start, const uint8_t *end, uint8_t **ptr);

void checksum_i32(const uint8_t *start, const uint8_t *end, uint8_t **ptr, 
                  uint32_t& checksum);


} // namespace SWC

#ifdef SWC_IMPL_SOURCE
#include "../../../lib/swcdb/core/Checksum.cc"
#endif 

#endif /* swc_core_Checksum_h */
