
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_ColumnMng_h
#define swcdb_db_protocol_mngr_params_ColumnMng_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {
  

class ColumnMng : public Serializable {
  public:

  enum Function : uint8_t { // corelation-sequence required
    INTERNAL_LOAD_ALL     = 0,

    INTERNAL_LOAD         = 1,
    INTERNAL_ACK_LOAD     = 2,

    CREATE                = 3,
    INTERNAL_ACK_CREATE   = 4,

    DELETE                = 5,
    INTERNAL_ACK_DELETE   = 6,

    MODIFY                = 7,
    INTERNAL_ACK_MODIFY   = 8,

    INTERNAL_EXPECT       = 9,
  };

  ColumnMng();

  ColumnMng(Function function, const DB::Schema::Ptr& schema);

  virtual ~ColumnMng();

  Function        function;
  DB::Schema::Ptr schema;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

  };


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/ColumnMng.cc"
#endif 

#endif // swcdb_db_protocol_mngr_params_ColumnMng_h
