/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/RangeData.h"

#include "swcdb/core/Serialization.h"
#include "swcdb/core/Time.h"

#include "swcdb/db/Cells/Interval.h"
#include "swcdb/ranger/db/CellStoreReaders.h"


namespace SWC { namespace Ranger {


// SET
void RangeData::write(DynamicBuffer& dst_buf,
                      CellStore::Readers& cellstores) {
  size_t sz = cellstores.encoded_length();
  dst_buf.ensure(HEADER_SIZE + sz);

  Serialization::encode_i8(&dst_buf.ptr, VERSION);
  Serialization::encode_i32(&dst_buf.ptr, sz);

  uint8_t* checksum_data_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);
  uint8_t* checksum_header_ptr = dst_buf.ptr;
  Serialization::encode_i32(&dst_buf.ptr, 0);

  const uint8_t* start_data_ptr = dst_buf.ptr;
  cellstores.encode(&dst_buf.ptr);

  Core::checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
  Core::checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);
}

void RangeData::save(int& err, CellStore::Readers& cellstores) {

  DynamicBuffer input;
  write(input, cellstores);
  StaticBuffer send_buf(input);

  Env::FsInterface::interface()->write(
    err,
    FS::SmartFd::make_ptr(
      cellstores.range->get_path(DB::RangeBase::RANGE_FILE),
      FS::OpenFlags::OPEN_FLAG_OVERWRITE
    ),
    1, // as hint-file without cellstores.range->cfg->file_replication()
    send_buf
  );
}


//  GET
void RangeData::read(int& err, const uint8_t **ptr, size_t* remain,
                     CellStore::Readers& cellstores) {

  const uint8_t *ptr_end = *ptr+*remain;
  cellstores.decode(err, ptr, remain);

  if(*ptr != ptr_end){
    SWC_LOGF(LOG_WARN, "decode overrun remain=" SWC_FMT_LU, *remain);
    cellstores.clear();
  }
}

SWC_CAN_INLINE
void RangeData::load(int& err, CellStore::Readers& cellstores) {
  StaticBuffer read_buf;
  Env::FsInterface::interface()->read(
    err,
    cellstores.range->get_path(DB::RangeBase::RANGE_FILE),
    &read_buf
  );
  if(!err) try {
    const uint8_t *ptr = read_buf.base;
    size_t remain = read_buf.size;

    Serialization::decode_i8(&ptr, &remain); //int8_t version =
    size_t sz = Serialization::decode_i32(&ptr, &remain);

    size_t chksum_data = Serialization::decode_i32(&ptr, &remain);

    if(!Core::checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain),
          read_buf.base, HEADER_SIZE, HEADER_OFFSET_CHKSUM) ||
       !Core::checksum_i32_chk(chksum_data, ptr, sz)) {
      err = Error::CHECKSUM_MISMATCH;
    } else {
      read(err, &ptr, &sz, cellstores);
    }
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
  }

  if(err || cellstores.empty()) {
    err = Error::OK;
    cellstores.load_from_path(err);
    if(!err && !cellstores.empty())
      save(err, cellstores);
  }
}


}}
