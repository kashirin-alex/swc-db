
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_db_protocol_rgr_params_Report_h
#define swc_db_protocol_rgr_params_Report_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {
namespace Report {


enum Function {
  RESOURCES        = 0x00,
  CIDS             = 0x01,
  COLUMN_RIDS      = 0x02,
  COLUMN_RANGES    = 0x03,
  COLUMNS_RANGES   = 0x04,
};



class ReqColumn : public Serializable {
  public:

  ReqColumn(cid_t cid = 0);

  virtual ~ReqColumn();

  cid_t   cid;

  private:

  size_t internal_encoded_length() const;
    
  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RspRes  : public Serializable {
  public:
  
  RspRes();

  virtual ~RspRes();

  uint32_t    mem;
  uint32_t    cpu;
  size_t      ranges;

  void display(std::ostream& out, const std::string& offset="") const;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RspCids  : public Serializable {
  public:

  RspCids();

  virtual ~RspCids();

  mutable std::vector<cid_t> cids;

  void display(std::ostream& out, const std::string& offset = "") const;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RspColumnRids  : public Serializable {
  public:

  RspColumnRids();

  virtual ~RspColumnRids();

  mutable std::vector<rid_t> rids;

  void display(std::ostream& out, const std::string& offset = "") const;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};



class RspColumnsRanges  : public Serializable {
  public:
  
  struct Range {

    Range(Types::KeySeq seq);

    static bool before(Range* r1, Range* r2);

    rid_t               rid;
    DB::Cells::Interval interval;

    ~Range();

    size_t encoded_length () const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);
  
    void display(std::ostream& out, bool pretty=true, 
                 const std::string& offset = "") const;
  };

  struct Column {

    static bool before(Column* c1, Column* c2);

    cid_t                cid;
    Types::KeySeq        col_seq;
    uint64_t             mem_bytes;
    std::vector<Range*>  ranges;

    ~Column();

    size_t encoded_length () const;

    void encode(uint8_t** bufp) const;

    void decode(const uint8_t** bufp, size_t* remainp);
    
    void display(std::ostream& out, bool pretty=true, 
                 const std::string& offset = "") const;
  };


  explicit RspColumnsRanges();

  RspColumnsRanges(rgrid_t rgrid, const EndPoints& endpoints);

  RspColumnsRanges& operator=(const RspColumnsRanges& other) = delete;

  virtual ~RspColumnsRanges();

  rgrid_t              rgrid; 
  EndPoints            endpoints;
  std::vector<Column*> columns;

  void display(std::ostream& out, bool pretty=true, 
               const std::string& offset = "") const;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;
    
  void internal_decode(const uint8_t** bufp, size_t* remainp);

};


}
}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/Report.cc"
#endif 

#endif // swc_db_protocol_rgr_params_Report_h
