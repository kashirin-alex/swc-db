
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_Report_h
#define swc_db_protocol_rgr_params_Report_h


#include "swcdb/core/comm/ClientConnQueue.h"
#include "swcdb/core/Serializable.h"
#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class ReportReq : public Serializable {
  public:
  static const uint8_t RANGES     = 0x1;
  static const uint8_t RESOURCES  = 0x4;

  ReportReq(uint8_t flags=0x1);

  virtual ~ReportReq();

  uint8_t flags;

  private:

  uint8_t encoding_version() const;

  size_t encoded_length_internal() const;
    
  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};



class ReportRsp  : public Serializable {
  public:
  
  struct Range {

    static bool before(Range* r1, Range* r2);

    int64_t             rid;
    DB::Cells::Interval interval;

    ~Range();

    const size_t encoded_length () const;

    void encode(uint8_t **bufp) const;

    void decode(const uint8_t **bufp, size_t *remainp);
  
    void display(std::ostream& out, bool pretty=false, 
                 std::string offset = "") const;
  };

  struct Column {

    static bool before(Column* c1, Column* c2);

    int64_t               cid;
    std::vector<Range*>  ranges;

    ~Column();

    const size_t encoded_length () const;

    void encode(uint8_t **bufp) const;

    void decode(const uint8_t **bufp, size_t *remainp);
    
    void display(std::ostream& out, bool pretty=false, 
                 std::string offset = "") const;
  };


  explicit ReportRsp(int err=Error::OK);

  ReportRsp& operator=(const ReportRsp& other) = delete;

  virtual ~ReportRsp();

  int                  err; 
  int64_t              rgr_id; 
  EndPoints            endpoints;
  std::vector<Column*> columns;

  void display(std::ostream& out, bool pretty=false, 
               std::string offset = "") const;

  private:

  uint8_t encoding_version() const;
    
  size_t encoded_length_internal() const;

  void encode_internal(uint8_t **bufp) const;
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp);

};
  

}}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Rgr/params/Report.cc"
#endif 

#endif // swc_db_protocol_rgr_params_Report_h
