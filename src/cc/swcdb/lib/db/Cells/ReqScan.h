/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_ReqScan_h
#define swcdb_db_Cells_ReqScan_h


#include "Mutable.h"


namespace SWC { namespace DB { namespace Cells {
  
class ReqScan  {
  public:
  typedef std::shared_ptr<ReqScan>  Ptr;
  typedef std::function<void(int)>  Cb_t;

  inline static Ptr make(){
    return std::make_shared<ReqScan>();
  }

  inline static Ptr make(Specs::Interval::Ptr spec, Mutable::Ptr cells, Cb_t cb){
    return std::make_shared<ReqScan>(spec, cells, cb);
  }

  ReqScan() {}

  ReqScan(Specs::Interval::Ptr spec, Mutable::Ptr cells, Cb_t cb)
          : spec(spec), cells(cells), cb(cb) {}

  void response(int err) {
    cb(err);
  }

  virtual ~ReqScan() {}
  
  const std::string to_string() {
    std::string s("ReqScan(");
    s.append(spec->to_string());
    s.append(" ");
    s.append(cells->to_string());
    return s;
  }

  Cb_t                    cb;
  Specs::Interval::Ptr    spec;
  Mutable::Ptr            cells;
};

}}}

#endif