/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/RangeData.h"

#include "swcdb/core/Serialization.h"
#include "swcdb/core/Time.h"

#include "swcdb/db/Cells/Interval.h"
#include "swcdb/ranger/db/CellStoreReaders.h"


namespace SWC { namespace Ranger { namespace RangeData {


// SET 
void write(SWC::DynamicBuffer &dst_buf, CellStore::Readers& cellstores) {

  size_t sz = cellstores.encoded_length();
  dst_buf.ensure(HEADER_SIZE+sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  uint8_t* checksum_data_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);
  uint8_t* checksum_header_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);

  const uint8_t* start_data_ptr = dst_buf.ptr;
  cellstores.encode(&dst_buf.ptr);

  checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
  checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);
  
  assert(dst_buf.fill() <= dst_buf.size);
}

void save(int& err, CellStore::Readers& cellstores) {

  DynamicBuffer input;
  write(input, cellstores);
  StaticBuffer send_buf(input);

  Env::FsInterface::interface()->write(
    err,
    FS::SmartFd::make_ptr(
      cellstores.range->get_path(Range::RANGE_FILE), 
      FS::OpenFlags::OPEN_FLAG_OVERWRITE
    ), 
    cellstores.range->cfg->file_replication(), 
    -1, 
    send_buf
  );
}


//  GET
void read(int& err, const uint8_t **ptr, size_t* remain, 
          CellStore::Readers& cellstores) {
  
  const uint8_t *ptr_end = *ptr+*remain;
  cellstores.decode(err, ptr, remain);

  if(*ptr != ptr_end){
    SWC_LOGF(LOG_WARN, "decode overrun remain=%d", remain);
    cellstores.clear();
  }
}

void load(int& err, CellStore::Readers& cellstores) {
  StaticBuffer read_buf;
  Env::FsInterface::interface()->read(
    err, 
    cellstores.range->get_path(Range::RANGE_FILE), 
    &read_buf
  );
  if(!err) {
    const uint8_t *ptr = read_buf.base;
    size_t remain = read_buf.size;

    int8_t version = Serialization::decode_i8(&ptr, &remain);
    size_t sz = Serialization::decode_i32(&ptr, &remain);

    size_t chksum_data = Serialization::decode_i32(&ptr, &remain);
      
    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         read_buf.base, HEADER_SIZE, HEADER_OFFSET_CHKSUM) || 
       !checksum_i32_chk(chksum_data, ptr, sz)) {
      err = Error::CHECKSUM_MISMATCH;
    } else {
      read(err, &ptr, &sz, cellstores);
    }
  }

  if(err || cellstores.empty()) {
    err = Error::OK;
    cellstores.load_from_path(err);
    if(!err && !cellstores.empty())
      save(err, cellstores);
  }
}

}}}