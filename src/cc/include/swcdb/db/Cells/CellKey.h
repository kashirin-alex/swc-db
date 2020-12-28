/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_cells_CellKey_h
#define swcdb_db_cells_CellKey_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/CellKeyVec.h"
#include <memory>


namespace SWC { namespace DB {


//! The SWC-DB Cell C++ namespace 'SWC::DB::Cell'
namespace Cell {


class Key final {
  public:

  typedef std::shared_ptr<Key> Ptr;

  explicit Key(bool own = true);

  explicit Key(const Key& other);

  explicit Key(const Key& other, bool own);

  Key(const Key&&) = delete;

  Key& operator=(const Key&) = delete;

  void copy(const Key& other);

  ~Key();

  void _free();

  void free();

  bool sane() const;

  void add(const std::string_view& fraction);

  void add(const std::string& fraction);

  void add(const char* fraction);

  void add(const char* fraction, uint32_t len);

  void add(const uint8_t* fraction, uint32_t len);

  void add(const std::vector<std::string>::const_iterator cbegin,
           const std::vector<std::string>::const_iterator cend);

  void add(const std::vector<std::string>& fractions);

  void insert(uint32_t idx, const std::string& fraction);

  void insert(uint32_t idx, const char* fraction);

  void insert(uint32_t idx, const char* fraction, uint32_t len);

  void insert(uint32_t idx, const uint8_t* fraction, uint32_t len);

  void remove(uint32_t idx, bool recursive=false);

  std::string get_string(uint32_t idx) const;

  void get(uint32_t idx, const char** fraction, uint32_t* length) const;

  bool equal(const Key& other) const;

  bool empty() const;

  uint32_t encoded_length() const;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp, bool owner);

  void convert_to(std::vector<std::string>& key) const;

  void convert_to(std::vector<KeyVec::Fraction>& key) const;

  void read(const std::vector<std::string>& key);

  bool equal(const std::vector<std::string>& key) const;

  std::string to_string() const;

  void display(std::ostream& out, bool pretty=true,
               const char* sep = ",") const;

  void display_details(std::ostream& out, bool pretty=true) const;

  void print(std::ostream& out) const;

  friend std::ostream& operator<<(std::ostream& out, const Key& key) {
    key.print(out);
    return out;
  }

  bool      own;
  uint24_t  count;
  uint32_t  size;
  uint8_t*  data;

  private:

  uint8_t* _data(const uint8_t* ptr);

};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/CellKey.cc"
#endif

#endif // swcdb_db_Cells_CellKey_h
