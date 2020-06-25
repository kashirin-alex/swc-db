
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_protocol_mngr_params_ColumnList_h
#define swc_db_protocol_mngr_params_ColumnList_h


#include "swcdb/core/Serializable.h"
#include "swcdb/db/Columns/Schema.h"
#include <vector>

namespace SWC { namespace Protocol { namespace Mngr { namespace Params {


class ColumnListReq  : public Serializable {
  public:

  ColumnListReq();

  virtual ~ColumnListReq();

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

#endif // swc_db_protocol_params_ColumnListRsp_h
