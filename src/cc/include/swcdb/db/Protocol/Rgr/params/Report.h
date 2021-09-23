/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_rgr_params_Report_h
#define swcdb_db_protocol_rgr_params_Report_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Params {

namespace Report {


enum Function : uint8_t {
  RESOURCES        = 0x00,
  CIDS             = 0x01,
  COLUMN_RIDS      = 0x02,
  COLUMN_RANGES    = 0x03,
  COLUMNS_RANGES   = 0x04,
};



class ReqColumn final : public Serializable {
  public:

  SWC_CAN_INLINE
  ReqColumn(cid_t a_cid = 0) noexcept : cid(a_cid) { }

  //~ReqColumn() { }

  cid_t   cid;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RspRes final : public Serializable {
  public:

  SWC_CAN_INLINE
  RspRes() noexcept { }

  SWC_CAN_INLINE
  RspRes(uint32_t a_mem, uint32_t a_cpu, size_t a_ranges) noexcept
        : mem(a_mem), cpu(a_cpu), ranges(a_ranges) {
  }

  //~RspRes() { }

  uint32_t    mem;
  uint32_t    cpu;
  size_t      ranges;

  void display(std::ostream& out, const std::string& offset="") const;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RspCids final : public Serializable {
  public:

  SWC_CAN_INLINE
  RspCids() noexcept { }

  SWC_CAN_INLINE
  ~RspCids() noexcept { }

  mutable cids_t cids;

  void display(std::ostream& out, const std::string& offset = "") const;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RspColumnRids final : public Serializable {
  public:

  SWC_CAN_INLINE
  RspColumnRids() noexcept { }

  SWC_CAN_INLINE
  ~RspColumnRids() noexcept { }

  mutable rids_t rids;

  void display(std::ostream& out, const std::string& offset = "") const;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};



class RspColumnsRanges final : public Serializable {
  public:

  struct Range {

    SWC_CAN_INLINE
    Range(DB::Types::KeySeq seq) noexcept : interval(seq) { }

    static bool before(Range* r1, Range* r2);

    rid_t               rid;
    DB::Cells::Interval interval;

    ~Range() noexcept { }

    size_t SWC_PURE_FUNC encoded_length () const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, bool pretty=true,
                 const std::string& offset = "") const;
  };

  struct Column {

    static bool SWC_PURE_FUNC before(Column* c1, Column* c2);

    cid_t                cid;
    DB::Types::KeySeq    col_seq;
    uint64_t             mem_bytes;
    Core::Vector<Range*> ranges;

    ~Column() noexcept;

    size_t SWC_PURE_FUNC encoded_length() const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);

    void display(std::ostream& out, bool pretty=true,
                 const std::string& offset = "") const;
  };


  SWC_CAN_INLINE
  explicit RspColumnsRanges() noexcept : rgrid(0) { }

  RspColumnsRanges(rgrid_t rgrid, const EndPoints& endpoints);

  RspColumnsRanges& operator=(const RspColumnsRanges& other) = delete;

  ~RspColumnsRanges() noexcept;

  rgrid_t               rgrid;
  EndPoints             endpoints;
  Core::Vector<Column*> columns;

  void display(std::ostream& out, bool pretty=true,
               const std::string& offset = "") const;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}
}}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/Report.cc"
#endif

#endif // swcdb_db_protocol_rgr_params_Report_h
