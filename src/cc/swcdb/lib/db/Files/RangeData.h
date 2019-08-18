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


namespace SWC { namespace Files { namespace RangeData {

const int HEADER_SIZE=5;
const int8_t VERSION=1;

/* file-format: 
    header: i8(version), i32(data-len)
    data:   vi32(num-intervals), [vi32(cellstore-count), intervals]
*/


// SET 
void write(SWC::DynamicBuffer &dst_buf, CellStores &cellstores){
    
  size_t sz = Serialization::encoded_length_vi32(cellstores.size());
  for(auto cs : cellstores) {
    sz += Serialization::encoded_length_vi32(cs->cs_id)
        + cs->intervals->encoded_length();
  }
  dst_buf.ensure(HEADER_SIZE+sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  Serialization::encode_vi32(&dst_buf.ptr, cellstores.size());
  for(auto cs : cellstores){
    Serialization::encode_vi32(&dst_buf.ptr, cs->cs_id);
    cs->intervals->encode(&dst_buf.ptr);
  }
  
  assert(dst_buf.fill() <= dst_buf.size);
}

bool save(const std::string filepath, CellStores &cellstores){

  FS::SmartFdPtr smartfd = FS::SmartFd::make_ptr(
    filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);

  int err=Error::OK;
  EnvFsInterface::fs()->create(err, smartfd, -1, -1, -1);
  if(err != Error::OK) 
    return false;

  DynamicBuffer input;
  write(input, cellstores);
  StaticBuffer send_buf(input);
  EnvFsInterface::fs()->append(err, smartfd, send_buf, FS::Flags::SYNC);
  EnvFsInterface::fs()->close(err, smartfd);
  return err == Error::OK;
}


//  GET
void read(const uint8_t **ptr, size_t* remain, CellStores &cellstores) {
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

bool load(const std::string filepath, CellStores &cellstores){
  FS::SmartFdPtr smartfd = FS::SmartFd::make_ptr(filepath, 0);

  int err = Error::OK;
  if(!EnvFsInterface::fs()->exists(err, smartfd->filepath()) 
    || err != Error::OK) 
    return false;

  EnvFsInterface::fs()->open(err, smartfd);
  if(!smartfd->valid())
    return false;

  uint8_t buf[HEADER_SIZE];
  const uint8_t *ptr = buf;
  if (EnvFsInterface::fs()->read(err, smartfd, buf, 
                                HEADER_SIZE) != HEADER_SIZE){
    EnvFsInterface::fs()->close(err, smartfd);
    return false;
  }

  size_t remain = HEADER_SIZE;
  int8_t version = Serialization::decode_i8(&ptr, &remain);
  size_t sz = Serialization::decode_i32(&ptr, &remain);

  StaticBuffer read_buf(sz);
  ptr = read_buf.base;
  if (EnvFsInterface::fs()->read(err, smartfd, read_buf.base, sz) == sz)
    read(&ptr, &sz, cellstores);
    
  EnvFsInterface::fs()->close(err, smartfd);

  return cellstores.size() > 0;
}

void load_by_path(const std::string cs_path, CellStores &cellstores){
  std::cout << "Files::RangeData::load_by_path:\n";
  FS::IdEntries_t entries;
  int err = Error::OK;
  EnvFsInterface::interface()->get_structured_ids(err, cs_path, entries);
  for(auto cs_id : entries){
    CellStorePtr cs = std::make_shared<CellStore>(cs_id); 
    // cs->load_trailer(); // sets cs->intervals
    cellstores.push_back(cs);
    std::cout << cs->to_string() << "\n";
  }
}


}}}
#endif