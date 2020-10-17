
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_Report_h
#define swcdb_db_protocol_mngr_params_Report_h

#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Columns/Schema.h"
#include "swcdb/db/Types/MngrColumnState.h"
#include "swcdb/db/Types/MngrRangeState.h"
#include "swcdb/db/Types/MngrRangerState.h"
#include "swcdb/db/Types/MngrState.h"
#include "swcdb/db/Types/MngrRole.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"



namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {

namespace Report {


enum Function : uint8_t {
  CLUSTER_STATUS  = 0x00,
  COLUMN_STATUS   = 0x01,
  RANGERS_STATUS  = 0x02,
  MANAGERS_STATUS = 0x03
};




class ReqColumnStatus : public Serializable {
  public:

  ReqColumnStatus(cid_t cid = DB::Schema::NO_CID);

  virtual ~ReqColumnStatus();

  cid_t cid;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};


class RspColumnStatus : public Serializable {
  public:

  RspColumnStatus();

  virtual ~RspColumnStatus();

  struct RangeStatus {

    DB::Types::MngrRange::State state;
    rid_t                   rid;
    rgrid_t                 rgr_id;
    
    size_t encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };

  void display(std::ostream& out, const std::string& offset = "") const;

  DB::Types::MngrColumn::State  state;
  std::vector<RangeStatus>      ranges;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RspRangersStatus : public Serializable {
  public:

  RspRangersStatus();

  virtual ~RspRangersStatus();

  struct Ranger final : public Common::Params::HostEndPoints {

    uint8_t     state;
    rgrid_t     rgr_id;
    int32_t     failures;
    uint64_t    interm_ranges;
    uint16_t    load_scale;
    uint8_t     rebalance;

    size_t encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };

  void display(std::ostream& out, const std::string& offset = "") const;

  std::vector<Ranger> rangers;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RspManagersStatus : public Serializable {
  public:

  RspManagersStatus();

  virtual ~RspManagersStatus();

  struct Manager final : public Common::Params::HostEndPoints {

    uint32_t            priority;
    DB::Types::MngrState    state;
    uint8_t             role;
    cid_t               cid_begin;
    cid_t               cid_end;
    int                 failures;

    size_t encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };


  void display(std::ostream& out, const std::string& offset = "") const;

  std::vector<Manager>  managers;
  EndPoint              inchain;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};


}
}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/Report.cc"
#endif 

#endif // swcdb_db_protocol_mngr_params_Report_h
