/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_SpecsKey_h
#define swcdb_db_cells_SpecsKey_h


#include "swcdb/db/Cells/KeyComparator.h"


namespace SWC { namespace DB { namespace Specs {


struct Fraction final : public std::string {

  Condition::Comp comp;
  mutable void*   compiled = nullptr;

  Fraction() { }

  Fraction(const char* buf, uint32_t len, Condition::Comp comp);

  Fraction(std::string&& fraction, Condition::Comp comp) noexcept;

  Fraction(const Fraction& other);

  Fraction(Fraction&& other) noexcept;

  Fraction& operator=(const Fraction& other);

  Fraction& operator=(Fraction&& other) noexcept;

  Fraction& operator=(std::string&& other) noexcept;

  ~Fraction();

  bool operator==(const Fraction &other) const;

  uint32_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);

  void print(std::ostream& out, bool pretty) const;

  template<Types::KeySeq T_seq> // internal use
  bool _is_matching(const uint8_t* ptr, uint32_t len) const;

};


class Key final : public std::vector<Fraction> {
  public:

  using std::vector<Fraction>::insert;

  typedef std::shared_ptr<Key> Ptr;

  explicit Key() noexcept;

  explicit Key(const Key& other);

  explicit Key(Key&& other) noexcept;

  explicit Key(const DB::Cell::Key &cell_key, Condition::Comp comp);

  ~Key();

  size_t size_of_internal() const noexcept;

  void copy(const Key &other);

  Key& operator=(Key&& other) noexcept;

  void move(Key& other) noexcept;

  bool equal(const Key &other) const noexcept;


  void set(const DB::Cell::Key &cell_key, Condition::Comp comp);

  void set(int32_t idx, Condition::Comp comp);


  Fraction& add(Fraction&& other);

  Fraction& add(std::string&& fraction, Condition::Comp comp);

  Fraction& add(const char* buf, uint32_t len, Condition::Comp comp);

  Fraction& add(const std::string& fraction, Condition::Comp comp);

  Fraction& add(const std::string_view& fraction, Condition::Comp comp);

  Fraction& add(const char* fraction, Condition::Comp comp);

  Fraction& add(const uint8_t* fraction, uint32_t len, Condition::Comp comp);


  Fraction& insert(uint32_t idx, Fraction&& other);

  Fraction& insert(uint32_t idx, std::string&& fraction,
                   Condition::Comp comp);

  Fraction& insert(uint32_t idx, const char* buf, uint32_t len,
                   Condition::Comp comp);

  Fraction& insert(uint32_t idx, const std::string& fraction,
                   Condition::Comp comp);

  Fraction& insert(uint32_t idx, const std::string_view& fraction,
                   Condition::Comp comp);

  Fraction& insert(uint32_t idx, const uint8_t* fraction, uint32_t len,
                   Condition::Comp comp);

  Fraction& insert(uint32_t idx, const char* fraction, Condition::Comp comp);


  std::string_view get(const uint32_t idx, Condition::Comp& comp) const;

  std::string_view get(const uint32_t idx) const;

  void get(DB::Cell::Key& key) const;


  void remove(uint32_t idx, bool recursive=false);


  uint32_t encoded_length() const noexcept;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);


  bool is_matching(const Types::KeySeq seq, const Cell::Key &key) const;

  bool is_matching_lexic(const Cell::Key &key) const;

  bool is_matching_volume(const Cell::Key &key) const;


  std::string to_string() const;

  void print(std::ostream& out) const;

  void display(std::ostream& out, bool pretty=true) const;

  private:

  template<Types::KeySeq T_seq> // internal use
  bool _is_matching(const Cell::Key &key) const;

};


}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/SpecsKey.cc"
#endif

#endif // swcdb_db_cells_SpecsKey_h
