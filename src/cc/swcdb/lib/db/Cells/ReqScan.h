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

  inline static Ptr 
  make(){
    return std::make_shared<ReqScan>();
  }

  inline static Ptr 
  make(Specs::Interval::Ptr spec, Mutable::Ptr cells, Cb_t cb, 
       Mutable::Selector_t selector = 0){
    return std::make_shared<ReqScan>(spec, cells, cb, selector);
  }

  ReqScan() {}

  ReqScan(Specs::Interval::Ptr spec, Mutable::Ptr cells, Cb_t cb, 
          Mutable::Selector_t selector = 0)
          : spec(spec), cells(cells), cb(cb), 
            offset(spec->flags.offset), limit(spec->flags.limit), size(0), 
            selector(selector) {}

  void response(int err) {
    cb(err);
  }

  void adjust() {
    if(size == 0 || size == cells->size())
      return; 
    //cells->adjust(spec, &offset, &limit);
    
    size = cells->size();
  }

  bool more() {
    return limit > 0;
  }
 
  virtual ~ReqScan() {}
  
  const std::string to_string() const {
    std::string s("ReqScan(");
    s.append(spec->to_string());
    s.append(" ");
    s.append(cells->to_string());
    return s;
  }

  Specs::Interval::Ptr      spec;
  Mutable::Ptr              cells;
  Cb_t                      cb;
  const Mutable::Selector_t selector;

  // state of a scan
  size_t                  offset;
  uint32_t                limit;
  size_t                  size;
};

}}}

#endif