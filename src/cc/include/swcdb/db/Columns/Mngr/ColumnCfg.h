/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_ColumnCfg_h
#define swcdb_lib_db_Columns_Rgr_ColumnCfg_h

namespace SWC { namespace DB { 

class ColumnCfg final {
  
  public:
  const int64_t cid;

  ColumnCfg(const int64_t cid) : cid(cid) { 
  }


  virtual ~ColumnCfg() { }


  const std::string to_string() const {
    std::string s("cid=");
    s.append(std::to_string(cid));
    s.append(" blk_size=");
    //s.append(std::to_string(blk_size));
    s.append(")");
    return s;
  }
};

}}

#endif