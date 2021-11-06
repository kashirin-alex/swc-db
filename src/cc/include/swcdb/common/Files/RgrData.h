/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_Files_RgrData_h
#define swcdb_common_Files_RgrData_h


#include "swcdb/db/Columns/RgrData.h"


namespace SWC { namespace Common { namespace Files {


namespace RgrData {
/*  file-format:
    header: i32(data-len), i32(data-checksum), i32(header-checksum)
    data:   vi64(rgrid), i32(num-points), [endpoint]
*/

static const uint8_t HEADER_SIZE = 12;
static const uint8_t HEADER_OFFSET_CHKSUM = 8;


static void read(DB::RgrData& data, int &err, const std::string& filepath) {
  StaticBuffer read_buf;
  Env::FsInterface::interface()->read(err, filepath, &read_buf);
  if(err)
    return;

  const uint8_t *ptr = read_buf.base;
  size_t remain = read_buf.size;

  size_t sz = Serialization::decode_i32(&ptr, &remain);
  size_t chksum_data = Serialization::decode_i32(&ptr, &remain);

  if(!Core::checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain),
        read_buf.base, HEADER_SIZE, HEADER_OFFSET_CHKSUM) ||
     !Core::checksum_i32_chk(chksum_data, ptr, sz) ) {
    err = Error::CHECKSUM_MISMATCH;
    return;
  }
  data.decode(ptr, remain);
}

static inline void set_rgr(const DB::RgrData& data,
                           int &err, std::string&& filepath,
                           uint8_t replication) noexcept {
  try {
    DynamicBuffer dst_buf;

    size_t len = data.encoded_length();
    dst_buf.ensure(HEADER_SIZE + len);
    Serialization::encode_i32(&dst_buf.ptr, len);

    uint8_t* checksum_data_ptr = dst_buf.ptr;
    Serialization::encode_i32(&dst_buf.ptr, 0);
    uint8_t* checksum_header_ptr = dst_buf.ptr;
    Serialization::encode_i32(&dst_buf.ptr, 0);

    const uint8_t* start_data_ptr = dst_buf.ptr;
    data.encode(&dst_buf.ptr);

    Core::checksum_i32(start_data_ptr, dst_buf.ptr, &checksum_data_ptr);
    Core::checksum_i32(dst_buf.base, start_data_ptr, &checksum_header_ptr);

    StaticBuffer send_buf(dst_buf);
    Env::FsInterface::interface()->write(
      err,
      FS::SmartFd::make_ptr(
        std::move(filepath), FS::OpenFlags::OPEN_FLAG_OVERWRITE),
      replication,
      send_buf
    );
  } catch(...) {
    err = SWC_CURRENT_EXCEPTION("").code();
  }
}

static void get_rgr(DB::RgrData& data ,const std::string& filepath) noexcept {
  int err = Error::OK;
  try {
    read(data, err, filepath);
  } catch(...) {
    err = ENOKEY;
  }
  if(err) {
    data.rgrid.store(0);
    data.endpoints.clear();
  }
}

}}}} // SWC::Common::Files::RgrData namespace



#endif // swcdb_common_Files_RgrData_h
