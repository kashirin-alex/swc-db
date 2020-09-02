/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_CellStoreBlockHeader_h
#define swc_ranger_db_CellStoreBlockHeader_h



namespace SWC { namespace Ranger { namespace CellStore { namespace Block {


struct Header final {
  static const uint8_t SIZE = 21; // not-idx

  static const uint8_t ANY_BEGIN  = 0x01;
  static const uint8_t ANY_END    = 0x2;

  uint64_t             offset_data;
  DB::Cells::Interval  interval;
  uint8_t              is_any;

  Types::Encoding      encoder;
  uint32_t             size_plain;
  uint32_t             size_enc;
  uint32_t             cells_count;
  uint32_t             checksum_data;
  
  Header(Types::KeySeq key_seq);
  
  Header(const Header& other);

  ~Header();

  void encode(uint8_t** bufp);

  void decode(const uint8_t** bufp, size_t* remainp);
  
  size_t encoded_length_idx() const;

  void encode_idx(uint8_t** bufp) const;

  void decode_idx(const uint8_t** bufp, size_t* remainp);

  std::string to_string() const;

};



}}}} // namespace SWC::Ranger::CellStore::Block

#endif // swc_ranger_db_CellStoreBlockHeader_h