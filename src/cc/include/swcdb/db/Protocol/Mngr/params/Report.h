
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_mngr_params_Report_h
#define swc_db_protocol_mngr_params_Report_h

#include "swcdb/core/Serializable.h"
#include "swcdb/db/Columns/Schema.h"
#include "swcdb/db/Types/MngrColumnState.h"
#include "swcdb/db/Types/MngrRangeState.h"
#include "swcdb/db/Types/MngrRangerState.h"
#include "swcdb/db/Protocol/Common/params/HostEndPoints.h"



namespace SWC { namespace Protocol { namespace Mngr { namespace Params { namespace Report {


enum Function {
  CLUSTER_STATUS  = 0x00,
  COLUMN_STATUS   = 0x01,
  RANGERS_STATUS  = 0x02
};


class RspClusterStatus : public Serializable {
  public:

  RspClusterStatus(uint8_t status);

  virtual ~RspClusterStatus();

  std::string to_string() const;

  uint8_t status;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

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

    Types::MngrRange::State state;
    rid_t                   rid;
    rgrid_t                 rgr_id;
    
    size_t encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };

  void display(std::ostream& out, const std::string& offset = "") const;

  Types::MngrColumn::State state;
  std::vector<RangeStatus> ranges;

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

    Types::MngrRanger::State state;
    rgrid_t                  rgr_id;
    int32_t                  failures;
    uint64_t                 interm_ranges;
    uint16_t                 load_scale;

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



}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/Report.cc"
#endif 

#endif // swc_db_protocol_mngr_params_Report_h
