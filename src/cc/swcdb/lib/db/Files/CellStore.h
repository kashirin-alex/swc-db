/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStore_h
#define swcdb_db_Files_CellStore_h


namespace SWC { namespace Files {


class CellStore {
  /* file-format: 
      header: 
      data:   
  */

  public:

  static const int HEADER_SIZE=12;
  static const int8_t VERSION=1;

  typedef std::shared_ptr<CellStore> Ptr;

  DB::Cells::Interval::Ptr interval;
  const uint32_t            cs_id;

  CellStore(uint32_t cs_id)
            : cs_id(cs_id), version(VERSION),
              interval(std::make_shared<DB::Cells::Interval>()) { }

  virtual ~CellStore(){}

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CellStore(version=");
    s.append(std::to_string(version));
    s.append(" cs_id=");
    s.append(std::to_string(cs_id));
    s.append(" ");
    s.append(interval->to_string());
    if(m_smartfd != nullptr){
      s.append(" ");
      s.append(m_smartfd->to_string());
    }
    s.append(")");
    return s;
  } 

  void remove(int &err) {}

  void load_trailer() {

    // sets cs->interval
  }

  private:
  std::mutex              m_mutex;

  int8_t                  version;
  FS::SmartFdPtr          m_smartfd = nullptr;

};
typedef std::vector<Files::CellStore::Ptr> CellStores;


} // Files namespace

}
#endif