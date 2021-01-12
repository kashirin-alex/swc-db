/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_sql_Reader_h
#define swcdb_db_client_sql_Reader_h

#include "swcdb/db/Cells/CellKey.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace client { namespace SQL {

namespace {

  static const char     TOKEN_AND[] = "and";
  static const uint8_t  LEN_AND = 3;
  static const char     TOKEN_BOOL_FALSE[] = "false";
  static const uint8_t  LEN_BOOL_FALSE = 5;
  static const char     TOKEN_BOOL_TRUE[] = "true";
  static const uint8_t  LEN_BOOL_TRUE = 4;

}

class Reader {
  public:

  Reader(const std::string& sql, std::string& message);

  ~Reader();

  bool is_char(const char* stop) const;

  bool found_char(const char c);

  bool found_space();

  bool found_quote_single(bool& quote);

  bool found_quote_double(bool& quote);

  bool found_token(const char* token, uint8_t token_len);

  bool found_comparator(Condition::Comp& comp, bool extended=false);

  void expect_eq();

  void expect_comma(bool& comma);

  void expect_comparator(Condition::Comp& comp, bool extended=false);

  void expect_digit();

  void expected_boolean(bool& value);

  void expect_token(const char* token, uint8_t token_len, bool& found);

  DB::Schema::Ptr get_schema(const std::string& col);

  std::vector<DB::Schema::Ptr>
  get_schema(const  std::vector<DB::Schemas::Pattern>& patterns);

  void read(std::string& buf, const char* stop = 0, bool keep_escape=false);

  void read_uint8_t(uint8_t& value, bool& was_set);

  void read_uint16_t(uint16_t& value, bool& was_set);

  void read_uint32_t(uint32_t& value, bool& was_set,
                     const char* stop=nullptr);

  void read_int64_t(int64_t& value, bool& was_set,
                    const char* stop=nullptr);

  void read_double_t(long double& value, bool& was_set,
                     const char* stop=nullptr);

  DB::Types::Encoder read_encoder();

  DB::Cell::Serial::Value::Type read_serial_value_type();

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

#endif //swcdb_db_client_sql_Reader_h
