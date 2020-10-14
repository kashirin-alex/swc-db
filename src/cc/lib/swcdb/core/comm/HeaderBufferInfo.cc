/* 
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Serialization.h"
#include "swcdb/core/Checksum.h"
#include "swcdb/core/comm/HeaderBufferInfo.h"


namespace SWC { namespace Comm {


BufferInfo::BufferInfo()
  : size(0), 
    encoder(Core::Encoder::Type::PLAIN), 
    size_plain(0), 
    chksum(0) {
}

SWC_SHOULD_INLINE
void BufferInfo::reset() {
  size = 0;
  encoder = Core::Encoder::Type::PLAIN;
  size_plain = 0;
  chksum = 0;
}

size_t BufferInfo::encoded_length() const {
  size_t sz = Serialization::encoded_length_vi32(size) + 5;
  if(encoder != Core::Encoder::Type::PLAIN)
    sz += Serialization::encoded_length_vi32(size_plain);
  return sz;
}

void BufferInfo::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, size);
  Serialization::encode_i8(bufp, (uint8_t)encoder);
  if(encoder != Core::Encoder::Type::PLAIN)
    Serialization::encode_vi32(bufp, size_plain); 
  Serialization::encode_i32(bufp, chksum);
}

void BufferInfo::decode(const uint8_t** bufp, size_t* remainp) {
  size = Serialization::decode_vi32(bufp, remainp);
  encoder = (Core::Encoder::Type)Serialization::decode_i8(bufp, remainp);
  if(encoder != Core::Encoder::Type::PLAIN)
    size_plain = Serialization::decode_vi32(bufp, remainp);
  chksum = Serialization::decode_i32(bufp, remainp);
}

void BufferInfo::encode(Core::Encoder::Type _enc, StaticBuffer& data) {
  if(_enc != Core::Encoder::Type::PLAIN && 
     encoder == Core::Encoder::Type::PLAIN &&
     data.size > 32) { // at least size if encoder not encrypt-type
    
    int err = Error::OK;
    size_t len_enc = 0;
    DynamicBuffer output;
    Core::Encoder::encode(err, _enc, data.base, data.size, 
                          &len_enc, output, 0, true);
    if(len_enc) {
      encoder = _enc;
      size_plain = data.size;
      data.set(output);
    }
  }

  size   = data.size;
  chksum = Core::checksum32(data.base, data.size);
}

void BufferInfo::decode(int& err, StaticBuffer& data) const {
  if(size_plain) {
    StaticBuffer decoded_buf((size_t)size_plain);
    Core::Encoder::decode(
      err, encoder, 
      data.base, data.size, 
      decoded_buf.base, size_plain
    );
    if(!err)
      data.set(decoded_buf);
  }
}

void BufferInfo::print(std::ostream& out) const {
  out << " Buffer(sz=" << size
      << " enc=" << Core::Encoder::to_string(encoder)
      << " szplain=" << size_plain
      << " chk=" << chksum << ')';
}


}}
