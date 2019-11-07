/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_Schema_h
#define swcdb_db_Files_Schema_h

#include "swcdb/lib/core/Checksum.h"

namespace SWC { namespace Files { namespace Schema {

const uint8_t HEADER_SIZE=13;
const uint8_t HEADER_OFFSET_CHKSUM=9;
const uint8_t VERSION=1;
const std::string schema_file = "schema.data";

/* file-format: 
    header: i8(version), i32(data-len), 
            i32(data-checksum), i32(header-checksum)
    data:   schema-encoded
*/

const std::string filepath(int64_t cid){
  std::string path = DB::RangeBase::get_column_path(cid);
  path.append("/");
  path.append(schema_file);
  return path;
}

// REMOVE

void remove(int &err, int64_t cid){
  Env::FsInterface::interface()->remove(err, filepath(cid));
}

// SET 

void write(SWC::DynamicBuffer &dst_buf, DB::Schema::Ptr schema){
    
  size_t sz = schema->encoded_length();
  dst_buf.ensure(HEADER_SIZE+sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  uint8_t* checksum_data_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);
  uint8_t* checksum_header_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);

  const uint8_t* start_data_ptr = dst_buf.ptr;
  schema->encode(&dst_buf.ptr);

  checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
  checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);

  assert(dst_buf.fill() <= dst_buf.size);
}

void save(int &err, DB::Schema::Ptr schema){
  DynamicBuffer input;
  write(input, schema);
  StaticBuffer send_buf(input);
  
  Env::FsInterface::interface()->write(
    err,
    FS::SmartFd::make_ptr(filepath(schema->cid), 
                          FS::OpenFlags::OPEN_FLAG_OVERWRITE), 
    -1, -1, 
    send_buf
  );
}


//  GET

void load(int &err, FS::SmartFd::Ptr smartfd, DB::Schema::Ptr &schema) {

  for(;;) {
    if(err != Error::OK)
      HT_DEBUGF("load, retrying to err=%d(%s)", err, Error::get_text(err));

    err = Error::OK;
    
    if(!Env::FsInterface::fs()->exists(err, smartfd->filepath())){
      if(err != Error::OK && err != Error::SERVER_SHUTTING_DOWN)
        continue;
      return;
    }

    Env::FsInterface::fs()->open(err, smartfd);
    if(err == Error::FS_PATH_NOT_FOUND || err == Error::FS_PERMISSION_DENIED)
      break;
    if(!smartfd->valid())
      continue;
    if(err != Error::OK) {
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
    int8_t version = Serialization::decode_i8(&ptr, &remain);
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
    
    schema = std::make_shared<DB::Schema>(&ptr, &sz);
    break;
  }

  if(smartfd->valid())
    Env::FsInterface::fs()->close(err, smartfd);
}

DB::Schema::Ptr load(int &err, int64_t cid, bool recover=true) {

  DB::Schema::Ptr schema = nullptr;
  try{
    load(err, FS::SmartFd::make_ptr(filepath(cid), 0), schema);
  } catch (const std::exception& e) {
    HT_ERRORF("schema load exception (%s)", e.what());
    schema = nullptr;
  }

  if(schema == nullptr && err != Error::SERVER_SHUTTING_DOWN && recover){ 
    HT_WARNF("Missing Column(cid=%d) Schema", cid);
    std::string name;
    if(cid < 4) {
      err == Error::OK;
      name.append("sys_");
      if(cid == 3) {
        name.append("stats");
        schema = DB::Schema::make(
          cid, name, Types::Column::COUNTER_I64, 1,
          Env::Config::settings()->get<int32_t>("swc.stats.ttl", 1036800));
      } else {
        name.append(cid==1?"master":"meta");
        schema = DB::Schema::make(cid, name);
      }
    } else {
     // schama backups || instant create || throw ?
      name.append("noname_");
      name.append(std::to_string(cid));
      schema = DB::Schema::make(cid, name);
    }

    HT_WARNF("Missing Column(cid=%d) Schema set to %s", 
              cid, schema->to_string().c_str());
    save(err, schema);
  }
  
  return schema;
}


void save_with_validation(int &err, DB::Schema::Ptr schema_save){
  save(err, schema_save); // ?tmp-file 
  if(err != Error::OK) 
    return;
  DB::Schema::Ptr schema_new = load(err, schema_save->cid, false);
  if(err != Error::OK) 
    return;
  if(!schema_new->equal(schema_save)) {
    err = Error::COLUMN_SCHEMA_BAD_SAVE;
    return;
  }
}

}}}

#endif