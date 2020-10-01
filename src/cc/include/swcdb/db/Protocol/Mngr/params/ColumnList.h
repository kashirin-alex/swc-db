
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_ColumnList_h
#define swcdb_db_protocol_mngr_params_ColumnList_h


#include "swcdb/core/Serializable.h"
#include "swcdb/db/Columns/Schemas.h"
#include <vector>

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class ColumnListReq  : public Serializable {
  public:

  ColumnListReq();

  virtual ~ColumnListReq();

  std::vector<DB::Schemas::Pattern> patterns;

  private:
    
  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class ColumnListRsp  : public Serializable {
  public:

  ColumnListRsp();

  virtual ~ColumnListRsp();

  std::vector<DB::Schema::Ptr> schemas;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/ColumnList.cc"
#endif 

#endif // swcdb_db_protocol_params_ColumnListRsp_h
