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
  typedef std::unique_ptr<KeyInterval> Ptr;
  Key  start, finish;
    
  static Ptr make_ptr(const Key& start, const Key& finish) {
    return std::make_unique<KeyInterval>(start, finish);
  }

  KeyInterval() { }
    
  KeyInterval(const KeyInterval& other);

  KeyInterval(const Key& start, const Key& finish);
  
  KeyInterval(const uint8_t** bufp, size_t* remainp);

  KeyInterval(KeyInterval&& other) = delete;

};



class KeyIntervals : private std::vector<KeyInterval::Ptr> {
  public:
  
  typedef std::vector<KeyInterval::Ptr> Vec;
  
  using Vec::empty;
  using Vec::size;
  using Vec::begin;
  using Vec::end;
  using Vec::front;
  using Vec::back;
  using Vec::operator[];

  KeyIntervals() { }

  KeyIntervals(const KeyIntervals& other) = delete;

  KeyIntervals(KeyIntervals&& other) = delete;

  void copy(const KeyIntervals& other);

  void free();

  KeyInterval::Ptr& add();

  KeyInterval::Ptr& add(const KeyInterval& other);

  KeyInterval::Ptr& add(const Key& start, const Key& finish);

  size_t size_of_internal() const;

  bool equal(const KeyIntervals& other) const;
  
  bool is_matching(const Types::KeySeq key_seq, 
                   const DB::Cell::Key& cellkey) const;

  bool is_matching_start(const Types::KeySeq key_seq, 
                         const DB::Cell::Key& cellkey) const;
  
  size_t encoded_length() const;

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
