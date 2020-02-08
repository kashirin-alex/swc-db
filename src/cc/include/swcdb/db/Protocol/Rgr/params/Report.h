
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_db_protocol_rgr_params_Report_h
#define swc_db_protocol_rgr_params_Report_h

#include "swcdb/core/Serializable.h"

#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace Protocol { namespace Rgr { namespace Params {

class ReportReq : public Serializable {
  public:
  static const uint8_t RANGES     = 0x1;
  static const uint8_t RESOURCES  = 0x4;

  ReportReq(uint8_t flags=0x1): flags(flags) {}

  virtual ~ReportReq(){ }

  uint8_t flags;

  private:

  uint8_t encoding_version() const  {
    return 1; 
  }

  size_t encoded_length_internal() const {
    return 1;
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_i8(bufp, flags);
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    flags = Serialization::decode_i8(bufp, remainp);
  }

};



class ReportRsp  : public Serializable {
  public:
  
  struct Range {

    static bool before(Range* r1, Range* r2) {
      return r2->interval.is_in_end(r1->interval.key_end);
    }

    int64_t             rid;
    DB::Cells::Interval interval;

    const size_t encoded_length () const {
      return Serialization::encoded_length_vi64(rid)
          + interval.encoded_length();
    } 

    void encode(uint8_t **bufp) const {
      Serialization::encode_vi64(bufp, rid);
      interval.encode(bufp);
    }

    void decode(const uint8_t **bufp, size_t *remainp) {
      rid = Serialization::decode_vi64(bufp, remainp);
      interval.decode(bufp, remainp);
    }
    


    void display(std::ostream& out, bool pretty=false, 
                 std::string offset = "") const {
      out << offset << "-------------------------------------" << std::endl;
      out << offset << "rid(" << rid << "):" << std::endl;
      out << offset << " begin";
      interval.key_begin.display(out, pretty);
      out << std::endl;
      out << offset << "   end";
      interval.key_end.display(out, pretty);
      out << std::endl;
      out << offset << " " << interval.ts_earliest.value 
          << "<=TS<=" << interval.ts_latest.value ;
      out << std::endl;
    }
  };

  struct Column {

    static bool before(Column* c1, Column* c2) {
      return c1->cid < c2->cid;
    }

    int64_t               cid;
    std::vector<Range*>  ranges;

    ~Column() {
      for(auto r : ranges)
        delete r;
      ranges.clear();
    }

    const size_t encoded_length () const {
      size_t sz = Serialization::encoded_length_vi64(cid)
              + Serialization::encoded_length_vi64(ranges.size());
      for(auto r : ranges)
        sz += r->encoded_length();
      return sz;
    }

    void encode(uint8_t **bufp) const {
      Serialization::encode_vi64(bufp, cid);
      Serialization::encode_vi64(bufp, ranges.size());
      for(auto r : ranges)
        r->encode(bufp);
    }

    void decode(const uint8_t **bufp, size_t *remainp) {
      cid = Serialization::decode_vi64(bufp, remainp);
      for(int64_t n = Serialization::decode_vi64(bufp, remainp);n--;) {
        auto r = new Range();
        r->decode(bufp, remainp);
        ranges.push_back(r);
      }
      std::sort(ranges.begin(), ranges.end(), Range::before); 
    }
    
    void display(std::ostream& out, bool pretty=false, 
                 std::string offset = "") const {  
      out << offset << "**************************************" << std::endl;
      out << offset << "cid(" << cid << ")" 
          << " ranges(" << ranges.size() << "):" << std::endl;
      for(auto& r : ranges)
        r->display(out, pretty, offset+" ");
    }
  };

  explicit ReportRsp(int err=Error::OK) : err(err), rgr_id(0) { }

  ReportRsp& operator=(const ReportRsp& other) = delete;

  virtual ~ReportRsp() {
    for(auto c : columns)
      delete c;
    columns.clear();
  }

  int                  err; 
  int64_t              rgr_id; 
  EndPoints            endpoints;
  std::vector<Column*> columns;

  void display(std::ostream& out, bool pretty=false, 
               std::string offset = "") const {
    out << offset << "Ranger: id("<< rgr_id << ")" 
        << " endpoints(";
    for(auto& endpoint : endpoints)
      out << endpoint << ", ";
    out << ")" << std::endl;

    out << offset << "Columns(" << columns.size() << "):" << std::endl;
    for(auto& c : columns)
      c->display(out, pretty, offset+" ");
  }

  private:

  uint8_t encoding_version() const {
    return 1;
  }
    
  size_t encoded_length_internal() const {
    size_t sz = Serialization::encoded_length_vi32(err);
    if(!err) {
      sz += Serialization::encoded_length_vi64(rgr_id);  

      sz += Serialization::encoded_length_vi32(endpoints.size()); 
      for(auto& endpoint : endpoints)
        sz += Serialization::encoded_length(endpoint);

      sz += Serialization::encoded_length_vi64(columns.size());      
      for(auto c : columns)
        sz += c->encoded_length();
    }
    return sz;
  }
    
  void encode_internal(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, err);
    if(!err) {
      Serialization::encode_vi64(bufp, rgr_id);

      Serialization::encode_vi32(bufp, endpoints.size());
      for(auto& endpoint : endpoints)
        Serialization::encode(endpoint, bufp);

      Serialization::encode_vi64(bufp, columns.size());
      for(auto c : columns)
        c->encode(bufp);
    }
  }
    
  void decode_internal(uint8_t version, const uint8_t **bufp, 
                       size_t *remainp) {
    if(!(err = Serialization::decode_vi32(bufp, remainp))) {
      rgr_id = Serialization::decode_vi64(bufp, remainp);
      
      size_t len = Serialization::decode_vi32(bufp, remainp);
      endpoints.clear();
      endpoints.resize(len);
      for(size_t i=0;i<len;i++)
        endpoints[i] = Serialization::decode(bufp, remainp);

      for(int64_t n = Serialization::decode_vi64(bufp, remainp);n--;) {
        auto c = new Column();
        c->decode(bufp, remainp);
        columns.push_back(c);
      }
      std::sort(columns.begin(), columns.end(), Column::before); 
    }
  }

};
  

}}}}

#endif // swc_db_protocol_rgr_params_Report_h
