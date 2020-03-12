/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_client_sql_Reader_h
#define swcdb_client_sql_Reader_h

#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace client { namespace SQL {

namespace {
    

  static const char*    TOKEN_AND = "and";
  static const uint8_t  LEN_AND = 3;
  static const char*    TOKEN_BOOL_FALSE = "false";
  static const uint8_t  LEN_BOOL_FALSE = 5;
  static const char*    TOKEN_BOOL_TRUE = "true";
  static const uint8_t  LEN_BOOL_TRUE = 4;

}

class Reader {
  public:

  Reader(const std::string& sql, std::string& message);

  ~Reader();

  protected:

  const bool is_char(const char* stop) const;

  const bool found_char(const char c);

  const bool found_space();

  const bool found_quote_single(bool& quote);

  const bool found_quote_double(bool& quote);

  const bool found_token(const char* token, uint8_t token_len);

  const bool found_comparator(Condition::Comp& comp);

  void expect_eq();

  void expect_comma(bool& comma);
  
  void expect_comparator(Condition::Comp& comp);

  void expect_digit();

  void expected_boolean(bool& value);

  void expect_token(const char* token, uint8_t token_len, bool& found);

  DB::Schema::Ptr get_schema(const std::string& col);

  void read(std::string& buf, const char* stop = 0, bool keep_escape=false);

  void read_uint8_t(uint8_t& value, bool& was_set);

  void read_uint32_t(uint32_t& value, bool& was_set);

  void read_int64_t(int64_t& value, bool& was_set);

  void read_key(DB::Cell::Key& key);

  void error_msg(int error, const std::string& msg);
  
  const std::string&  sql;
  std::string&        message;
  const char*         ptr;
  uint32_t            remain;
  int                 err;

};


}}} // SWC::client:SQL namespace



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/Reader.cc"
#endif 

#endif //swcdb_client_sql_Reader_h