/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Encoder_h
#define swcdb_core_Encoder_h

#include "swcdb/core/Buffer.h"


namespace SWC { namespace Core {


//! The SWC-DB Encoder C++ namespace 'SWC::Core::Encoder'
namespace Encoder {


#ifndef SWC_DEFAULT_ENCODER
#define SWC_DEFAULT_ENCODER ZSTD
#endif

#define SWC_DEFAULT_COMM_ENCODER    SWC::Core::Encoder::Type::SWC_DEFAULT_ENCODER
#define SWC_DEFAULT_STORAGE_ENCODER SWC::Core::Encoder::Type::SWC_DEFAULT_ENCODER



enum class Type : uint8_t {
  DEFAULT = 0x00,
  PLAIN   = 0x01,
  ZLIB    = 0x02,
  SNAPPY  = 0x03,
  ZSTD    = 0x04,
  UNKNOWN = 0xff
};

const char* SWC_CONST_FUNC to_string(Type typ) noexcept;

Type SWC_PURE_FUNC encoding_from(const std::string& typ) noexcept;

SWC_CAN_INLINE
std::string repr_encoding(int typ) {
  return Encoder::to_string(Encoder::Type(typ));
}

SWC_CAN_INLINE
int from_string_encoding(const std::string& typ) noexcept {
  return int(Encoder::encoding_from(typ));
}



void decode(int& err, Type encoder,
            const uint8_t* src, size_t sz_enc,
            uint8_t *dst, size_t sz);

void encode(int& err, Type encoder,
            const uint8_t* src, size_t src_sz,
            size_t* sz_enc, DynamicBuffer& output,
            uint32_t reserve,
            bool no_plain_out=false, bool ok_more=false);



}}} // namespace SWC::Core::Encoder


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Encoder.cc"
#endif


#endif // swcdb_core_Encoder_h
