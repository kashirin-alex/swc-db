/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_core_Encoder_h
#define swc_core_Encoder_h

#include "swcdb/lib/db/Types/Encoding.h"
#include <snappy.h>

namespace SWC { namespace Encoder {


void decode(int& err, Types::Encoding encoder, 
            const uint8_t* src, size_t sz_enc, 
            uint8_t *dst, size_t sz){

  switch(encoder){
    case Types::Encoding::ZLIB: {
      break;
    }

    case Types::Encoding::SNAPPY: {
      if(!snappy::RawUncompress((const char *)src, sz_enc, (char *)dst))
        err = Error::ENCODER_DECODE;
      return;
    }

    default: {
      //HT_ASSERT(encoder==Types::Encoding::PLAIN);
      break;
    }
  }
}

void encode(int& err, Types::Encoding encoder, 
            const uint8_t* src, size_t src_sz, 
            size_t* sz_enc, DynamicBuffer& output, 
            uint32_t reserve){
  
  switch(encoder){
    case Types::Encoding::ZLIB: {

      if(*sz_enc)
        break;
    }

    case Types::Encoding::SNAPPY: {
      output.ensure(reserve + snappy::MaxCompressedLength(src_sz));
      output.ptr += reserve;
      snappy::RawCompress((const char *)src, src_sz, 
                          (char *)output.ptr, sz_enc);
      if(*sz_enc && *sz_enc < src_sz) {
        output.ptr += *sz_enc;
        break;
      }
      *sz_enc = 0; 
      output.free();
    }

    default: {
      output.ensure(reserve + src_sz);
      output.ptr += reserve;
      if(src_sz > 0)
        output.add_unchecked(src, src_sz);
    }
  }
  
}

}}

#endif // swc_core_Time_h
