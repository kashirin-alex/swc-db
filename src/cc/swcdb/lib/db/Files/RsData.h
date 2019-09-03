/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RsData_h
#define swcdb_db_Files_RsData_h


#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Time.h"
#include "swcdb/lib/core/Checksum.h"

namespace SWC { namespace Files {


class RsData;
typedef std::shared_ptr<RsData> RsDataPtr;

class RsData {
  /* file-format: 
      header: i8(version), i32(data-len), 
              i32(data-checksum), i32(header-checksum),
      data:   i64(ts), vi64(rs_id), 
              i32(num-points), [endpoint]
  */

  public:

  static const int HEADER_SIZE=13;
  static const int HEADER_OFFSET_CHKSUM=9;
  
  static const int8_t VERSION=1;

  static RsDataPtr get_rs(std::string filepath){
    RsDataPtr data = std::make_shared<RsData>();
    try{
      data->read(FS::SmartFd::make_ptr(filepath, 0));
    } catch(...){
      data = std::make_shared<RsData>();
    }
    return data;
  }

  RsData(): version(VERSION), rs_id(0), timestamp(0) { }

  void read(FS::SmartFdPtr smartfd) {
    int err;
    for(;;) {
      err = Error::OK;
    
      if(!Env::FsInterface::fs()->exists(err, smartfd->filepath())){
        if(err != Error::OK)
          continue;
        return;
      }

      Env::FsInterface::fs()->open(err, smartfd);
      if(!smartfd->valid())
        continue;
      if(err != Error::OK){
        Env::FsInterface::fs()->close(err, smartfd);
        continue;
      }

      uint8_t buf[HEADER_SIZE];
      const uint8_t *ptr = buf;
      if(Env::FsInterface::fs()->read(err, smartfd, buf, 
                                      HEADER_SIZE) != HEADER_SIZE){
        if(err != Error::FS_EOF){
          Env::FsInterface::fs()->close(err, smartfd);
          continue;
        }
        break;
      }

      size_t remain = HEADER_SIZE;
      version = Serialization::decode_i8(&ptr, &remain);
      size_t sz = Serialization::decode_i32(&ptr, &remain);
      size_t chksum_data = Serialization::decode_i32(&ptr, &remain);
      
      if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                           buf, HEADER_SIZE, HEADER_OFFSET_CHKSUM))
        break;


      StaticBuffer read_buf(sz);
      ptr = read_buf.base;
      if(Env::FsInterface::fs()->read(err, smartfd, read_buf.base, sz) != sz){
        if(err != Error::FS_EOF){
          Env::FsInterface::fs()->close(err, smartfd);
          continue;
        }
        break;
      }

      if(!checksum_i32_chk(chksum_data, ptr, sz))
        break;

      read(&ptr, &sz);
      break;
    }
    
    if(smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd);
  }

  void read(const uint8_t **ptr, size_t* remain) {

    timestamp = Serialization::decode_i64(ptr, remain);
    rs_id = Serialization::decode_vi64(ptr, remain);

    uint32_t len = Serialization::decode_i32(ptr, remain);
    for(size_t i=0;i<len;i++)
      endpoints.push_back(Serialization::decode(ptr, remain));
  }
  
  // SET 
  bool set_rs(std::string filepath, int64_t ts = 0){

    FS::SmartFdPtr smartfd = 
      FS::SmartFd::make_ptr(filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);

    DynamicBuffer input;
    write(input, ts==0 ? Time::now_ns() : ts);
    StaticBuffer send_buf(input);
    send_buf.own=false;

    int err;
    for(;;) {
      err = Error::OK;
      Env::FsInterface::fs()->write(err, smartfd, -1, -1, send_buf);
      if (err == Error::OK)
        return true;
      else if(err == Error::FS_FILE_NOT_FOUND 
              || err == Error::FS_PERMISSION_DENIED)
        return false;
      HT_DEBUGF("set_rs, retrying to err=%d(%s)", err, Error::get_text(err));
    } 
    send_buf.own=true;
  }

  void write(SWC::DynamicBuffer &dst_buf, int64_t ts){
    
    size_t len = 12 // (ts+endpoints.size)
               + Serialization::encoded_length_vi64(rs_id.load());
    for(auto& endpoint : endpoints)
      len += Serialization::encoded_length(endpoint);
    dst_buf.ensure(HEADER_SIZE+len);

    Serialization::encode_i8(&dst_buf.ptr, version);
    Serialization::encode_i32(&dst_buf.ptr, len);

    uint8_t* checksum_data_ptr = dst_buf.ptr;
    Serialization::encode_i32(&dst_buf.ptr, 0);
    uint8_t* checksum_header_ptr = dst_buf.ptr;
    Serialization::encode_i32(&dst_buf.ptr, 0);

    const uint8_t* start_data_ptr = dst_buf.ptr;
    Serialization::encode_i64(&dst_buf.ptr, ts);
    Serialization::encode_vi64(&dst_buf.ptr, rs_id.load());
    
    Serialization::encode_i32(&dst_buf.ptr, endpoints.size());
    for(auto& endpoint : endpoints)
      Serialization::encode(endpoint, &dst_buf.ptr);

    checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
    checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);

    assert(dst_buf.fill() <= dst_buf.size);
  }

  std::string to_string(){
    std::string s("RsData(");
    s.append("endpoints=(");
    for(auto& endpoint : endpoints){
      s.append("[");
      s.append(endpoint.address().to_string());
      s.append("]:");
      s.append(std::to_string(endpoint.port()));
      s.append(",");
    }
    s.append(")");
    
    s.append(", version=");
    s.append(std::to_string(version));
    s.append(", rs_id=");
    s.append(std::to_string(rs_id.load()));
    s.append(", timestamp=");
    s.append(std::to_string(timestamp));
    s.append(")");
    return s;
  } 
  
  virtual ~RsData(){ }

  int8_t     version;
  std::atomic<int64_t>   rs_id;
  int64_t   timestamp;
  EndPoints endpoints;

};

} // Files namespace


namespace Env {
class RsData {  
  public:

  static void init() {
    m_env = std::make_shared<RsData>();
  }

  static Files::RsDataPtr get(){
    HT_ASSERT(m_env != nullptr);
    return m_env->m_rs_data;
  }

  RsData() : m_rs_data(std::make_shared<Files::RsData>()) {}
  virtual ~RsData(){}

  private:
  Files::RsDataPtr                      m_rs_data = nullptr;
  inline static std::shared_ptr<RsData> m_env = nullptr;
};
}

}
#endif