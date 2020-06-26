/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Files_RgrData_h
#define swcdb_db_Files_RgrData_h


#include "swcdb/core/Serialization.h"
#include "swcdb/core/DynamicBuffer.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Checksum.h"

namespace SWC { namespace Files {


class RgrData final {
  /* file-format: 
      header: i8(version), i32(data-len), 
              i32(data-checksum), i32(header-checksum),
      data:   i64(ts), vi64(rgrid), 
              i32(num-points), [endpoint]
  */

  public:

  typedef std::shared_ptr<RgrData> Ptr;

  static const int HEADER_SIZE=13;
  static const int HEADER_OFFSET_CHKSUM=9;
  
  static const int8_t VERSION=1;

  static Ptr get_rgr(int &err, const std::string& filepath) {
    Ptr data = std::make_shared<RgrData>();
    try{
      data->read(err, filepath);
    } catch(...){
      data = std::make_shared<RgrData>();
    }
    return data;
  }

  RgrData(): version(VERSION), rgrid(0), timestamp(0) { }

  void read(int &err, const std::string& filepath) {

    StaticBuffer read_buf;
    Env::FsInterface::interface()->read(err, filepath, &read_buf);
    if(err) 
      return;
  
    const uint8_t *ptr = read_buf.base;
    size_t remain = read_buf.size;

    version = Serialization::decode_i8(&ptr, &remain);
    size_t sz = Serialization::decode_i32(&ptr, &remain);
    size_t chksum_data = Serialization::decode_i32(&ptr, &remain);
      
    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         read_buf.base, HEADER_SIZE, HEADER_OFFSET_CHKSUM) ||
       !checksum_i32_chk(chksum_data, ptr, sz) ) {
      err = Error::CHECKSUM_MISMATCH;
      return;
    }

    timestamp = Serialization::decode_i64(&ptr, &remain);
    rgrid = Serialization::decode_vi64(&ptr, &remain);

    uint32_t len = Serialization::decode_i32(&ptr, &remain);
    endpoints.clear();
    endpoints.resize(len);
    for(size_t i=0;i<len;++i)
      endpoints[i] = Serialization::decode(&ptr, &remain);
  }
  
  // SET 
  void set_rgr(int &err, std::string filepath, uint8_t replication, 
               int64_t ts = 0) {

    DynamicBuffer input;
    write(input, ts==0 ? Time::now_ns() : ts);
    StaticBuffer send_buf(input);

    Env::FsInterface::interface()->write(
      err,
      FS::SmartFd::make_ptr(filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE), 
      replication, 
      -1, 
      send_buf
    );
  }

  void write(SWC::DynamicBuffer &dst_buf, int64_t ts){
    
    size_t len = 12 // (ts+endpoints.size)
               + Serialization::encoded_length_vi64(rgrid.load());
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
    Serialization::encode_vi64(&dst_buf.ptr, rgrid.load());
    
    Serialization::encode_i32(&dst_buf.ptr, endpoints.size());
    for(auto& endpoint : endpoints)
      Serialization::encode(endpoint, &dst_buf.ptr);

    checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
    checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);

    SWC_ASSERT(dst_buf.fill() <= dst_buf.size);
  }

  std::string to_string(){
    std::string s("RgrData(");
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
    s.append(", rgrid=");
    s.append(std::to_string(rgrid.load()));
    s.append(", timestamp=");
    s.append(std::to_string(timestamp));
    s.append(")");
    return s;
  } 
  
  ~RgrData(){ }

  int8_t     version;
  std::atomic<rgrid_t>   rgrid;
  int64_t   timestamp;
  EndPoints endpoints;

};

}} // SWC::Files namespace

#endif