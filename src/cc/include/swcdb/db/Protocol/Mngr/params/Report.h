/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_mngr_params_Report_h
#define swcdb_db_protocol_mngr_params_Report_h

#include "swcdb/core/comm/Serializable.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/db/Columns/Schema.h"
#include "swcdb/db/Types/MngrColumnState.h"
#include "swcdb/db/Types/MngrRangeState.h"
#include "swcdb/db/Types/MngrRangerState.h"
#include "swcdb/db/Types/MngrState.h"
#include "swcdb/db/Types/MngrRole.h"



namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Params {

namespace Report {


enum Function : uint8_t {
  CLUSTER_STATUS  = 0x00,
  COLUMN_STATUS   = 0x01,
  RANGERS_STATUS  = 0x02,
  MANAGERS_STATUS = 0x03
};




class ReqColumnStatus final : public Serializable {
  public:

  SWC_CAN_INLINE
  ReqColumnStatus(cid_t a_cid = DB::Schema::NO_CID)
                  noexcept : cid(a_cid) {
  }

  //~ReqColumnStatus() { }

  cid_t cid;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


class RspColumnStatus final : public Serializable {
  public:

  SWC_CAN_INLINE
  RspColumnStatus() noexcept
                  : state(DB::Types::MngrColumn::State::NOTSET),
                    ranges() {
  }

  SWC_CAN_INLINE
  ~RspColumnStatus() noexcept { }

  struct RangeStatus {

    DB::Types::MngrRange::State state;
    rid_t                       rid;
    rgrid_t                     rgr_id;

    SWC_CAN_INLINE
    RangeStatus() noexcept : state(), rid(), rgr_id() { }

    SWC_CAN_INLINE
    RangeStatus(const uint8_t** bufp, size_t* remainp)
                : state(), rid(), rgr_id() {
      decode(bufp, remainp);
    }

    size_t SWC_PURE_FUNC encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };

  void display(std::ostream& out, const std::string& offset = "") const;

  DB::Types::MngrColumn::State  state;
  Core::Vector<RangeStatus>     ranges;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RspRangersStatus final : public Serializable {
  public:

  SWC_CAN_INLINE
  RspRangersStatus() noexcept : rangers() { }

  SWC_CAN_INLINE
  ~RspRangersStatus() noexcept { }

  struct Ranger final {

    EndPoints   endpoints;
    uint8_t     state;
    rgrid_t     rgr_id;
    int32_t     failures;
    uint64_t    interm_ranges;
    uint16_t    load_scale;
    uint8_t     rebalance;

    SWC_CAN_INLINE
    Ranger() noexcept
          : endpoints(), state(), rgr_id(),
            failures(), interm_ranges(), load_scale(), rebalance() {
    }

    SWC_CAN_INLINE
    Ranger(const uint8_t** bufp, size_t* remainp)
          : endpoints(), state(), rgr_id(),
            failures(), interm_ranges(), load_scale(), rebalance() {
      decode(bufp, remainp);
    }

    SWC_CAN_INLINE
    ~Ranger() noexcept { }

    size_t SWC_PURE_FUNC encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };

  void display(std::ostream& out, const std::string& offset = "") const;

  Core::Vector<Ranger> rangers;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RspManagersStatus final : public Serializable {
  public:

  SWC_CAN_INLINE
  RspManagersStatus() noexcept: managers(), inchain() { }

  SWC_CAN_INLINE
  ~RspManagersStatus() noexcept { }

  struct Manager final {

    EndPoints             endpoints;
    uint32_t              priority;
    DB::Types::MngrState  state;
    uint8_t               role;
    cid_t                 cid_begin;
    cid_t                 cid_end;
    int                   failures;

    SWC_CAN_INLINE
    Manager() noexcept
            : endpoints(),
              priority(), state(), role(),
              cid_begin(), cid_end(), failures() {
    }

    SWC_CAN_INLINE
    Manager(const uint8_t** bufp, size_t* remainp)
            : endpoints(),
              priority(), state(), role(),
              cid_begin(), cid_end(), failures() {
      decode(bufp, remainp);
    }

    SWC_CAN_INLINE
    ~Manager() noexcept { }

    size_t SWC_PURE_FUNC encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, const std::string& offset) const;

  };


  void display(std::ostream& out, const std::string& offset = "") const;

  Core::Vector<Manager> managers;
  EndPoint              inchain;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}
}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Mngr/params/Report.cc"
#endif

#endif // swcdb_db_protocol_mngr_params_Report_h
