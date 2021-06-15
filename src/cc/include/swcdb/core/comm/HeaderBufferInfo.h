/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_HeaderBufferInfo_h
#define swcdb_core_comm_HeaderBufferInfo_h


#include "swcdb/core/Compat.h"
#include "swcdb/core/Encoder.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"


namespace SWC { namespace Comm {


struct BufferInfo final {

  SWC_CAN_INLINE
  BufferInfo()  noexcept
    : size(0),
      size_plain(0),
      chksum(0),
      encoder(Core::Encoder::Type::PLAIN) {
  }

  SWC_CAN_INLINE
  void reset() noexcept {
    size = size_plain = chksum = 0;
    encoder = Core::Encoder::Type::PLAIN;
  }

  SWC_CAN_INLINE
  uint8_t encoded_length() const noexcept {
    uint8_t sz = Serialization::encoded_length_vi32(size) + 5;
    if(encoder != Core::Encoder::Type::PLAIN)
      sz += Serialization::encoded_length_vi32(size_plain);
    return sz;
  }

  SWC_CAN_INLINE
  void encode(uint8_t** bufp) const {
    Serialization::encode_vi32(bufp, size);
    Serialization::encode_i8(bufp, uint8_t(encoder));
    if(encoder != Core::Encoder::Type::PLAIN)
      Serialization::encode_vi32(bufp, size_plain);
    Serialization::encode_i32(bufp, chksum);
  }

  SWC_CAN_INLINE
  void decode(const uint8_t** bufp, size_t* remainp) {
    size = Serialization::decode_vi32(bufp, remainp);
    encoder = Core::Encoder::Type(Serialization::decode_i8(bufp, remainp));
    if(encoder != Core::Encoder::Type::PLAIN)
      size_plain = Serialization::decode_vi32(bufp, remainp);
    chksum = Serialization::decode_i32(bufp, remainp);
  }

  void encode(Core::Encoder::Type encoder, StaticBuffer& data);

  void decode(int& err, StaticBuffer& data) const;

  void print(std::ostream& out) const;

  uint32_t            size;        //!< Buffer size
  uint32_t            size_plain;  //!< Buffer set if Encoder not PLAIN
  uint32_t            chksum;      //!< Buffer checksum
  Core::Encoder::Type encoder;     //!< Buffer Encoder

} __attribute__((packed));


}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/HeaderBufferInfo.cc"
#endif

#endif // swcdb_core_comm_HeaderBufferInfo_h
