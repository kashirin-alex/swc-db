/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Encoder.h"
#include "swcdb/ranger/db/CommitLogFragment.h"

namespace SWC { namespace Ranger { namespace CommitLog {


std::string Fragment::to_string(Fragment::State state) {
  switch(state) {
    case State::NONE:
      return std::string("NONE");
    case State::LOADING:
      return std::string("LOADING");
    case State::LOADED:
      return std::string("LOADED");
    case State::WRITING:
      return std::string("WRITING");
    default:
      return std::string("UKNOWN");
  }
}


Fragment::Ptr Fragment::make_read(int& err, const std::string& filepath, 
                                  const Types::KeySeq key_seq) {
  auto smartfd = FS::SmartFd::make_ptr(filepath, 0);
    
  uint8_t               version;
  DB::Cells::Interval   interval(key_seq);
  Types::Encoding       encoder;
  size_t                size_plain;
  size_t                size_enc;
  uint32_t              cell_revs;
  uint32_t              cells_count;
  uint32_t              data_checksum;
  uint32_t              offset_data;

  load_header(
    err, smartfd, 
    version, interval, 
    encoder, size_plain, size_enc, 
    cell_revs, cells_count, data_checksum, offset_data
  );

  return err ? nullptr : new Fragment(
    smartfd, 
    version, interval, 
    encoder, size_plain, size_enc, 
    cell_revs, cells_count, data_checksum, offset_data, 
    State::NONE
  );
}

void Fragment::load_header(int& err, FS::SmartFd::Ptr& smartfd, 
                           uint8_t& version,
                           DB::Cells::Interval& interval, 
                           Types::Encoding& encoder,
                           size_t& size_plain, size_t& size_enc,
                           uint32_t& cell_revs, uint32_t& cells_count,
                           uint32_t& data_checksum, uint32_t& offset_data) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();

  while(err != Error::FS_PATH_NOT_FOUND &&
        err != Error::SERVER_SHUTTING_DOWN) {
    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s", 
        err, Error::get_text(err), smartfd->to_string().c_str());
      fs_if->close(err, smartfd);
      err = Error::OK;
    }

    if(!smartfd->valid() && !fs_if->open(err, smartfd) && err)
      return;
    if(err)
      continue;
    
    StaticBuffer buf;
    if(fs->pread(err, smartfd, 0, &buf, HEADER_SIZE) != HEADER_SIZE)
      continue;
    
    const uint8_t *ptr = buf.base;

    size_t remain = HEADER_SIZE;
    version = Serialization::decode_i8(&ptr, &remain);
    uint32_t header_extlen = Serialization::decode_i32(&ptr, &remain);
    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         buf.base, HEADER_SIZE-4)) {  
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }
    buf.free();
    
    if(fs->pread(err, smartfd, HEADER_SIZE, &buf, header_extlen) 
        != header_extlen)
      continue;

    ptr = buf.base;
    remain = header_extlen;

    interval.decode(&ptr, &remain, true);
    encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
    size_enc = Serialization::decode_i32(&ptr, &remain);
    size_plain = Serialization::decode_i32(&ptr, &remain);
    cell_revs = Serialization::decode_i32(&ptr, &remain);
    cells_count = Serialization::decode_i32(&ptr, &remain);
    data_checksum = Serialization::decode_i32(&ptr, &remain);

    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         buf.base, header_extlen-4)) {  
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }
    offset_data = HEADER_SIZE+header_extlen;
    break;
  }

  int tmperr = Error::OK;
  fs_if->close(tmperr, smartfd);
}


Fragment::Ptr Fragment::make_write(int& err, const std::string& filepath, 
                                   const DB::Cells::Interval& interval,
                                   Types::Encoding encoder,
                                   const uint32_t cell_revs, 
                                   const uint32_t cells_count,
                                   DynamicBuffer& cells, 
                                   StaticBuffer::Ptr& buffer) {
  auto smartfd = FS::SmartFd::make_ptr(
    filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);
    
  const uint8_t version = VERSION;
  const size_t  size_plain = cells.fill();
  size_t        size_enc;
  uint32_t      data_checksum;
  uint32_t      offset_data;

  write(
    err, smartfd, 
    version, interval, 
    encoder, size_plain, size_enc, 
    cell_revs, cells_count, data_checksum, offset_data, 
    cells, buffer
  );
  if(err)
    return nullptr;

  auto frag = new Fragment(
    smartfd, 
    version, interval, 
    encoder, size_plain, size_enc, 
    cell_revs, cells_count, data_checksum, offset_data, 
    State::WRITING
  );
  frag->m_buffer.set(cells);
  return frag;
}

