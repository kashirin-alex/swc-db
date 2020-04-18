/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_cells_CellKey_h
#define swcdb_db_cells_CellKey_h

#include "swcdb/core/Compat.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Cells/CellKeyVec.h"
#include <memory>


namespace SWC { namespace DB { namespace Cell {

class Key final {
  public:

  typedef std::shared_ptr<Key> Ptr;

  explicit Key(bool own = true);

  explicit Key(const Key& other);

  void copy(const Key& other);

  ~Key();

  void free();

  bool sane() const;
  
  void add(const std::string_view& fraction);

  void add(const std::string& fraction);

  void add(const char* fraction);

  void add(const char* fraction, uint32_t len);

  void add(const uint8_t* fraction, uint32_t len);

  void insert(uint32_t idx, const std::string& fraction);

  void insert(uint32_t idx, const char* fraction);

  void insert(uint32_t idx, const char* fraction, uint32_t len);

  void insert(uint32_t idx, const uint8_t* fraction, uint32_t len);
  
  void remove(uint32_t idx, bool recursive=false);

  std::string get_string(uint32_t idx) const;

  void get(uint32_t idx, const char** fraction, uint32_t* length) const;

  bool equal(const Key& other) const;

  Condition::Comp compare(const Key& other, uint32_t max=0, 
                          bool empty_ok=false, bool empty_eq=false) const;

  //bool align(KeyVec& start, KeyVec& finish) const;

  bool compare(const KeyVec& other, Condition::Comp break_if,
               uint32_t max = 0, bool empty_ok=false) const;

  bool empty() const;
  
  uint32_t encoded_length() const;

  void encode(uint8_t **bufp) const;

  void decode(const uint8_t **bufp, size_t* remainp, bool owner=false);

  void convert_to(std::vector<std::string>& key) const;

  void read(const std::vector<std::string>& key);

  bool equal(const std::vector<std::string>& key) const;

  std::string to_string() const;
  
  void display(std::ostream& out, bool pretty=false) const;

  void display_details(std::ostream& out, bool pretty=false) const;

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