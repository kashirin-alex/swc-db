/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Schemas_h
#define swcdb_manager_Schemas_h


#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace Manager {


class Schemas final : public DB::Schemas {
  public:

  static constexpr const char schemas_file[] = "schemas.store";

  SWC_CAN_INLINE
  Schemas() noexcept { }

  void store_create(int& err, uint8_t replicas, uint32_t blksz,
                    const DB::Types::Encoder cfg_encoder) {
    auto& fs_if = Env::FsInterface::interface();
    auto smartfd = FS::SmartFd::make_ptr(
      schemas_file, FS::OpenFlags::OPEN_FLAG_OVERWRITE);

    Env::FsInterface::interface()->create(err, smartfd, 0, replicas, -1);
    if(err)
      return store_remove(smartfd);

    uint64_t size = 0;
    DynamicBuffer blk_buff;
    blk_buff.ensure(blksz);

    DB::SchemasVec entries;
    DB::Schemas::all(entries);
    for(auto it = entries.cbegin(); it != entries.cend(); ) {
      blk_buff.ensure((*it)->encoded_length());
      (*it)->encode(&blk_buff.ptr);
      if(++it == entries.cend() || blk_buff.fill() >= blksz) {
        store_make_block(
          err,
          &size,
          blk_buff,
          cfg_encoder,
          smartfd,
          it == entries.cend()
        );
        if(err)
          return store_remove(smartfd);
        blk_buff.clear();
        blk_buff.ensure(blksz);
      }
    }
    fs_if->close(err, smartfd);
    if(!err && (fs_if->length(err, smartfd->filepath()) != size || err)) {
      if(!err)
        err = Error::FS_EOF;
      SWC_LOGF(LOG_DEBUG, "Not-matching file-size=%lu", size);
    }
    if(err)
      store_remove(smartfd);
  }

  bool store_load(int& err) {
    auto& fs_if = Env::FsInterface::interface();
    auto smartfd = FS::SmartFd::make_ptr(schemas_file, 0);

    if(!fs_if->exists(err, smartfd->filepath())) {
      if(err)
        store_remove(smartfd);
      return false;
    }

    size_t length = fs_if->length(err, smartfd->filepath());
    if(!err)
      fs_if->open(err, smartfd);
    auto& fs = fs_if->get_fs();
    const uint8_t* ptr;
    size_t remain;
    if(!err) {
      uint8_t buf[8];
      ptr = buf;
      remain = 8;
      if(fs->pread(err, smartfd, length - 8, buf, 8) != 8 ||
         Serialization::decode_i64(&ptr, &remain) != length) {
        err = Error::FS_EOF;
      } else {
        length -= 8;
      }
    }
    uint8_t buf[BLOCK_HEADER_SIZE];
    StaticBuffer buffer;
    for(size_t offset = 0; !err && offset < length; ) {
      if(fs->pread(err, smartfd, offset, buf, BLOCK_HEADER_SIZE)
                  != BLOCK_HEADER_SIZE) {
        err = Error::FS_EOF;
        break;
      }
      offset += BLOCK_HEADER_SIZE;
      ptr = buf;
      remain = BLOCK_HEADER_SIZE;
      DB::Types::Encoder encoder = DB::Types::Encoder(
        Serialization::decode_i8(&ptr, &remain));
      uint32_t size_enc = Serialization::decode_i32(&ptr, &remain);
      uint32_t size_plain = Serialization::decode_i32(&ptr, &remain);
      uint32_t checksum_data = Serialization::decode_i32(&ptr, &remain);
      if(!Core::checksum_i32_chk(Serialization::decode_i32(&ptr, &remain),
                                 buf, BLOCK_HEADER_SIZE - 4)) {
        err = Error::CHECKSUM_MISMATCH;
        break;
      }

      if(fs->pread(err, smartfd, offset, &buffer, size_enc) != size_enc) {
        err = Error::FS_EOF;
        break;
      }
      offset += size_enc;
      if(!Core::checksum_i32_chk(checksum_data, buffer.base, size_enc)) {
        err = Error::CHECKSUM_MISMATCH;
        break;
      }
      if(encoder != DB::Types::Encoder::PLAIN) {
        StaticBuffer decoded_buf(static_cast<size_t>(size_plain));
        Core::Encoder::decode(
          err, encoder,
          buffer.base, size_enc,
          decoded_buf.base, size_plain
        );
        if(err)
          break;
        buffer.set(decoded_buf);
      }
      ptr = buffer.base;
      remain = size_plain;
      while(remain) {
        DB::Schemas::replace(DB::Schema::make(&ptr, &remain));
      }
      buffer.free();
    }
    store_remove(smartfd);
    return err ? false : true;
  }


  private:

  static constexpr const uint8_t BLOCK_HEADER_SIZE = 1 + 4 + 4 + 4 + 4;

  void store_remove(FS::SmartFd::Ptr& smartfd) {
    int err = Error::OK;
    if(smartfd->valid())
      Env::FsInterface::interface()->close(err, smartfd);
    Env::FsInterface::interface()->remove(err, smartfd->filepath());
  }

  void store_make_block(int& err, uint64_t* sizep, DynamicBuffer& blk_buff,
                        DB::Types::Encoder encoder, FS::SmartFd::Ptr& smartfd,
                        bool last_blk) {
    uint32_t size_plain = blk_buff.fill();
    size_t len_enc = 0;
    DynamicBuffer output;
    Core::Encoder::encode(err, encoder, blk_buff.base, size_plain,
                          &len_enc, output, BLOCK_HEADER_SIZE);
    if(err)
      return;

    uint32_t size_enc;
    if(len_enc) {
      size_enc = len_enc;
    } else {
      encoder = DB::Types::Encoder::PLAIN;
      size_enc = size_plain;
    }
    uint8_t* ptr = output.base;
    const uint8_t* base = ptr;
    Serialization::encode_i8(&ptr, uint8_t(encoder));
    Serialization::encode_i32(&ptr, size_enc);
    Serialization::encode_i32(&ptr, size_plain);
    Core::checksum_i32(base + BLOCK_HEADER_SIZE, size_enc, &ptr);
    Core::checksum_i32(base, ptr, &ptr);

    *sizep += output.fill();
    if(last_blk) {
      // trailer (total-size)
      *sizep += 8;
      output.ensure(8);
      Serialization::encode_i64(&output.ptr, *sizep);
    }
    StaticBuffer buff_write(output);
    Env::FsInterface::fs()->append(
      err,
      smartfd,
      buff_write,
      FS::Flags::FLUSH
    );
  }

};



}}

#endif // swcdb_manager_Schemas_h
