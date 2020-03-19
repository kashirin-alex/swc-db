/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_cells_Cell_h
#define swcdb_db_cells_Cell_h

#include <memory>
#include "swcdb/core/Serialization.h"
#include "swcdb/core/DynamicBuffer.h"

#include "swcdb/db/Types/Column.h"
#include "swcdb/db/Cells/CellKey.h"


namespace SWC { namespace DB { 

enum DisplayFlag {
  TIMESTAMP     = 0x01,
  DATETIME      = 0x04,
  BINARY        = 0x08,
  SPECS         = 0x10,
  STATS         = 0x20
};

enum OutputFlag {
  NO_TS     = 0x01,
  NO_VALUE  = 0x04
};


namespace Cells {

enum Flag {
  NONE                      = 0x0, // empty instance
  INSERT                    = 0x1,
  DELETE                    = 0x2,
  DELETE_VERSION            = 0x3
};

const std::string to_string(Flag flag);

const Flag flag_from(const uint8_t* rptr, uint32_t len);


static const int64_t TIMESTAMP_MIN  = INT64_MIN;
static const int64_t TIMESTAMP_MAX  = INT64_MAX;
static const int64_t TIMESTAMP_NULL = INT64_MIN + 1;
static const int64_t TIMESTAMP_AUTO = INT64_MIN + 2;
static const int64_t AUTO_ASSIGN    = TIMESTAMP_AUTO;

static const uint8_t HAVE_REVISION      =  0x80;
static const uint8_t HAVE_TIMESTAMP     =  0x40;
static const uint8_t AUTO_TIMESTAMP     =  0x20;
static const uint8_t REV_IS_TS          =  0x10;
//static const uint8_t TYPE_DEFINED     =  0x2;
static const uint8_t TS_DESC            =  0x1;

static const uint8_t OP_EQUAL  = 0x1;


class Cell final {
  public:
  typedef std::shared_ptr<Cell> Ptr;

  explicit Cell();

  explicit Cell(const Cell& other);

  explicit Cell(const Cell& other, bool no_value);

  explicit Cell(const uint8_t** bufp, size_t* remainp, bool own=false);

  void copy(const Cell& other, bool no_value=false);

  ~Cell();

  void free();

  void set_time_order_desc(bool desc);

  void set_timestamp(int64_t ts);

  void set_value(uint8_t* v, uint32_t len, bool owner);

  void set_value(const char* v, uint32_t len, bool owner);

  void set_value(const char* v, bool owner=false);

  void set_value(const std::string& v, bool owner=false);

  void set_counter(uint8_t op, int64_t v,  
                  Types::Column typ = Types::Column::COUNTER_I64, 
                  int64_t rev = TIMESTAMP_NULL);

  const uint8_t get_counter_op() const;

  const int64_t get_counter() const;

  const int64_t get_counter(uint8_t& op, int64_t& rev) const;

  void read(const uint8_t **bufp, size_t* remainp, bool owner=false);

  const uint32_t encoded_length() const;

  void write(SWC::DynamicBuffer &dst_buf) const;

  const bool equal(const Cell& other) const;

  const bool removal() const;
  
  const bool is_removing(const int64_t& rev) const;

  const int64_t get_timestamp() const;

  const int64_t get_revision() const;

  const bool has_expired(const uint64_t ttl) const;

  const std::string to_string(Types::Column typ = Types::Column::PLAIN) const;

  void display(std::ostream& out, Types::Column typ = Types::Column::PLAIN, 
               uint8_t flags=0, bool meta=false) const;

  DB::Cell::Key   key;
  bool            own;
  uint8_t         flag;
  uint8_t         control;
  uint32_t        vlen;
  int64_t         timestamp;
  int64_t         revision;
  uint8_t*        value;

  private:

  uint8_t* _value(const uint8_t* v);

};



}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Cell.cc"
#endif 

#endif // swcdb_db_Cells_Cell_h