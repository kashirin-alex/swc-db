/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsKeyIntervals_h
#define swcdb_db_cells_SpecsKeyIntervals_h


#include "swcdb/db/Types/KeySeq.h"
#include "swcdb/db/Cells/SpecsKey.h"


namespace SWC { namespace DB { namespace Specs {


struct KeyInterval {
  Key  start, finish;

  KeyInterval() noexcept { }

  KeyInterval(const KeyInterval& other);

  KeyInterval(KeyInterval&& other) noexcept;

  KeyInterval(const Key& start, const Key& finish);

  KeyInterval(Key&& start, Key&& finish) noexcept;

  KeyInterval(const uint8_t** bufp, size_t* remainp);

  KeyInterval& operator=(const KeyInterval& other);

  KeyInterval& operator=(KeyInterval&& other) noexcept;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

};



class KeyIntervals : public std::vector<KeyInterval> {
  public:

  typedef std::vector<KeyInterval> Vec;
  using Vec::insert;
  using Vec::emplace_back;


  KeyIntervals() noexcept { }

  KeyIntervals(const KeyIntervals& other);

  KeyIntervals(KeyIntervals&& other) noexcept;

  KeyIntervals& operator=(const KeyIntervals& other);

  KeyIntervals& operator=(KeyIntervals&& other) noexcept;

  void copy(const KeyIntervals& other);

  void move(KeyIntervals& other) noexcept;

  KeyInterval& add();

  KeyInterval& add(const KeyInterval& other);

  KeyInterval& add(KeyInterval&& other);

  KeyInterval& add(const Key& start, const Key& finish);

  KeyInterval& add(Key&& start, Key&& finish);

  size_t size_of_internal() const noexcept;

  bool equal(const KeyIntervals& other) const noexcept;

  bool is_matching(const Types::KeySeq key_seq,
                   const DB::Cell::Key& cellkey) const;

  bool is_matching_start(const Types::KeySeq key_seq,
                         const DB::Cell::Key& cellkey) const;

  size_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty,
               const std::string& offset) const;

};


}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsKeyIntervals.cc"
#endif


#endif // swcdb_db_cells_SpecsKeyIntervals_h
