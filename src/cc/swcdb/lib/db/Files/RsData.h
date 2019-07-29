/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RsData_h
#define swcdb_db_Files_RsData_h



#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Time.h"


namespace SWC { namespace Files {

class RsData;
typedef std::shared_ptr<RsData> RsDataPtr;

namespace {
  const int HEADER_SIZE=5;
  const int8_t VERSION=1;
}

class RsData {

  /* file-format: 
      header: i8(version), i32(data-len)
      data:   i64(ts), vi64(rs_id), i32(num-points), [endpoint]
  */

  public:
  static RsDataPtr get_rs(FS::InterfacePtr fs_if, std::string filepath){
    RsDataPtr rs_data = std::make_shared<RsData>();
    
    int err=0;
    if(fs_if->get_fs()->exists(err, filepath)){
      FS::SmartFdPtr smartfd = FS::SmartFd::make_ptr(filepath, 0);
      rs_data->read(fs_if, smartfd);
      fs_if->get_fs()->close(err, smartfd);
    }
    return rs_data;
  }

  RsData(): version(VERSION), rs_id(0), timestamp(0) { }

  virtual ~RsData(){ }

  // GET 
  void read(FS::InterfacePtr fs_if, FS::SmartFdPtr smartfd){
    int err=0;

    fs_if->get_fs()->open(err, smartfd);
    if(!smartfd->valid())
      return;

    uint8_t buf[HEADER_SIZE];
    const uint8_t *ptr = buf;
    if (fs_if->get_fs()->read(err, smartfd,buf, 
                              HEADER_SIZE) != HEADER_SIZE)
      return;

    size_t remain = HEADER_SIZE;
    version = Serialization::decode_i8(&ptr, &remain);
    size_t sz = Serialization::decode_i32(&ptr, &remain);

    StaticBuffer read_buf(sz);
    ptr = read_buf.base;
    if (fs_if->get_fs()->read(err, smartfd, read_buf.base, sz) != sz)
      return;

    read(&ptr, &sz);
  }

  void read(const uint8_t **ptr, size_t* remain) {

    timestamp = Serialization::decode_i64(ptr, remain);
    rs_id = Serialization::decode_vi64(ptr, remain);

    uint32_t len = Serialization::decode_i32(ptr, remain);
    for(size_t i=0;i<len;i++)
      endpoints.push_back(Serialization::decode(ptr, remain));
  }
  
  // SET 
  bool set_rs(FS::InterfacePtr fs_if, std::string filepath, int64_t ts = 0){
    int err=0;
    FS::SmartFdPtr smartfd = 
      FS::SmartFd::make_ptr(filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);

    fs_if->get_fs()->create(err, smartfd, -1, -1, -1);
    if(err > 0) 
      return false;

    DynamicBuffer input;
    write(input, ts==0 ? Time::now_ns() : ts);
    StaticBuffer send_buf(input);
    fs_if->get_fs()->append(err, smartfd, send_buf, FS::Flags::FLUSH);
    fs_if->get_fs()->close(err, smartfd);
    return err == 0;
  }

  void write(SWC::DynamicBuffer &dst_buf, int64_t ts){
    
    size_t len = 12 // (ts+endpoints.size)
               + Serialization::encoded_length_vi64(rs_id.load());
    for(auto endpoint : endpoints)
      len += Serialization::encoded_length(endpoint);
    dst_buf.ensure(HEADER_SIZE+len);

    Serialization::encode_i8(&dst_buf.ptr, version);
    Serialization::encode_i32(&dst_buf.ptr, len);

    Serialization::encode_i64(&dst_buf.ptr, ts);
    Serialization::encode_vi64(&dst_buf.ptr, rs_id.load());
    
    Serialization::encode_i32(&dst_buf.ptr, endpoints.size());
    for(auto endpoint : endpoints)
      Serialization::encode(endpoint, &dst_buf.ptr);
  
    assert(dst_buf.fill() <= dst_buf.size);
  }

  std::string to_string(){
    std::string s("RsData(");
    s.append("endpoints=(");
    for(auto e : endpoints){
      s.append("[");
      s.append(e.address().to_string());
      s.append("]:");
      s.append(std::to_string(e.port()));
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
  
  int8_t     version;
  std::atomic<int64_t>   rs_id;
  int64_t   timestamp;
  EndPoints endpoints;
};

}}
#endif