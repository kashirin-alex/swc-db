/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStoreBlockHeader.h"


namespace SWC { namespace Ranger { namespace CellStore { namespace Block {



Header::Header(Types::KeySeq key_seq) 
              : offset_data(0), 
                interval(key_seq), 
                is_any(0),
                encoder(Types::Encoding::UNKNOWN),
                size_plain(0),
                size_enc(0),
                cells_count(0),
                checksum_data(0) {
}

Header::Header(const Header& other) 
              : offset_data(other.offset_data),
                interval(other.interval),
                is_any(other.is_any),
                encoder(other.encoder),
                size_plain(other.size_plain),
                size_enc(other.size_enc),
                cells_count(other.cells_count),
                checksum_data(other.checksum_data) {
}

Header::~Header() { }

void Header::encode(uint8_t** bufp) {
  const uint8_t* base = *bufp;
  Serialization::encode_i8(bufp, (uint8_t)encoder);
  Serialization::encode_i32(bufp, size_enc);
  Serialization::encode_i32(bufp, size_plain);
  Serialization::encode_i32(bufp, cells_count);
  if(size_enc) 
    checksum_i32(base + Header::SIZE, size_enc, bufp, checksum_data);
  else 
    Serialization::encode_i32(bufp, 0);
  checksum_i32(base, *bufp, bufp);
}

void Header::decode(const uint8_t** bufp, size_t* remainp) {
  encoder = (Types::Encoding)Serialization::decode_i8(bufp, remainp);
  size_enc = Serialization::decode_i32(bufp, remainp);
  size_plain = Serialization::decode_i32(bufp, remainp);
  cells_count = Serialization::decode_i32(bufp, remainp);
  checksum_data = Serialization::decode_i32(bufp, remainp);
  // i32(checksum) remains
}

size_t Header::encoded_length_idx() const {
  return Serialization::encoded_length_vi64(offset_data) 
        + interval.encoded_length()
        + 2
        + Serialization::encoded_length_vi32(size_enc)
        + Serialization::encoded_length_vi32(size_plain)
        + Serialization::encoded_length_vi32(cells_count)
        + Serialization::encoded_length_vi32(checksum_data);
}

void Header::encode_idx(uint8_t** bufp) const {
  Serialization::encode_vi64(bufp, offset_data);
  interval.encode(bufp);
  Serialization::encode_i8(bufp, is_any);
  Serialization::encode_i8(bufp, (uint8_t)encoder);
  Serialization::encode_vi32(bufp, size_enc);
  Serialization::encode_vi32(bufp, size_plain);
  Serialization::encode_vi32(bufp, cells_count);
  Serialization::encode_vi32(bufp, checksum_data);
}

void Header::decode_idx(const uint8_t** bufp, size_t* remainp) {
  offset_data = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp, false);
  is_any = Serialization::decode_i8(bufp, remainp);
  encoder = (Types::Encoding)Serialization::decode_i8(bufp, remainp);
  size_enc = Serialization::decode_vi32(bufp, remainp);
  size_plain = Serialization::decode_vi32(bufp, remainp);
  cells_count = Serialization::decode_vi32(bufp, remainp);
  checksum_data = Serialization::decode_vi32(bufp, remainp);
}

void Header::print(std::ostream& out) const {
  out << "offset=" << offset_data
      << " encoder=" << Types::to_string(encoder)
      << " enc/size=" << size_enc << '/' << size_plain
      << " cells_count=" << cells_count
      << " checksum=" << checksum_data
      << " is_any=" << (int)is_any
      << ' ' << interval;
}


}}}} //  namespace SWC::Ranger::CellStore::Block

