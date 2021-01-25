/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Exception.h"
#include "swcdb/core/Encoder.h"

#include <snappy.h>
#include <zlib.h>
#include <zstd.h>

namespace SWC { namespace Core {


namespace {
  const char Encoder_DEFAULT[]  = "DEFAULT";
  const char Encoder_PLAIN[]    = "PLAIN";
  const char Encoder_ZLIB[]     = "ZLIB";
  const char Encoder_SNAPPY[]   = "SNAPPY";
  const char Encoder_ZSTD[]     = "ZSTD";
  const char Encoder_UNKNOWN[]  = "UNKNOWN";
}


const char* Encoder::to_string(Encoder::Type typ) noexcept {
  switch(typ) {
    case Encoder::Type::DEFAULT:
      return Encoder_DEFAULT;
    case Encoder::Type::PLAIN:
      return Encoder_PLAIN;
    case Encoder::Type::ZLIB:
      return Encoder_ZLIB;
    case Encoder::Type::SNAPPY:
      return Encoder_SNAPPY;
    case Encoder::Type::ZSTD:
      return Encoder_ZSTD;
    case Encoder::Type::UNKNOWN:
      return Encoder_UNKNOWN;
    default:
      return "UNKNOWN";
  }
}

Encoder::Type Encoder::encoding_from(const std::string& typ) noexcept {
  switch(typ.length()) {
    case 1: {
      switch(*typ.data()) {
        case '0':
          return Encoder::Type::DEFAULT;
        case '1':
          return Encoder::Type::PLAIN;
        case '2':
          return Encoder::Type::ZLIB;
        case '3':
          return Encoder::Type::SNAPPY;
        case '4':
          return Encoder::Type::ZSTD;
        default:
          break;
      }
      break;
    }
    case 4: {
      if(!strncasecmp(typ.data(), Encoder_ZLIB, 4))
        return Encoder::Type::ZLIB;
      if(!strncasecmp(typ.data(), Encoder_ZSTD, 4))
        return Encoder::Type::ZSTD;
      break;
    }
    case 5: {
      if(!strncasecmp(typ.data(), Encoder_PLAIN, 5))
        return Encoder::Type::PLAIN;
      break;
    }
    case 6: {
      if(!strncasecmp(typ.data(), Encoder_SNAPPY, 6))
        return Encoder::Type::SNAPPY;
      break;
    }
    case 7: {
      if(!strncasecmp(typ.data(), Encoder_DEFAULT, 7))
        return Encoder::Type::DEFAULT;
      break;
    }
    default:
      break;
  }
  return Encoder::Type::UNKNOWN;
}

SWC_SHOULD_INLINE
std::string Encoder::repr_encoding(int typ) {
  return Encoder::to_string(Encoder::Type(typ));
}

SWC_SHOULD_INLINE
int Encoder::from_string_encoding(const std::string& typ) noexcept {
  return int(Encoder::encoding_from(typ));
}



void Encoder::decode(
            int& err, Encoder::Type encoder,
            const uint8_t* src, size_t sz_enc,
            uint8_t *dst, size_t sz) {

  switch(encoder) {
    case Encoder::Type::ZLIB: {
      z_stream strm;
      memset(&strm, 0, sizeof(z_stream));
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.avail_in = 0;
      strm.next_in = Z_NULL;
      if(::inflateInit(&strm) != Z_OK) {
        err = Error::ENCODER_DECODE;
        return;
      }
      strm.avail_in = sz_enc;
      strm.next_in = const_cast<Bytef*>(src);
      strm.avail_out = sz;
      strm.next_out = dst;
      if(::inflate(&strm, Z_NO_FLUSH) != Z_STREAM_END || strm.avail_out)
        err = Error::ENCODER_DECODE;
      ::inflateReset(&strm);
      return;
    }

    case Encoder::Type::SNAPPY: {
      if(!snappy::RawUncompress(
            reinterpret_cast<const char*>(src), sz_enc,
            reinterpret_cast<char*>(dst)))
        err = Error::ENCODER_DECODE;
      return;
    }

    case Encoder::Type::ZSTD: {
      if(ZSTD_decompress(
          static_cast<void*>(dst), sz,
          static_cast<void*>(const_cast<uint8_t*>(src)), sz_enc) != sz)
        err = Error::ENCODER_DECODE;
      return;
    }

    default: {
      //SWC_ASSERT(encoder==Encoder::Type::PLAIN);
      break;
    }
  }
}


void Encoder::encode(
            int&, Encoder::Type encoder,
            const uint8_t* src, size_t src_sz,
            size_t* sz_enc, DynamicBuffer& output,
            uint32_t reserve, bool no_plain_out) {

  switch(encoder) {
    case Encoder::Type::ZLIB: {

      z_stream strm;
      memset(&strm, 0, sizeof(z_stream));
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      if(::deflateInit(&strm, Z_DEFAULT_COMPRESSION) == Z_OK) {

        uint32_t avail_out = src_sz + 6 + (((src_sz / 16000) + 1) * 5);
        output.ensure(reserve + avail_out);
        output.ptr += reserve;

        strm.avail_out = avail_out;
        strm.next_out = output.ptr;
        strm.avail_in = src_sz;
        strm.next_in = const_cast<Bytef*>(src);
        if(::deflate(&strm, Z_FINISH) == Z_STREAM_END)
          *sz_enc = avail_out - strm.avail_out;
      }
      ::deflateReset(&strm);
      if(*sz_enc && *sz_enc < src_sz) {
        output.ptr += *sz_enc;
        return;
      }
      break;
    }

    case Encoder::Type::SNAPPY: {
      output.ensure(reserve + snappy::MaxCompressedLength(src_sz));
      output.ptr += reserve;
      snappy::RawCompress(
        reinterpret_cast<const char*>(src), src_sz,
        reinterpret_cast<char*>(output.ptr), sz_enc);
      if(*sz_enc && *sz_enc < src_sz) {
        output.ptr += *sz_enc;
        return;
      }
      break;
    }

    case Encoder::Type::ZSTD: {
      size_t const avail_out = ZSTD_compressBound(src_sz);
      output.ensure(reserve + avail_out);
      output.ptr += reserve;

      *sz_enc = ZSTD_compress(
        static_cast<void*>(output.ptr), avail_out,
        static_cast<void*>(const_cast<uint8_t*>(src)), src_sz,
        ZSTD_CLEVEL_DEFAULT
      );
      if(*sz_enc && !ZSTD_isError(*sz_enc) && *sz_enc < src_sz) {
        output.ptr += *sz_enc;
        return;
      }
      break;
    }

    default:
      break;
  }

  *sz_enc = 0;
  if(no_plain_out)
    return;

  output.free();
  output.ensure(reserve + src_sz);
  output.ptr += reserve;
  if(src_sz)
    output.add_unchecked(src, src_sz);
}


}}
