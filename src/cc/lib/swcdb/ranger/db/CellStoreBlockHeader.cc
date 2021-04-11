/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStoreBlockHeader.h"


namespace SWC { namespace Ranger { namespace CellStore { namespace Block {



Header::Header(DB::Types::KeySeq key_seq) noexcept
              : offset_data(0),
                interval(key_seq),
                is_any(0),
                encoder(DB::Types::Encoder::UNKNOWN),
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

Header::Header(Header&& other) noexcept
              : offset_data(other.offset_data),
                interval(std::move(other.interval)),
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
  Serialization::encode_i8(bufp, uint8_t(encoder));
  Serialization::encode_i32(bufp, size_enc);
  Serialization::encode_i32(bufp, size_plain);
  Serialization::encode_i32(bufp, cells_count);
  if(size_enc)
    Core::checksum_i32(base + Header::SIZE, size_enc, bufp, checksum_data);
  else
    Serialization::encode_i32(bufp, 0);
  Core::checksum_i32(base, *bufp, bufp);
}

void Header::decode(const uint8_t** bufp, size_t* remainp) {
  encoder = DB::Types::Encoder(Serialization::decode_i8(bufp, remainp));
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
  Serialization::encode_i8(bufp, uint8_t(encoder));
  Serialization::encode_vi32(bufp, size_enc);
  Serialization::encode_vi32(bufp, size_plain);
  Serialization::encode_vi32(bufp, cells_count);
  Serialization::encode_vi32(bufp, checksum_data);
}

void Header::decode_idx(const uint8_t** bufp, size_t* remainp) {
  offset_data = Serialization::decode_vi64(bufp, remainp);
  interval.decode(bufp, remainp, true);
  is_any = Serialization::decode_i8(bufp, remainp);
  encoder = DB::Types::Encoder(Serialization::decode_i8(bufp, remainp));
  size_enc = Serialization::decode_vi32(bufp, remainp);
  size_plain = Serialization::decode_vi32(bufp, remainp);
  cells_count = Serialization::decode_vi32(bufp, remainp);
  checksum_data = Serialization::decode_vi32(bufp, remainp);
}

void Header::print(std::ostream& out) const {
  out << "offset=" << offset_data
      << " encoder=" << Core::Encoder::to_string(encoder)
      << " enc/size=" << size_enc << '/' << size_plain
      << " cells_count=" << cells_count
      << " checksum=" << checksum_data
      << " is_any=" << int(is_any)
      << ' ' << interval;
}


}}}} //  namespace SWC::Ranger::CellStore::Block

