
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_RangeQuerySelect_h
#define swcdb_db_protocol_rgr_params_RangeQuerySelect_h

#include "swcdb/core/Buffer.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Cells/SpecsInterval.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {


class RangeQuerySelectReq : public Serializable {
  public:

  RangeQuerySelectReq();

  RangeQuerySelectReq(cid_t cid, rid_t rid, 
                      const DB::Specs::Interval& interval);

  virtual ~RangeQuerySelectReq();

  void print(std::ostream& out) const;

  cid_t                cid;
  rid_t                rid;
  DB::Specs::Interval  interval;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RangeQuerySelectRsp  : public Serializable {
  public:

  RangeQuerySelectRsp(int err = Error::OK, bool reached_limit=false, 
                      uint64_t offset=0);
  
  RangeQuerySelectRsp(int err, StaticBuffer& data);

  virtual ~RangeQuerySelectRsp();

  void print(std::ostream& out) const;

  int32_t         err;
  bool            reached_limit;
  uint64_t        offset;
  StaticBuffer    data;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};
  

}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.cc"
#endif 

#endif // swcdb_db_protocol_rgr_params_RangeQuerySelect_h
