/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStore_h
#define swcdb_db_Files_CellStore_h


namespace SWC { namespace Files {

class CellStore;
typedef std::shared_ptr<CellStore> CellStorePtr;

class CellStore {
  /* file-format: 
      header: i8(version), vi64(data-len)
      data:   i32(num-intervals), [interval vi32(kslen), begin({chars\0}) 
                                   interval vi32(kslen), end({chars\0}) 
                                   : vi32(cellstore)]
  */

  public:

  static const int HEADER_SIZE=12;
  static const int8_t VERSION=1;

  Cells::KeysIntervalPtr  interval;
  uint32_t                cs_id;

  CellStore(uint32_t cs_id)
            : cs_id(cs_id), version(VERSION),
              interval(std::make_shared<Cells::KeysInterval>()) { }

  virtual ~CellStore(){}

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CellStore(version=");
    s.append(std::to_string(version));
    s.append(", cs_id=");
    s.append(std::to_string(cs_id));
    s.append(", ");
    s.append(interval->to_string());
    if(m_smartfd != nullptr){
      s.append(", ");
      s.append(m_smartfd->to_string());
    }
    s.append(")");
    return s;
  } 

  private:
  std::mutex              m_mutex;

  int8_t                  version;
  FS::SmartFdPtr          m_smartfd = nullptr;

};

} // Files namespace

}
#endif