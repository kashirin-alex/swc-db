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
  typedef std::function<void()>     NextCall_t;

  inline static Ptr 
  make(){
    return std::make_shared<ReqScan>();
  }

  inline static Ptr 
  make(Specs::Interval::Ptr spec, Mutable::Ptr cells, Cb_t cb, 
       const Mutable::Selector_t& selector = 0){
    return std::make_shared<ReqScan>(spec, cells, cb, selector);
  }

  ReqScan(): limit_buffer_sz(0) {}

  ReqScan(Specs::Interval::Ptr spec, Mutable::Ptr cells, Cb_t cb, 
          const Mutable::Selector_t& selector = 0)
          : spec(spec), cells(cells), cb(cb), 
            offset(spec->flags.offset), selector(selector), 
            limit_buffer_sz(0) {
  }

  void response(int err) {
    if(!err && next_call && !reached_limits()){
      auto call = next_call;
      next_call = 0;
      call();
    } else 
      cb(err);
  }

  bool reached_limits() {
    return (spec->flags.limit > 0 && spec->flags.limit == cells->size()) 
           || 
           (limit_buffer_sz > 0 && limit_buffer_sz <= cells->size_bytes());
  }
 
  virtual ~ReqScan() {}
  
  const std::string to_string() const {
    std::string s("ReqScan(");
    s.append(spec->to_string());
    s.append(" selector=");
    s.append(selector?"true":"false");
    s.append(" ");
    s.append(cells->to_string());
    s.append(" limit_buffer_sz=");
    s.append(std::to_string(limit_buffer_sz));
    return s;
  }

  Specs::Interval::Ptr      spec;
  Mutable::Ptr              cells;
  Cb_t                      cb;
  const Mutable::Selector_t selector;

  uint32_t                  limit_buffer_sz = 0;
  NextCall_t                next_call = 0;
  // state of a scan
  size_t                    offset;
};

}}}

#endif