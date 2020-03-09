/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_manager_db_ColumnCfg_h
#define swc_manager_db_ColumnCfg_h

namespace SWC { namespace Manager { 

class ColumnCfg final {
  
  public:
  const int64_t cid;

  ColumnCfg(const int64_t cid) : cid(cid) { }

  ~ColumnCfg() { }


  const std::string to_string() const {
    std::string s("cid=");
    s.append(std::to_string(cid));
    return s;
  }
};

}}

#endif // swc_manager_db_ColumnCfg_h