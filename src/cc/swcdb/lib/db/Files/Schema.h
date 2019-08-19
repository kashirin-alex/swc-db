/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_Schema_h
#define swcdb_db_Files_Schema_h

namespace SWC { namespace Files { namespace Schema {

const int HEADER_SIZE=5;
const int8_t VERSION=1;
const std::string schema_file = "schema.data";

/* file-format: 
    header: i8(version), i32(data-len)
    data:   schema-encoded
*/

const std::string filepath(int64_t cid){
  std::string path = DB::RangeBase::get_column_path(cid);
  path.append("/");
  path.append(schema_file);
  return path;
}

// SET 

void write(SWC::DynamicBuffer &dst_buf, DB::SchemaPtr schema){
    
  size_t sz = schema->encoded_length();
  dst_buf.ensure(HEADER_SIZE+sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  schema->encode(&dst_buf.ptr);
  
  assert(dst_buf.fill() <= dst_buf.size);
}

bool save(DB::SchemaPtr schema){

  FS::SmartFdPtr smartfd = FS::SmartFd::make_ptr(
    filepath(schema->cid), FS::OpenFlags::OPEN_FLAG_OVERWRITE);

  int err=Error::OK;
  EnvFsInterface::fs()->create(err, smartfd, -1, -1, -1);
  if(err != Error::OK) 
    return false;

  DynamicBuffer input;
  write(input, schema);

  StaticBuffer send_buf(input);
  EnvFsInterface::fs()->append(err, smartfd, send_buf, FS::Flags::SYNC);
  
  EnvFsInterface::fs()->close(err, smartfd);
  return err == Error::OK;
}


//  GET

void load(FS::SmartFdPtr smartfd, DB::SchemaPtr &schema, int &err) {

  if(!EnvFsInterface::fs()->exists(err, smartfd->filepath()) 
    || err != Error::OK) 
    return;

  EnvFsInterface::fs()->open(err, smartfd);
  if(!smartfd->valid())
    return;

  uint8_t buf[HEADER_SIZE];
  const uint8_t *ptr = buf;
  if (EnvFsInterface::fs()->read(err, smartfd, buf, 
                                 HEADER_SIZE) != HEADER_SIZE)
    return;
  
  size_t remain = HEADER_SIZE;
  int8_t version = Serialization::decode_i8(&ptr, &remain);
  size_t sz = Serialization::decode_i32(&ptr, &remain);

  StaticBuffer read_buf(sz);
  ptr = read_buf.base;
  if (EnvFsInterface::fs()->read(err, smartfd, read_buf.base, sz) == sz)
    schema = std::make_shared<DB::Schema>(&ptr, &sz);
}

DB::SchemaPtr load(int64_t cid) {

  FS::SmartFdPtr smartfd = FS::SmartFd::make_ptr(filepath(cid), 0);
  DB::SchemaPtr schema = nullptr;
  int err = Error::OK;
  load(smartfd, schema, err);
  if(smartfd->valid())
    EnvFsInterface::fs()->close(err, smartfd);

  if(schema == nullptr){
    HT_WARNF("Missing Column(cid=%d) Schema", cid);
    std::string name;
    if(cid < 4) {
      name.append("sys_");
      if(cid == 3) {
        name.append("stats");
        schema = DB::Schema::make(
          cid, name, Types::Column::COUNTER_I64, 1,
          EnvConfig::settings()->get<int32_t>("swc.stats.ttl", 1036800));
      } else {
        name.append(cid==1?"master":"meta");
        schema = DB::Schema::make(cid, name);
      }
    } else {
      name.append("noname_");
      name.append(std::to_string(cid));
      schema = DB::Schema::make(cid, name);
    }

    HT_WARNF("Missing Column(cid=%d) Schema set to %s", 
              cid, schema->to_string().c_str());
    save(schema);
  }
  
  return schema;
}


}}}

#endif