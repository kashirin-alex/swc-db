/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RangeData_h
#define swcdb_db_Files_RangeData_h


#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Time.h"

#include "swcdb/lib/db/Cells/Intervals.h"
#include "swcdb/lib/db/Files/CellStore.h"


namespace SWC { namespace Files {

class RangeData;
typedef std::shared_ptr<RangeData> RangeDataPtr;

class RangeData {
  /* file-format: 
      header: i8(version), i32(data-len)
      data:   vi32(num-intervals), [vi32(cellstore-count), intervals]
  */

  public:

  inline static const std::string range_data_file = "range.data";
  static const int HEADER_SIZE=5;
  static const int8_t VERSION=1;

  static RangeDataPtr get_data(const std::string path){
    return std::make_shared<RangeData>(path);
  }

  Cells::IntervalsPtr               intervals;
  std::vector<Files::CellStorePtr>  cellstores;

  RangeData(const std::string range_path)
          : m_path(range_path),
            m_smartfd(FS::SmartFd::make_ptr(get_path(range_data_file), 0)), 
            version(VERSION), intervals(std::make_shared<Cells::Intervals>()) {
    load();   
  }

  virtual ~RangeData(){ }

  const std::string to_string(){
    std::string s("RangeData(version=");
    s.append(std::to_string(version));
    s.append(", ");
    s.append(intervals->to_string());

    s.append(", cellstores=[");
    for(auto cs : cellstores) {
      s.append(cs->to_string());
      s.append(", ");
    }
    s.append("], ");
    s.append(m_smartfd->to_string());
    
    s.append(")");
    return s;
  } 

  const std::string get_path(const std::string suff){
    std::string s(m_path);
    s.append(suff);
    return s;
  }
  // SET 
  bool save(){
    m_smartfd->flags(FS::OpenFlags::OPEN_FLAG_OVERWRITE);

    int err=Error::OK;
    EnvFsInterface::fs()->create(err, m_smartfd, -1, -1, -1);
    if(err != Error::OK) 
      return false;

    DynamicBuffer input;
    write(input);
    StaticBuffer send_buf(input);
    EnvFsInterface::fs()->append(err, m_smartfd, send_buf, FS::Flags::SYNC);
    EnvFsInterface::fs()->close(err, m_smartfd);
    return err == Error::OK;
  }

  private:

  // GET 
  void load() {
    if(!read())
      load_from_path();
    
    int64_t ts;
    int64_t ts_earliest = ScanSpecs::TIMESTAMP_NULL;
    int64_t ts_latest = ScanSpecs::TIMESTAMP_NULL;
    for(auto cs : cellstores){
      // cs->intervals->apply_if_wider(intervals);

      if(intervals->is_any_keys_begin() 
        || !intervals->is_in_begin(cs->intervals->get_keys_begin()))
        intervals->set_keys_begin(cs->intervals);

      if(intervals->is_any_keys_end() 
        || !intervals->is_in_end(cs->intervals->get_keys_end()))
        intervals->set_keys_end(cs->intervals);

      ts = cs->intervals->get_ts_earliest();
      if(ts_earliest == ScanSpecs::TIMESTAMP_NULL || ts_earliest > ts)
        ts_earliest = ts;
      ts = cs->intervals->get_ts_latest();
      if(ts_latest == ScanSpecs::TIMESTAMP_NULL || ts_latest < ts)
        ts_latest = ts;
    }
    
    intervals->set_ts_earliest(ts_earliest);
    intervals->set_ts_latest(ts_latest);

  }

  bool read(){
    int err = Error::OK;

    if(!EnvFsInterface::fs()->exists(err, m_smartfd->filepath()) || err != Error::OK) 
      return false;

    EnvFsInterface::fs()->open(err, m_smartfd);
    if(!m_smartfd->valid())
      return false;

    uint8_t buf[HEADER_SIZE];
    const uint8_t *ptr = buf;
    if (EnvFsInterface::fs()->read(err, m_smartfd, buf, 
                              HEADER_SIZE) != HEADER_SIZE){
      EnvFsInterface::fs()->close(err, m_smartfd);
      return false;
    }

    size_t remain = HEADER_SIZE;
    version = Serialization::decode_i8(&ptr, &remain);
    size_t sz = Serialization::decode_i32(&ptr, &remain);

    StaticBuffer read_buf(sz);
    ptr = read_buf.base;
    if (EnvFsInterface::fs()->read(err, m_smartfd, read_buf.base, sz) == sz)
      read(&ptr, &sz);
    
    EnvFsInterface::fs()->close(err, m_smartfd);

    return cellstores.size() > 0;
  }

  void read(const uint8_t **ptr, size_t* remain) {
    const uint8_t *ptr_end = *ptr+*remain;
    
    uint32_t len = Serialization::decode_vi32(ptr, remain);
    for(size_t i=0;i<len;i++) {
      CellStorePtr cs = std::make_shared<CellStore>(
        Serialization::decode_vi32(ptr, remain)); 
      cs->intervals->decode(ptr, remain);
      cellstores.push_back(cs);
    }

    if(*ptr != ptr_end){
      HT_WARNF("decode overrun remain=%d", remain);
      cellstores.clear();
    }
  }

  void load_from_path(){
    std::cout << "load_from_path:\n";
    FS::IdEntries_t entries;
    int err = Error::OK;
    EnvFsInterface::interface()->get_structured_ids(err, get_path("cs"), entries);
    for(auto cs_id : entries){
      CellStorePtr cs = std::make_shared<CellStore>(cs_id); 
      // cs->load_trailer(); // sets cs->intervals
      cellstores.push_back(cs);
      std::cout << cs->to_string() << "\n";
    }
  }

  // SET
  void write(SWC::DynamicBuffer &dst_buf){
    
    size_t sz = Serialization::encoded_length_vi32(cellstores.size());
    for(auto cs : cellstores) {
      sz += Serialization::encoded_length_vi32(cs->cs_id)
          + cs->intervals->encoded_length();
    }
    dst_buf.ensure(HEADER_SIZE+sz);

    Serialization::encode_i8(&dst_buf.ptr, version);
    Serialization::encode_i32(&dst_buf.ptr, sz);

    Serialization::encode_vi32(&dst_buf.ptr, cellstores.size());
    for(auto cs : cellstores){
      Serialization::encode_vi32(&dst_buf.ptr, cs->cs_id);
      cs->intervals->encode(&dst_buf.ptr);
    }
  
    assert(dst_buf.fill() <= dst_buf.size);
  }

  const std::string     m_path;
  FS::SmartFdPtr        m_smartfd;
  int8_t                version;
};


} // Files namespace

}
#endif