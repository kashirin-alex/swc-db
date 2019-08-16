/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RangeData_h
#define swcdb_db_Files_RangeData_h


#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Time.h"

#include "swcdb/lib/db/Cells/Cell.h"
#include "swcdb/lib/db/Cells/KeysInterval.h"
#include "swcdb/lib/db/Files/CellStore.h"


namespace SWC { namespace Files {

class RangeData;
typedef std::shared_ptr<RangeData> RangeDataPtr;

class RangeData {
  /* file-format: 
      header: i8(version), i32(data-len)
      data:   vi32(num-intervals), [vi32(cellstore-count),
                                    interval vi32(kslen), begin([chars\0]) 
                                    interval vi32(kslen), end([chars\0])   ]
  */

  public:

  static const int HEADER_SIZE=5;
  static const int8_t VERSION=1;

  static RangeDataPtr get_data(const std::string filepath){
    RangeDataPtr data = std::make_shared<RangeData>(
      FS::SmartFd::make_ptr(filepath, 0));
    
    int err = Error::OK;
    if(EnvFsInterface::fs()->exists(err, filepath))
      data->read();
    if(data->cellstores.size() == 0) {
      // readir
    }
    return data;
  }
  
  Cells::KeysIntervalPtr            interval;
  std::vector<Files::CellStorePtr>  cellstores;

  RangeData(FS::SmartFdPtr smartfd)
          : m_smartfd(smartfd), version(VERSION), 
            interval(std::make_shared<Cells::KeysInterval>()) { }

  virtual ~RangeData(){ }

  const std::string to_string(){
    std::string s("RangeData(version=");
    s.append(std::to_string(version));
    s.append(", ");
    s.append(interval->to_string());

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

  // GET 
  void read(){
    int err = Error::OK;

    EnvFsInterface::fs()->open(err, m_smartfd);
    if(!m_smartfd->valid())
      return;

    uint8_t buf[HEADER_SIZE];
    const uint8_t *ptr = buf;
    if (EnvFsInterface::fs()->read(err, m_smartfd, buf, 
                              HEADER_SIZE) != HEADER_SIZE){
      EnvFsInterface::fs()->close(err, m_smartfd);
      return;
    }

    size_t remain = HEADER_SIZE;
    version = Serialization::decode_i8(&ptr, &remain);
    size_t sz = Serialization::decode_i32(&ptr, &remain);

    StaticBuffer read_buf(sz);
    ptr = read_buf.base;
    if (EnvFsInterface::fs()->read(err, m_smartfd, read_buf.base, sz) == sz)
      read(&ptr, &sz);
    
    EnvFsInterface::fs()->close(err, m_smartfd);
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

  void read(const uint8_t **ptr, size_t* remain) {
    const uint8_t *ptr_end = *ptr+*remain;
    
    uint32_t len = Serialization::decode_vi32(ptr, remain);
    for(size_t i=0;i<len;i++) {
      CellStorePtr cs = std::make_shared<CellStore>(
        Serialization::decode_vi32(ptr, remain)); 
      cs->interval->decode(ptr, remain);
      cellstores.push_back(cs);
    }

    if(*ptr != ptr_end){
      HT_WARNF("decode overrun remain=%d", remain);
      cellstores.clear();
    }

    // interval - apply by cellstores keys(lowest-begin + highest-end);
  }

  void write(SWC::DynamicBuffer &dst_buf){
    
    size_t sz = Serialization::encoded_length_vi32(cellstores.size());
    for(auto cs : cellstores) {
      sz += Serialization::encoded_length_vi32(cs->cs_id)
          + cs->interval->encoded_length();
    }
    dst_buf.ensure(HEADER_SIZE+sz);

    Serialization::encode_i8(&dst_buf.ptr, version);
    Serialization::encode_i32(&dst_buf.ptr, sz);

    Serialization::encode_vi32(&dst_buf.ptr, cellstores.size());
    for(auto cs : cellstores){
      Serialization::encode_vi32(&dst_buf.ptr, cs->cs_id);
      cs->interval->encode(&dst_buf.ptr);
    }
  
    assert(dst_buf.fill() <= dst_buf.size);
  }

  int8_t                version;
  FS::SmartFdPtr        m_smartfd;
};


} // Files namespace

}
#endif