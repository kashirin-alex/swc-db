/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_db_Columns_RangeBase_h
#define swcdb_db_Columns_RangeBase_h


namespace SWC { namespace DB {
  


class RangeBase final {
  public:

  static constexpr const char* COLUMN_DIR = "C"; 
  static constexpr const char* RANGE_DIR = "/R"; 
  static constexpr const char* RANGER_FILE = "ranger.data";
  // (swc.fs.path.data)+COLUMN_DIR+/+{cid}+/RANGE_DIR+/+{rid}+/+(types)
  
  static const char* get_column_path() {
    return COLUMN_DIR;
  }

  static const std::string get_column_path(const cid_t cid) {
    std::string s(get_column_path());
    //if(!s.empty())
    s.append("/");
    FS::set_structured_id(std::to_string(cid), s);
    return s;
  }

  static const std::string get_path(const cid_t cid) {
    std::string s(get_column_path(cid));
    s.append(RANGE_DIR);
    return s;
  }
  
  static const std::string get_path(const cid_t cid, const rid_t rid) {
    std::string s(get_path(cid));
    s.append("/");
    FS::set_structured_id(std::to_string(rid), s);
    s.append("/");
    return s;
  }

  static const std::string get_path_ranger(const std::string& range_path) {
    std::string s(range_path);
    s.append(RANGER_FILE);
    return s;
  }
  
  
};

}}
#endif // swcdb_db_Columns_RangeBase_h