/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_Files_RgrData_h
#define swcdb_common_Files_RgrData_h


#include "swcdb/core/Serialization.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/core/Time.h"
#include "swcdb/core/Checksum.h"


namespace SWC { namespace Common { namespace Files {


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
      
    if(!Core::checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain), 
          read_buf.base, HEADER_SIZE, HEADER_OFFSET_CHKSUM) ||
       !Core::checksum_i32_chk(chksum_data, ptr, sz) ) {
      err = Error::CHECKSUM_MISMATCH;
      return;
    }

    timestamp = Serialization::decode_i64(&ptr, &remain);
    rgrid.store(Serialization::decode_vi64(&ptr, &remain));

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
    write(input, ts ? ts : Time::now_ns());
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
    dst_buf.ensure(HEADER_SIZE + len);

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

    Core::checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
    Core::checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);

    SWC_ASSERT(dst_buf.fill() <= dst_buf.size);
  }

  void print(std::ostream& out) {
    out << "RgrData(endpoints=[";
    for(auto& endpoint : endpoints)
      out << endpoint << ',';
    out << "] version="   << int(version)
        << " rgrid="      << rgrid.load()
        << " timestamp="  << timestamp
        << ')';
  }
  
  ~RgrData(){ }

  int8_t                version;
  Core::Atomic<rgrid_t> rgrid;
  int64_t               timestamp;
  Comm::EndPoints       endpoints;

};


}}} // SWC::Common::Files namespace



#endif // swcdb_common_Files_RgrData_h