void Fragment::write(int& err, FS::SmartFd::Ptr& smartfd, 
                     const uint8_t version,
                     const DB::Cells::Interval& interval, 
                     Types::Encoding& encoder,
                     const size_t size_plain, size_t& size_enc,
                     const uint32_t cell_revs, const uint32_t cells_count,
                     uint32_t& data_checksum, uint32_t& offset_data,
                     DynamicBuffer& cells, StaticBuffer::Ptr& buffer) {
  uint32_t header_extlen = interval.encoded_length()+HEADER_EXT_FIXED_SIZE;
  offset_data = HEADER_SIZE + header_extlen;

  DynamicBuffer output;
  size_enc = 0;
  err = Error::OK;
  Encoder::encode(err, encoder, cells.base, size_plain, 
                  &size_enc, output, offset_data);
  if(err)
    return;

  if(!size_enc) {
    size_enc = size_plain;
    encoder = Types::Encoding::PLAIN;
  }
                  
  uint8_t * bufp = output.base;
  Serialization::encode_i8(&bufp, version);
  Serialization::encode_i32(&bufp, header_extlen);
  checksum_i32(output.base, bufp, &bufp);

  uint8_t * header_extptr = bufp;
  interval.encode(&bufp);
  Serialization::encode_i8(&bufp, (uint8_t)encoder);
  Serialization::encode_i32(&bufp, size_enc);
  Serialization::encode_i32(&bufp, size_plain);
  Serialization::encode_i32(&bufp, cell_revs);
  Serialization::encode_i32(&bufp, cells_count);
  
  checksum_i32(output.base+offset_data, output.base+output.fill(), 
               &bufp, data_checksum);
  checksum_i32(header_extptr, bufp, &bufp);

  buffer->set(output);
}


Fragment::Fragment(const FS::SmartFd::Ptr& smartfd, 
                   const uint8_t version,
                   const DB::Cells::Interval& interval, 
                   const Types::Encoding encoder,
                   const size_t size_plain, const size_t size_enc,
                   const uint32_t cell_revs, const uint32_t cells_count,
                   const uint32_t data_checksum, const uint32_t offset_data,
                   Fragment::State state)
                  : version(version), interval(interval), encoder(encoder),
                    size_plain(size_plain), size_enc(size_enc), 
                    cell_revs(cell_revs), cells_count(cells_count),
                    data_checksum(data_checksum), offset_data(offset_data),
                    m_state(state), m_smartfd(smartfd), 
                    m_processing(m_state == State::WRITING), 
                    m_err(Error::OK),
                    m_cells_remain(cells_count) {
}

SWC_SHOULD_INLINE
Fragment::Ptr Fragment::ptr() {
  return this;
}

Fragment::~Fragment() { }

SWC_SHOULD_INLINE
const std::string& Fragment::get_filepath() const {
  return m_smartfd->filepath();
}

void Fragment::write(int err, uint8_t blk_replicas, int64_t blksz, 
                     const StaticBuffer::Ptr& buff_write, Semaphore* sem) {
  if(!err && (Env::FsInterface::interface()->length(err, m_smartfd->filepath())
              != buff_write->size || err))
    if(err != Error::SERVER_SHUTTING_DOWN)
      err = Error::FS_EOF;

  if(err && err != Error::SERVER_SHUTTING_DOWN) {
    if(err != Error::UNPOSSIBLE)
      SWC_LOGF(LOG_WARN, "write, retrying to err=%d(%s) %s", 
               err, Error::get_text(err), to_string().c_str());

    Env::FsInterface::fs()->write(
      [this, blk_replicas, blksz, buff_write, sem]
      (int err, const FS::SmartFd::Ptr& smartfd) {
        write(err, blk_replicas, blksz, buff_write, sem);
      }, 
      m_smartfd, blk_replicas, blksz, *buff_write.get()
    );
    return;
  }

  //if(err) remains Error::SERVER_SHUTTING_DOWN -- write local dump

  m_smartfd->flags(0);
  sem->release();
  buff_write->own = true;

  bool keep;
  {
    Mutex::scope lock(m_mutex);
    keep = --m_processing || !m_queue.empty();
    m_err = err;
    if((m_state = !m_err && keep ? State::LOADED : State::NONE) 
                                              != State::LOADED)
      m_buffer.free();
  }
  if(keep)
    run_queued();
  else if(Env::Resources.need_ram(size_plain))
    release();
}

