/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStore.h"


namespace SWC { namespace Ranger { namespace CellStore {


Read::Ptr Read::make(int& err, const csid_t csid,
                     const RangePtr& range,
                     const DB::Cells::Interval& interval, bool chk_base) {
  auto smartfd = FS::SmartFd::make_ptr(range->get_path_cs(csid), 0);
  DB::Cell::Key prev_key_end;
  DB::Cell::Key key_end;
  DB::Cells::Interval interval_by_blks(range->cfg->key_seq);
  std::vector<Block::Read::Ptr> blocks;
  uint32_t cell_revs = 0;
  try {
    load_blocks_index(
      err, smartfd, prev_key_end, key_end, interval_by_blks,
      blocks, cell_revs, chk_base);
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
    err = e.code();
  }
  if(err)
    SWC_LOG_OUT(LOG_ERROR,
      Error::print(SWC_LOG_OSTREAM << "CellStore load_blocks_index ", err);
      SWC_LOG_OSTREAM << " csid=" << csid
        << " range(" << range->cfg->cid << '/' << range->rid << ") ";
      interval.print(SWC_LOG_OSTREAM);
    );

  return new Read(
    csid,
    prev_key_end,
    key_end,
    interval_by_blks.was_set ? interval_by_blks : interval,
    blocks,
    cell_revs,
    smartfd
  );
}

bool Read::load_trailer(int& err, FS::SmartFd::Ptr& smartfd,
                         uint32_t& cell_revs,
                         uint32_t& blks_idx_count,
                         uint64_t& blks_idx_offset,
                         bool close_after, bool chk_base) {
  const auto& fs_if = Env::FsInterface::interface();
  const auto& fs = Env::FsInterface::fs();

  bool loaded = false;
  err = Error::OK;
  while(err != Error::FS_EOF) {
    if(err) {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        smartfd->print(SWC_LOG_OSTREAM << ' ');
      );
      fs_if->close(err, smartfd);
      err = Error::OK;
    }

    if(!fs_if->exists(err, smartfd->filepath())) {
      if(!err)
        err = Error::FS_PATH_NOT_FOUND;
      return loaded;
    }

    size_t length = fs_if->length(err, smartfd->filepath());
    if(err)
      return loaded;

    if(!smartfd->valid() && !fs_if->open(err, smartfd) && err)
      return loaded;
    if(err)
      continue;

    uint8_t buf[TRAILER_SIZE];
    const uint8_t *ptr = buf;
    if(fs->pread(err, smartfd, length - TRAILER_SIZE, buf,
                 TRAILER_SIZE) != TRAILER_SIZE) {
      if(chk_base)
        break;
      continue;
    }

    size_t remain = TRAILER_SIZE;
    Serialization::decode_i8(&ptr, &remain); // int8_t version =
    cell_revs = Serialization::decode_i32(&ptr, &remain);
    blks_idx_count = Serialization::decode_i32(&ptr, &remain);
    blks_idx_offset = Serialization::decode_i64(&ptr, &remain);

    if(!Core::checksum_i32_chk(Serialization::decode_i32(&ptr, &remain),
                               buf, TRAILER_SIZE-4)) {
      err = Error::CHECKSUM_MISMATCH;
      if(chk_base)
        break;
      continue;
    }
    loaded = true;
    break;
  }

