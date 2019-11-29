/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RangeData_h
#define swcdb_db_Files_RangeData_h


#include "swcdb/lib/core/DynamicBuffer.h"
#include "swcdb/lib/core/Serialization.h"
#include "swcdb/lib/core/Time.h"

#include "swcdb/lib/db/Cells/Interval.h"
#include "CellStoreReaders.h"


namespace SWC { namespace Files { namespace RangeData {

const int HEADER_SIZE=13;
const int HEADER_OFFSET_CHKSUM=9;
const int8_t VERSION=1;

/* file-format: 
    header: i8(version), i32(data-len), 
            i32(data-checksum), i32(header-checksum)
    data:   vi32(num-cellstores) ,[vi32(cellstore-id), interval]
*/


// SET 
void write(SWC::DynamicBuffer &dst_buf, 
           const CellStore::Readers::Ptr cellstores) {

  size_t sz = cellstores->encoded_length();
  dst_buf.ensure(HEADER_SIZE+sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  uint8_t* checksum_data_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);
  uint8_t* checksum_header_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);

  const uint8_t* start_data_ptr = dst_buf.ptr;
  cellstores->encode(&dst_buf.ptr);

  checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
  checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);
  
  assert(dst_buf.fill() <= dst_buf.size);
}

void save(int& err, const CellStore::Readers::Ptr cellstores) {

  DynamicBuffer input;
  write(input, cellstores);
  StaticBuffer send_buf(input);

  Env::FsInterface::interface()->write(
    err,
    FS::SmartFd::make_ptr(
      cellstores->range->get_path(DB::RangeBase::range_data_file), 
      FS::OpenFlags::OPEN_FLAG_OVERWRITE
    ), 
    -1, -1, 
    send_buf
  );
}


//  GET
void read(const uint8_t **ptr, size_t* remain, 
          const CellStore::Readers::Ptr cellstores) {
  
  const uint8_t *ptr_end = *ptr+*remain;
  cellstores->decode(ptr, remain);

  if(*ptr != ptr_end){
    SWC_LOGF(LOG_WARN, "decode overrun remain=%d", remain);
    cellstores->clear();
  }
}

void load(int& err, const CellStore::Readers::Ptr cellstores){
  HT_ASSERT(cellstores != nullptr);
  FS::SmartFd::Ptr smartfd = FS::SmartFd::make_ptr(
    cellstores->range->get_path(DB::RangeBase::range_data_file), 0);

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

    StaticBuffer read_buf;
    if(Env::FsInterface::fs()->read(err, smartfd, &read_buf, sz) != sz){
      if(err != Error::FS_EOF){
        Env::FsInterface::fs()->close(err, smartfd);
        continue;
      }
      break;
    }
    ptr = read_buf.base;

    if(!checksum_i32_chk(chksum_data, ptr, sz))
      break;
    
    read(&ptr, &sz, cellstores);
    break;
  } 

  if(smartfd->valid())
    Env::FsInterface::fs()->close(err, smartfd);
  
  if(err || cellstores->empty()) {
    err = Error::OK;
    cellstores->load_from_path(err);
    if(!err && !cellstores->empty())
      save(err, cellstores);
    std::cout << cellstores->to_string() << "\n";
  }
}

}}}
#endif