void Fragment::load(const QueueRunnable::Call_t& cb) {
  bool loaded;
  {
    Mutex::scope lock(m_mutex);
    ++m_processing;
    if(!(loaded = m_state == State::LOADED)) {
      m_queue.push(cb);

      if(m_state == State::LOADING || m_state == State::WRITING)
        return;
      m_state = State::LOADING;
    }
  }

  if(loaded)
    cb();
  else
    asio::post(*Env::IoCtx::io()->ptr(), [ptr=ptr()](){ ptr->load(); } );
}

void Fragment::load_cells(int& err, Ranger::Block::Ptr cells_block) {
  bool was_splitted = false;
  if(m_buffer.size) {
    m_cells_remain -= cells_block->load_cells(
      m_buffer.base, m_buffer.size, 
      cell_revs, cells_count, 
      was_splitted
    );
  } else {
    SWC_LOGF(LOG_WARN, "Fragment::load_cells empty buf %s", 
             to_string().c_str());
  }
  processing_decrement();

  if(!m_cells_remain || Env::Resources.need_ram(size_plain))
    release();
}

void Fragment::load_cells(int& err, DB::Cells::MutableVec& cells) {
  if(m_buffer.size) {
    size_t count = 0;
    DB::Cells::Cell cell;
    bool synced = cells.empty();
    size_t offset_hint = 0;
    size_t offset_it_hint = 0;
    const uint8_t* buf = m_buffer.base;
    size_t remain = m_buffer.size;
    while(remain) {
      ++count;
      try {
        cell.read(&buf, &remain);

      } catch(...) {
        SWC_LOGF(LOG_ERROR, "Cell trunclated at count=%lu/%u remain=%lu, %s",
                count, cells_count, remain, to_string().c_str());
        break;
      }
      if(synced)
        cells.add_sorted(cell);
      else
        cells.add_raw(cell, &offset_it_hint, &offset_hint);
    }
  } else {
    SWC_LOGF(LOG_WARN, "Fragment::load_cells empty buf %s", 
             to_string().c_str());
  }
  processing_decrement();
}

void Fragment::split(int& err, const DB::Cell::Key& key, 
                     Fragments::Ptr log_left, Fragments::Ptr log_right) {
  if(m_buffer.size) {
    size_t count = 0;
    DB::Cells::Cell cell;
    const uint8_t* buf = m_buffer.base;
    size_t remain = m_buffer.size;

    while(remain) {
      ++count;
      try {
        cell.read(&buf, &remain);

      } catch(...) {
        SWC_LOGF(LOG_ERROR, 
          "Cell trunclated at count=%lu/%u remain=%lu, %s",
          count, cells_count, remain, to_string().c_str());
        break;
      }

      if(DB::KeySeq::compare(interval.key_seq, key, cell.key) == Condition::GT)
        log_right->add(cell);
      else
        log_left->add(cell);
    }
  } else {
    SWC_LOGF(LOG_WARN, "Fragment::load_cells empty buf %s", 
      to_string().c_str());
  }
  processing_decrement();
  release();
}

void Fragment::processing_increment() {
  Mutex::scope lock(m_mutex);
  ++m_processing; 
}

void Fragment::processing_decrement() {
  Mutex::scope lock(m_mutex);
  --m_processing; 
}

size_t Fragment::release() {
  size_t released = 0;     
  bool support;
  if(m_mutex.try_full_lock(support)) {
    if(!m_processing && m_state == State::LOADED) {
      m_state = State::NONE;
      released += m_buffer.size;
      m_buffer.free();
      m_cells_remain = cells_count;
    }
    m_mutex.unlock(support);
  }
  return released;
}

