/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RangeData_h
#define swcdb_db_Files_RangeData_h


#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Time.h"

#include "swcdb/lib/db/Cells/Interval.h"
#include "swcdb/lib/db/Files/CellStore.h"


namespace SWC { namespace Files { namespace RangeData {

const int HEADER_SIZE=13;
const int HEADER_OFFSET_CHKSUM=9;
const int8_t VERSION=1;

/* file-format: 
    header: i8(version), i32(data-len), 
            i32(data-checksum), i32(header-checksum)
    data:   vi32(num-cellstore) ,[vi32(cellstore-count), interval]
*/


// SET 
void write(SWC::DynamicBuffer &dst_buf, const CellStore::Readers &cellstores){

  size_t sz = Serialization::encoded_length_vi32(cellstores.size());
  for(auto& cs : cellstores) {
    sz += Serialization::encoded_length_vi32(cs->id)
          + cs->interval->encoded_length();
  }
  dst_buf.ensure(HEADER_SIZE+sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  uint8_t* checksum_data_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);
  uint8_t* checksum_header_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);

  const uint8_t* start_data_ptr = dst_buf.ptr;

  Serialization::encode_vi32(&dst_buf.ptr, cellstores.size());
  for(auto& cs : cellstores){
    Serialization::encode_vi32(&dst_buf.ptr, cs->id);
    cs->interval->encode(&dst_buf.ptr);
  }

  checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
  checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);
  
  assert(dst_buf.fill() <= dst_buf.size);
}

void save(int &err, DB::RangeBase::Ptr range, 
          CellStore::ReadersPtr &cellstores){

  DynamicBuffer input;
  write(input, *cellstores.get());
  StaticBuffer send_buf(input);

  Env::FsInterface::interface()->write(
    err,
    FS::SmartFd::make_ptr(
      range->get_path(DB::RangeBase::range_data_file), 
      FS::OpenFlags::OPEN_FLAG_OVERWRITE
    ), 
    -1, -1, 
    send_buf
  );
}


//  GET
void read(const uint8_t **ptr, size_t* remain, 
          DB::RangeBase::Ptr range, CellStore::ReadersPtr &cellstores) {
  const uint8_t *ptr_end = *ptr+*remain;
    
  uint32_t len = Serialization::decode_vi32(ptr, remain);
  for(size_t i=0;i<len;i++) {
    cellstores->push_back(
      std::make_shared<CellStore::Read>(
        Serialization::decode_vi32(ptr, remain), 
        range, 
        std::make_shared<DB::Cells::Interval>(ptr, remain)
      )
    );
  }

  if(*ptr != ptr_end){
    HT_WARNF("decode overrun remain=%d", remain);
    cellstores->clear();
  }
}

void load_by_path(int &err, DB::RangeBase::Ptr range, 
                  CellStore::ReadersPtr &cellstores){
  FS::IdEntries_t entries;
  Env::FsInterface::interface()->get_structured_ids(
    err, range->get_path(DB::RangeBase::cellstores_dir), entries);
  
  for(auto id : entries){
    cellstores->push_back(std::make_shared<CellStore::Read>(id, range));
    std::cout << cellstores->back()->to_string() << "\n";
  }
}

void load(int &err, DB::RangeBase::Ptr range, 
          CellStore::ReadersPtr &cellstores){

  FS::SmartFdPtr smartfd = FS::SmartFd::make_ptr(
    range->get_path(DB::RangeBase::range_data_file), 0);

  for(;;) {
    err = Error::OK;
    
    if(!Env::FsInterface::fs()->exists(err, smartfd->filepath())){
      if(err != Error::OK && err != Error::SERVER_SHUTTING_DOWN)
        continue;
      break;
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
    if (Env::FsInterface::fs()->read(err, smartfd, buf, 
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
    
    read(&ptr, &sz, range, cellstores);
    break;
  } 

  if(smartfd->valid())
    Env::FsInterface::fs()->close(err, smartfd);
  
  if(err || cellstores->empty()) {
    cellstores->clear();
    err = Error::OK;
    load_by_path(err, range, cellstores);
    if(!err){
      if(!cellstores->empty()) {
        for(auto& cs: *cellstores.get()) {
          cs->smartfd = FS::SmartFd::make_ptr(range->get_path_cs(cs->id), 0); 
          cs->load_blocks_index(err, true);
        }
        // sort cs
      }
      save(err, range, cellstores);
    }
  }
}

}}}
#endif