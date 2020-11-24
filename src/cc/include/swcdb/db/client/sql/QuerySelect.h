/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_sql_QuerySelect_h
#define swcdb_db_client_sql_QuerySelect_h


#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/db/Cells/SpecsScan.h"

namespace SWC { namespace client { namespace SQL {

class QuerySelect final : public Reader {

  public:
  QuerySelect(const std::string& sql, DB::Specs::Scan& specs, 
              std::string& message);

  int parse_select();

  int parse_dump(std::string& filepath);

  void parse_output_flags(uint8_t& output_flags);

  void parse_display_flags(uint8_t& display_flags);

  ~QuerySelect();

  private:

  void read_columns_intervals();

  cid_t add_column(const std::string& col);

  void add_column(const std::vector<DB::Schemas::Pattern>& patterns, 
                  std::vector<cid_t>& cols);

  void read_cells_intervals(const std::vector<cid_t>& cols);
  
  void read_cells_interval(DB::Specs::Interval& spec);

  void read_range(DB::Cell::Key& begin, DB::Cell::Key& end, bool flw);

  void read_key(DB::Specs::Key& start, DB::Specs::Key& finish, bool flw, 
                uint8_t& options);

  void read_key(DB::Specs::Key& key);

  void read_value(DB::Specs::Value& value);
  
  void read_timestamp(DB::Specs::Timestamp& start, 
                      DB::Specs::Timestamp& finish, bool flw);

  void read_flags(DB::Specs::Flags& flags);

  DB::Specs::Scan&  specs;
};





/*
# select all cells(INSERT) from column ID or NAME
select where 
  col(NAME|ID,"NAME|ID",) = (
    cells=()
  )
;

# select cells on-interval  from columns 1 and 2
select where 
  col(1,2) = (
    cells=(
        ["a", "start", "of", "range", "fractions"]
      <= range <=
        ["an", "end", "of", "range", "fractions"]
      AND
      key = [="where",="fractions", ="equal", ="to"]
      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      ONLY_KEYS
      ONLY_DELETES
      limit=1
      offset=0
    )
  )
;

# select cells on-interval-1  from columns 1 and 2 AND cells on-interval-2  from column "col-test-1"
select where 
  col(1,2) = (
    cells=(
        ["a", "start", "of", "range", "fractions"]
      <= range <=
        ["an", "end", "of", "range", "fractions"]
      AND
      key = ["where","fractions", "equal", "to"]
      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      ONLY_KEYS
      ONLY_DELETES
      limit=1
      offset=0
    )
  )

  AND
  
  col("col-test-1") = (
    cells=(
        ["a", "start", "of", "range", "fractions"]
      <= range <=
        ["an", "end", "of", "range", "fractions"]
      AND
        [>="where",>="fractions", =^"start_with", >="this", >="", >="fractions", !="NOT_THIS", <="within"]
      <= key <=
        [<="where under and more", <="fractions incl.", =^"start_with", >="this", r"is it a \\re", >="fractions and above", <="UP TO 1000", <="within"]

      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      ONLY_KEYS
      ONLY_DELETES
      limit=1
      offset=0
    )
    
    AND 
    
    cells=(
        ["2 a", "start", "of", "range", "fractions"]
      <= range <=
        ["2 an", "end", "of", "range", "fractions"]
      AND
        [>="where",>="fractions", =^"start_with", >="this", >="", >="fractions", !="NOT_THIS", <="within"]
      <= key <=
        [<="where under and more", <="fractions incl.", =^"start_with", >="this", r"is it a \\re", >="fractions and above", <="UP TO 1000", <="within"]

      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      ONLY_KEYS
      ONLY_DELETES
      limit=1
      offset=0
    )
    
    AND 
    
    cells=(
        ["3 a", "start", "of", "range", "fractions"]
      <= range <=
        ["3 an", "end", "of", "range", "fractions"]
      AND
        [>="where",>="fractions", =^"start_with", >="this", >="", >="fractions", !="NOT_THIS", <="within"]
      <= key <=
        [<="where under and more", <="fractions incl.", =^"start_with", >="this", r"is it a \\re", >="fractions and above", <="UP TO 1000", <="within"]

      AND
        "2019/12/28 06:51:27.857347289"
      <= timestamp <=
        "1577508687857347289"
      AND 
      OFFSET_KEY=["an", "offset", "key", "fractions", "to", "start", "from", "the", "scan"]
      AND 
      OFFSET_REV="1577508700000000000"
      AND 
      value re "\"aRegExp\""
      
      ONLY_KEYS
      ONLY_DELETES
      limit=1
      offset=0
    )
  )
  DISPLAY_TIMESTAMP DISPLAY_DATETIME DISPLAY_SPECS DISPLAY_STATS DISPLAY_BINARY
;


*/


}}} // SWC::client:SQL namespace


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/sql/QuerySelect.cc"
#endif 


#endif //swcdb_db_client_sql_QuerySelect_h