  if(close_after)
    fs_if->close(err, smartfd);
  return loaded;
}

void Read::load_blocks_index(int& err, FS::SmartFd::Ptr& smartfd,
                              DB::Cell::Key& prev_key_end,
                              DB::Cell::Key& key_end,
                              DB::Cells::Interval& interval,
                              std::vector<Block::Read::Ptr>& blocks,
                              uint32_t& cell_revs,
                              bool chk_base) {
  uint32_t blks_idx_count = 0;
  uint64_t offset_fixed = 0;
  uint64_t offset;

  const auto& fs_if = Env::FsInterface::interface();
  const auto& fs = Env::FsInterface::fs();

  StaticBuffer read_buf;
  bool trailer = false;
  err = Error::OK;

  const uint8_t* ptr;
  size_t remain;

  DB::Types::Encoder idx_encoder;
  size_t idx_size_plain;
  size_t idx_size_enc;
  uint32_t idx_checksum_data;

  uint32_t blks_count;
  Block::Header header(interval.key_seq);

  while(err != Error::FS_EOF) {
    if(err) {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        smartfd->print(SWC_LOG_OSTREAM << ' ');
      );
      fs_if->close(err, smartfd);
      err = Error::OK;
    }

    if(!trailer) {
      if(!load_trailer(err, smartfd, cell_revs, blks_idx_count, offset_fixed,
                       false, chk_base))
        break;
      trailer = true;
    }
    offset = offset_fixed;

    if(!smartfd->valid() && !fs_if->open(err, smartfd) && err)
      return;
    if(err)
      continue;

    for(uint32_t i=0; i < blks_idx_count; ++i) {

      for(auto blk : blocks)
        delete blk;
      blocks.clear();

      read_buf.free();
      if(fs->pread(err, smartfd, offset, &read_buf,
                   IDX_BLKS_HEADER_SIZE) != IDX_BLKS_HEADER_SIZE)  {
        trailer = false;
        break;
      }
      ptr = read_buf.base;
      remain = IDX_BLKS_HEADER_SIZE;

      idx_encoder = DB::Types::Encoder(
        Serialization::decode_i8(&ptr, &remain));
      idx_size_enc = Serialization::decode_i32(&ptr, &remain);
      idx_size_plain = Serialization::decode_i32(&ptr, &remain);
      idx_checksum_data = Serialization::decode_i32(&ptr, &remain);
      if(!Core::checksum_i32_chk(Serialization::decode_i32(&ptr, &remain),
                                 read_buf.base, IDX_BLKS_HEADER_SIZE - 4)) {
        err = Error::CHECKSUM_MISMATCH;
        break;
      }
      offset += IDX_BLKS_HEADER_SIZE;

      read_buf.free();
      if(fs->pread(err, smartfd, offset, &read_buf,
                   idx_size_enc) != idx_size_enc)  {
        trailer = false;
        break;
      }
      if(!Core::checksum_i32_chk(idx_checksum_data,
                                 read_buf.base, idx_size_enc)) {
        err = Error::CHECKSUM_MISMATCH;
        trailer = false;
        break;
      }
      offset += idx_size_enc;

      if(idx_encoder != DB::Types::Encoder::PLAIN) {
        StaticBuffer decoded_buf(idx_size_plain);
        Core::Encoder::decode(err, idx_encoder, read_buf.base, idx_size_enc,
                              decoded_buf.base, idx_size_plain);
        if(err)
          break;
        read_buf.set(decoded_buf);
      }

      ptr = read_buf.base;
      remain = idx_size_plain;
      if(!i)
        prev_key_end.decode(&ptr, &remain, true);
      blks_count = Serialization::decode_vi32(&ptr, &remain);

      for(uint32_t blk_i = 0; blk_i < blks_count; ) {
        header.decode_idx(&ptr, &remain);
        interval.align(header.interval);
        if(++blk_i == blks_count && i + 1 == blks_idx_count)
          key_end.copy(header.interval.key_end);

        if(header.is_any & Block::Header::ANY_BEGIN)
          header.interval.key_begin.free();
        if(header.is_any & Block::Header::ANY_END)
          header.interval.key_end.free();
        interval.expand(header.interval);
        blocks.push_back(new Block::Read(header));
      }

    }
    if(!trailer || err) {
      if(chk_base)
        break;
      continue;
    }
    break;
  }

