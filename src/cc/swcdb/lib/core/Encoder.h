/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_Encoder_h
#define swc_core_Encoder_h

#include "swcdb/lib/db/Types/Encoding.h"

namespace SWC { namespace Encoder {


void decode(Types::Encoding encoder, 
            const uint8_t* src, size_t sz_enc, 
            uint8_t **dst, size_t sz, 
            int& err){
}

void encode(Types::Encoding encoder, const uint8_t* src, size_t src_sz, 
            size_t* sz_enc, DynamicBuffer& output, uint32_t reserve, int& err){
  
  switch(encoder){
    case Types::Encoding::ZLIB:

      if(*sz_enc)
        break;

    case Types::Encoding::SNAPPY:

      if(*sz_enc)
        break;

    default: {
      output.ensure(reserve+src_sz);
      output.ptr += reserve;
      output.add_unchecked(src, src_sz);
    }
  }
  
}

}}

#endif // swc_core_Time_h
