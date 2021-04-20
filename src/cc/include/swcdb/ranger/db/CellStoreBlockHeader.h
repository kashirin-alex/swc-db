/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CellStoreBlockHeader_h
#define swcdb_ranger_db_CellStoreBlockHeader_h



namespace SWC { namespace Ranger {


//! The SWC-DB CellStore C++ namespace 'SWC::Ranger::CellStore'
namespace CellStore {


//! The SWC-DB Block C++ namespace 'SWC::Ranger::CellStore::Block'
namespace Block {


struct Header final {
  static const uint8_t SIZE = 21; // not-idx

  static const uint8_t ANY_BEGIN  = 0x01;
  static const uint8_t ANY_END    = 0x2;

  uint64_t                  offset_data;
  DB::Cells::Interval       interval;
  uint8_t                   is_any;

  DB::Types::Encoder        encoder;
  uint32_t                  size_plain;
  uint32_t                  size_enc;
  uint32_t                  cells_count;
  uint32_t                  checksum_data;

  Header(DB::Types::KeySeq key_seq) noexcept;

  Header(const Header& other);

  Header(Header&& other) noexcept;

  //~Header() { }

  void encode(uint8_t** bufp);

  void decode(const uint8_t** bufp, size_t* remainp);

  size_t encoded_length_idx() const;

  void encode_idx(uint8_t** bufp) const;

  void decode_idx(const uint8_t** bufp, size_t* remainp);

  void print(std::ostream& out) const;

};



}}}} // namespace SWC::Ranger::CellStore::Block

#endif // swcdb_ranger_db_CellStoreBlockHeader_h