  fs_if->close(err, smartfd);
}

//

Read::Read(const csid_t csid,
           const DB::Cell::Key& prev_key_end,
           const DB::Cell::Key& key_end,
           const DB::Cells::Interval& interval,
           const std::vector<Block::Read::Ptr>& blocks,
           const uint32_t cell_revs,
           const FS::SmartFd::Ptr& smartfd)
          : csid(csid),
            prev_key_end(prev_key_end),
            key_end(key_end),
            interval(interval),
            blocks(blocks),
            cell_revs(cell_revs),
            smartfd(smartfd) {
  Env::Rgr::res().more_mem_usage(size_of());
  for(auto blk : blocks)
    blk->init(this);
}

Read::~Read() {
  for(auto blk : blocks)
    delete blk;
  Env::Rgr::res().less_mem_usage(size_of());
}

size_t Read::size_of() const {
  return sizeof(*this)
        + prev_key_end.size
        + interval.size_of_internal()
        + blocks.size() * sizeof(Block::Read::Ptr)
        + sizeof(*smartfd.get())
      ;
}

const std::string& Read::filepath() const {
  return smartfd->filepath();
}

void Read::load_cells(BlockLoader* loader) {
  for(auto blk : blocks) {
    if(loader->block->is_consist(blk->header.interval)) {
      loader->add(blk);
      if(blk->load(loader)) {
        if(m_queue.activating(blk))
          blk->load();
      }
    } else if(!blk->header.interval.key_end.empty() &&
              !loader->block->is_in_end(blk->header.interval.key_end))
      break;
  }
}

void Read::_run_queued() {
  Block::Read::Ptr blk = nullptr;
  m_queue.deactivating(&blk) ? _release_fd() : blk->load();
}

void Read::get_blocks(int&, std::vector<Block::Read::Ptr>& to) const {
  to.insert(to.end(), blocks.begin(), blocks.end());
}

size_t Read::release(size_t bytes) {
  size_t released = 0;
  for(auto blk : blocks) {
    released += blk->release();
    if(released >= bytes)
      break;
  }

  if(!m_queue.is_active())
    _release_fd();
  return released;
}

void Read::_release_fd() {
  if(smartfd->valid() &&
     Env::FsInterface::interface()->need_fds() &&
     !processing()) {
    int err = Error::OK;
    close(err);
  }
}

SWC_SHOULD_INLINE
void Read::close(int &err) {
  int32_t fd = smartfd->invalidate();
  if(fd != -1) {
    auto r = FS::SmartFd::make_ptr(smartfd->filepath(), smartfd->flags(), fd);
    Env::FsInterface::interface()->close(err, r);
  }
}

SWC_SHOULD_INLINE
void Read::remove(int &err) {
  Env::FsInterface::interface()->remove(err, smartfd->filepath());
}

bool Read::processing() const noexcept{
  if(m_queue.is_active())
    return true;

  for(auto blk : blocks)
    if(blk->processing())
      return true;
  return false;
}

size_t Read::size_bytes(bool only_loaded) const {
  size_t size = 0;
  for(auto blk : blocks)
    size += blk->size_bytes(only_loaded);
  return size;
}

size_t Read::size_bytes_enc(bool only_loaded) const {
  size_t size = 0;
  for(auto blk : blocks)
    size += blk->size_bytes_enc(only_loaded);
  return size;
}

SWC_SHOULD_INLINE
size_t Read::blocks_count() const {
  return blocks.size();
}

void Read::print(std::ostream& out, bool minimal) const {
  out << "Read(v=" << int(VERSION)
      << " csid=" << csid
      << " prev=" << prev_key_end
      << " end=" << key_end
      << ' ' << interval
      << " file=" << smartfd->filepath()
      << " blocks=" << blocks_count();
  if(!minimal) {
    out << " blocks=[";
    for(auto blk : blocks) {
      blk->print(out);
      out << ", ";
    }
    out << ']';
  }
  out << " queue=" << m_queue.size()
      << " processing=" << processing()
      << " used/actual=" << size_bytes(true) << '/' << size_bytes()
      << ')';
}






Write::Write(const csid_t csid, const std::string& filepath,
             const RangePtr& range, uint32_t cell_revs)
            : csid(csid),
              smartfd(
                FS::SmartFd::make_ptr(
                  filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE)
              ),
              encoder(range->cfg->block_enc()),
              block_size(range->cfg->block_size()),
              cell_revs(cell_revs),
              size(0),
              interval(range->cfg->key_seq) {
}

Write::~Write() { }

void Write::create(int& err, int32_t bufsz, uint8_t blk_replicas,
                   int64_t blksz) {
  while(
    Env::FsInterface::interface()->create(
      err, smartfd, bufsz, blk_replicas, blksz));
}

void Write::block_encode(int& err, DynamicBuffer& cells_buff,
                         Block::Header& header) {
  header.encoder = encoder;
  DynamicBuffer output;
  Block::Write::encode(err, cells_buff, output, header);
  if(err)
    return;

  block_write(err, output, header);
}

void Write::block_write(int& err, DynamicBuffer& blk_buff,
                        Block::Header& header) {
  header.offset_data = size + Block::Header::SIZE;
  auto& blk = m_blocks.emplace_back(new Block::Write(header));
  block(err, blk_buff);

  blk->released = true;
  Env::Rgr::res().less_mem_usage(blk->header.size_enc);
}

void Write::block(int& err, DynamicBuffer& blk_buff) {
  StaticBuffer buff_write(blk_buff);
  size += buff_write.size;

  Env::FsInterface::fs()->append(
    err,
    smartfd,
    buff_write,
    FS::Flags::FLUSH
  );
}

void Write::write_blocks_index(int& err, uint32_t& blks_idx_count) {
  interval.free();
  if(m_blocks.empty())
    return;

  blks_idx_count = 0;
  uint32_t blks_count = 0;
  uint32_t len_data = 0;
  Block::Write::Ptr blk;

  auto it = m_blocks.begin();
  auto it_last = it;
  do {
    blk = *it;
    interval.expand(blk->header.interval);
    interval.align(blk->header.interval);

    if(!blks_idx_count && !blks_count)
      len_data += prev_key_end.encoded_length();
    ++blks_count;
    len_data += blk->header.encoded_length_idx();

    if(++it == m_blocks.end() || len_data >= block_size) {
      len_data += Serialization::encoded_length_vi32(blks_count);

      StaticBuffer raw_buffer(static_cast<size_t>(len_data));
      uint8_t* ptr = raw_buffer.base;

      if(!blks_idx_count)
        prev_key_end.encode(&ptr);
      ++blks_idx_count;
      Serialization::encode_vi32(&ptr, blks_count);

      for(; it_last != it; ++it_last)
        (*it_last)->header.encode_idx(&ptr);

      DynamicBuffer buffer_write;
      size_t len_enc = 0;
      Core::Encoder::encode(err, encoder, raw_buffer.base, len_data,
                            &len_enc, buffer_write, IDX_BLKS_HEADER_SIZE);
      raw_buffer.free();
      if(err)
        return;
      if(!len_enc) {
        encoder = DB::Types::Encoder::PLAIN;
        len_enc = len_data;
      }

      uint8_t* header_ptr = buffer_write.base;
      Serialization::encode_i8(&header_ptr, uint8_t(encoder));
      Serialization::encode_i32(&header_ptr, len_enc);
      Serialization::encode_i32(&header_ptr, len_data);
      Core::checksum_i32(
        buffer_write.base + IDX_BLKS_HEADER_SIZE, len_enc, &header_ptr);
      Core::checksum_i32(buffer_write.base, header_ptr, &header_ptr);

      StaticBuffer buff_write(buffer_write);
      Env::FsInterface::fs()->append(
        err,
        smartfd,
        buff_write,
        FS::Flags::FLUSH
      );
      if(err)
        return;

      size += buff_write.size;
      blks_count = len_data = 0;
    }
  } while (it != m_blocks.end());
}

void Write::write_trailer(int& err) {
  uint64_t blks_idx_offset = size;
  uint32_t blks_idx_count = 0;
  write_blocks_index(err, blks_idx_count);
  if(err)
    return;

  StaticBuffer buff_write(static_cast<size_t>(TRAILER_SIZE));
  uint8_t* ptr = buff_write.base;
  Serialization::encode_i8(&ptr, VERSION);
  Serialization::encode_i32(&ptr, cell_revs);
  Serialization::encode_i32(&ptr, blks_idx_count);
  Serialization::encode_i64(&ptr, blks_idx_offset);
  Core::checksum_i32(buff_write.base, ptr, &ptr);

  Env::FsInterface::fs()->append(
    err,
    smartfd,
    buff_write,
    FS::Flags::FLUSH
  );

  size += buff_write.size;
}

void Write::close_and_validate(int& err) {
  Env::FsInterface::interface()->close(err, smartfd);

  if(Env::FsInterface::interface()->length(err, smartfd->filepath())
      != size || err)
    err = Error::FS_EOF;
  // + trailer-checksum
}

void Write::finalize(int& err) {
  write_trailer(err);
  if(!err) {
    close_and_validate(err);
    if(!err)
      return;
  }
  int tmperr = Error::OK;
  remove(tmperr);
}

void Write::remove(int &err) {
  Env::FsInterface::interface()->close(err, smartfd);
  Env::FsInterface::interface()->remove(err, smartfd->filepath());
}

void Write::print(std::ostream& out) const {
  out << "Write(v=" << int(CellStore::VERSION)
      << " size=" << size
      << " encoder=" << Core::Encoder::to_string(encoder)
      << " cell_revs=" << cell_revs
      << " prev=" << prev_key_end
      << ' ' << interval;
  smartfd->print(out << ' ');
  out << " blocks=" << m_blocks.size()
      << " blocks=[";
  for(auto blk : m_blocks) {
    blk->print(out);
    out << ", ";
  }
  out << "])";
}


Read::Ptr create_initial(int& err, const RangePtr& range) {
  Write writer(1, range->get_path_cs(1), range, range->cfg->cell_versions());
  writer.create(
    err, -1, range->cfg->file_replication(), range->cfg->block_size());
  if(err)
    return nullptr;


  Block::Header header(range->cfg->key_seq);
  range->_get_interval(header.interval);

  if(header.interval.key_begin.empty())
    header.is_any |= Block::Header::ANY_BEGIN;
  if(header.interval.key_end.empty())
    header.is_any |= Block::Header::ANY_END;

  DynamicBuffer cells_buff;
  writer.block_encode(err, cells_buff, header);
  if(!err) {
    writer.finalize(err);
    if(!err) {
      auto cs = Read::make(err, 1, range, header.interval, true);
      if(!err)
        return cs;
      delete cs;
    }
  }
  int errtmp;
  writer.remove(errtmp);
  return nullptr;

}


}}} // namespace SWC::Ranger::CellStore