bool Fragment::loaded() {
  Mutex::scope lock(m_mutex);
  return m_state == State::LOADED;
}

int Fragment::error() {
  Mutex::scope lock(m_mutex);
  return m_err;
}

bool Fragment::loaded(int& err) {
  Mutex::scope lock(m_mutex);
  err = m_err;
  return !err && m_state == State::LOADED;
}

SWC_SHOULD_INLINE
size_t Fragment::size_bytes() const {
  return size_plain;
}

size_t Fragment::size_bytes(bool only_loaded) {
  return only_loaded && !loaded() ? 0 : size_plain;
}

SWC_SHOULD_INLINE
size_t Fragment::size_bytes_encoded() {
  return size_enc;
}

bool Fragment::processing() {
  bool support;
  bool busy;
  if(!(busy = !m_mutex.try_full_lock(support))) {
    busy = m_processing;
    m_mutex.unlock(support);
  }
  return busy;
}

void Fragment::remove(int &err) {
  if(m_smartfd->valid()) {
    int tmperr = Error::OK;
    Env::FsInterface::interface()->close(tmperr, m_smartfd);
  }
  Env::FsInterface::interface()->remove(err, m_smartfd->filepath()); 
}

std::string Fragment::to_string() {
  std::string s("Fragment(version=");
  s.append(std::to_string((int)version));

  s.append(" count=");
  s.append(std::to_string(cells_count));
  s.append(" offset=");
  s.append(std::to_string(offset_data));

  s.append(" encoder=");
  s.append(Types::to_string(encoder));

  s.append(" enc/size=");
  s.append(std::to_string(size_enc));
  s.append("/");
  s.append(std::to_string(size_plain));
  {
    Mutex::scope lock(m_mutex);
    s.append(" state=");
    s.append(to_string(m_state));
    
    s.append(" ");
    s.append(m_smartfd->to_string());

    s.append(" queue=");
    s.append(std::to_string(m_queue.size()));

    s.append(" processing=");
    s.append(std::to_string(m_processing));
  
    if(m_err) {
      s.append(" m_err=");
      s.append(std::to_string(m_err));
      s.append("(");
      s.append(Error::get_text(m_err));
      s.append(")");
    }
  }
  s.append(" ");
  s.append(interval.to_string());
  s.append(")");
  return s;
}

void Fragment::load() {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();

  int err = Error::OK;
  while(err != Error::FS_PATH_NOT_FOUND &&
        err != Error::SERVER_SHUTTING_DOWN) {
    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s", 
               err, Error::get_text(err), to_string().c_str());
      fs_if->close(err, m_smartfd);
      err = Error::OK;
    }

    if(!m_smartfd->valid() && !fs_if->open(err, m_smartfd) && err)
      break;
    if(err)
      continue;
    
    m_buffer.free();
    if(fs->pread(err, m_smartfd, offset_data, &m_buffer, size_enc) != size_enc)
      continue;
    
    if(!checksum_i32_chk(data_checksum, m_buffer.base, size_enc)) {
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }

    if(encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(size_plain);
      Encoder::decode(
        err, encoder, m_buffer.base, size_enc, decoded_buf.base, size_plain);
      if(err)
        continue;
      m_buffer.set(decoded_buf);
    }
    break;
  }

  if(m_smartfd->valid()) {
    int tmperr = Error::OK;
    fs_if->close(tmperr, m_smartfd);
  }

  {
    Mutex::scope lock(m_mutex);
    m_err = err == Error::FS_PATH_NOT_FOUND ? Error::OK : err;
    m_state = m_err ? State::NONE : State::LOADED;
    if(err)
      m_buffer.free();
  }
  if(err)
    SWC_LOGF(LOG_ERROR, "CommitLog::Fragment load %s", to_string().c_str());

  run_queued();
}


void Fragment::run_queued() {
  if(m_queue.need_run())
    asio::post(
      *Env::IoCtx::io()->ptr(), 
      [this](){ m_queue.run(); }
    );
}



}}} // namespace SWC::Ranger::CommitLog