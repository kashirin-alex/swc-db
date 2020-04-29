/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
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

Fragment::Ptr Fragment::make(const std::string& filepath, 
                             const Types::KeySeq key_seq, 
                             Fragment::State state) {
  return new Fragment(filepath, key_seq, state);
}


Fragment::Fragment(const std::string& filepath,
                   const Types::KeySeq key_seq, Fragment::State state)
                  : ts(Time::now_ns()), interval(key_seq), cells_count(0),
                    m_smartfd(
                    FS::SmartFd::make_ptr(
                      filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE)
                    ), 
                    m_state(state), 
                    m_size_enc(0), m_size(0), m_cell_revs(0), m_cells_offset(0), 
                    m_data_checksum(0), m_processing(0), m_cells_remain(0),
                    m_err(Error::OK) {
}

Fragment::Ptr Fragment::ptr() {
  return this;
}

Fragment::~Fragment() { }

const std::string& Fragment::get_filepath() const {
  return m_smartfd->filepath();
}

void Fragment::write(int& err, uint8_t blk_replicas, 
                     Types::Encoding encoder, 
                     DynamicBuffer& cells, uint32_t cell_revs,
                     Semaphore* sem) {

  m_version = VERSION;
  
  uint32_t header_extlen = interval.encoded_length()+HEADER_EXT_FIXED_SIZE;
  m_cell_revs = cell_revs;
  m_cells_remain = cells_count;
  m_size = cells.fill();
  m_cells_offset = HEADER_SIZE+header_extlen;

  DynamicBuffer output;
  m_size_enc = 0;
  err = Error::OK;
  Encoder::encode(err, encoder, cells.base, m_size, 
                  &m_size_enc, output, m_cells_offset);
  if(err)
    return;

  if(m_size_enc) {
    m_encoder = encoder;
  } else {
    m_size_enc = m_size;
    m_encoder = Types::Encoding::PLAIN;
  }
                  
  uint8_t * bufp = output.base;
  Serialization::encode_i8(&bufp, m_version);
  Serialization::encode_i32(&bufp, header_extlen);
  checksum_i32(output.base, bufp, &bufp);

  uint8_t * header_extptr = bufp;
  interval.encode(&bufp);
  Serialization::encode_i8(&bufp, (uint8_t)m_encoder);
  Serialization::encode_i32(&bufp, m_size_enc);
  Serialization::encode_i32(&bufp, m_size);
  Serialization::encode_i32(&bufp, m_cell_revs);
  Serialization::encode_i32(&bufp, cells_count);
  
  checksum_i32(output.base+m_cells_offset, output.base+output.fill(), 
               &bufp, m_data_checksum);
  checksum_i32(header_extptr, bufp, &bufp);

  auto buff_write = std::make_shared<StaticBuffer>(output);
  buff_write->own = false;
  m_buffer.set(cells);
  sem->acquire();
  
  write(
    Error::UNPOSSIBLE, 
    m_smartfd, blk_replicas, m_cells_offset+m_size_enc, 
    buff_write,
    sem
  );
}

void Fragment::write(int err, FS::SmartFd::Ptr smartfd, 
                     uint8_t blk_replicas, int64_t blksz, 
                     StaticBuffer::Ptr buff_write,
                     Semaphore* sem) {
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
      (int err, FS::SmartFd::Ptr smartfd) {
        write(err, smartfd, blk_replicas, blksz, buff_write, sem);
      }, 
      smartfd, blk_replicas, blksz, *buff_write.get()
    );
    return;
  }

  //if(err) remains Error::SERVER_SHUTTING_DOWN -- write local dump
    
  sem->release();

  buff_write->own = true;

  bool keep;
  {
    Mutex::scope lock(m_mutex);
    keep = !m_queue.empty() || m_processing;
    m_err = err;
    if((m_state = !m_err && keep ? State::LOADED : State::NONE) 
                                              != State::LOADED)
      m_buffer.free();
  }
  if(keep)
    run_queued();
  else if(Env::Resources.need_ram(m_size))
    release();
  
}

void Fragment::load_header(bool close_after) {
  m_err = Error::OK;
  load_header(m_err, close_after);
  if(m_err)
    SWC_LOGF(LOG_ERROR, "CommitLog::Fragment load_header %s", 
             to_string().c_str());
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
      m_cell_revs, cells_count, 
      was_splitted
    );
  } else {
    SWC_LOGF(LOG_WARN, "Fragment::load_cells empty buf %s", 
             to_string().c_str());
  }
  
  processing_decrement();

  if(!m_cells_remain.load() || Env::Resources.need_ram(m_size))
    release();
}

void Fragment::load_cells(int& err, DB::Cells::MutableVec& cells) {
  if(m_buffer.size) {
    size_t count = 0;
    DB::Cells::Cell cell;
    bool synced = cells.empty();
    const uint8_t* buf = m_buffer.base;
    size_t remain = m_buffer.size;
    while(remain) {
      ++count;
      try {
        cell.read(&buf, &remain);

      } catch(std::exception) {
        SWC_LOGF(LOG_ERROR, "Cell trunclated at count=%llu/%llu remain=%llu, %s",
                count, cells_count, remain, to_string().c_str());
        break;
      }
      if(synced)
        cells.add_sorted(cell);
      else
        cells.add_raw(cell);
    }
  } else {
    SWC_LOGF(LOG_WARN, "Fragment::load_cells empty buf %s", 
             to_string().c_str());
  }
}

void Fragment::split(int& err, const DB::Cell::Key& key, 
                     Fragments::Ptr log_left, Fragments::Ptr log_right) {
  uint64_t tts = Time::now_ns();
  size_t count = 0;
  if(m_buffer.size) {
    DB::Cells::Cell cell;
    const uint8_t* buf = m_buffer.base;
    size_t remain = m_buffer.size;

    while(remain) {
      ++count;
      try {
        cell.read(&buf, &remain);

      } catch(std::exception) {
        SWC_LOGF(LOG_ERROR, "Cell trunclated at count=%llu/%llu remain=%llu, %s",
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
  
  auto took = Time::now_ns()-tts;
  SWC_PRINT << "Fragment::split"
            << " added=" << count 
            << " avg=" << (count>0 ? took / count : 0)
            << " took=" << took
            << SWC_PRINT_CLOSE;
  
  processing_decrement();
  release();
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

size_t Fragment::size_bytes(bool only_loaded) {
  Mutex::scope lock(m_mutex);
  if(only_loaded && m_state != State::LOADED)
    return 0;
  return m_size;
}

size_t Fragment::size_bytes_encoded() {
  Mutex::scope lock(m_mutex);
  return m_size_enc;
}

bool Fragment::processing() {
  Mutex::scope lock(m_mutex);
  return m_processing;
}

void Fragment::remove(int &err) {
  Env::FsInterface::interface()->remove(err, m_smartfd->filepath()); 
}

std::string Fragment::to_string() {
  Mutex::scope lock(m_mutex);
  std::string s("Fragment(version=");
  s.append(std::to_string(m_version));

  s.append(" state=");
  s.append(to_string(m_state));

  s.append(" count=");
  s.append(std::to_string(cells_count));
  s.append(" offset=");
  s.append(std::to_string(m_cells_offset));

  s.append(" encoder=");
  s.append(Types::to_string(m_encoder));

  s.append(" enc/size=");
  s.append(std::to_string(m_size_enc));
  s.append("/");
  s.append(std::to_string(m_size));

  s.append(" ");
  s.append(interval.to_string());

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
  s.append(")");
  return s;
}


void Fragment::load_header(int& err, bool close_after) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();

  while(err != Error::FS_EOF) {
    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s", 
               err, Error::get_text(err), to_string().c_str());
      fs_if->close(err, m_smartfd);
      err = Error::OK;
    }

    if(!fs_if->exists(err, m_smartfd->filepath())) {
      err = Error::FS_PATH_NOT_FOUND;
      return;
    }

    if(!m_smartfd->valid() && !fs_if->open(err, m_smartfd) && err)
      return;
    if(err)
      continue;
    
    StaticBuffer buf;
    if(fs->pread(err, m_smartfd, 0, &buf, HEADER_SIZE) != HEADER_SIZE)
      continue;
    
    const uint8_t *ptr = buf.base;

    size_t remain = HEADER_SIZE;
    m_version = Serialization::decode_i8(&ptr, &remain);
    uint32_t header_extlen = Serialization::decode_i32(&ptr, &remain);
    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         buf.base, HEADER_SIZE-4)) {  
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }
    buf.free();
    
    if(fs->pread(err, m_smartfd, HEADER_SIZE, &buf, header_extlen) 
        != header_extlen)
      continue;

    ptr = buf.base;
    remain = header_extlen;

    interval.decode(&ptr, &remain, true);
    m_encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
    m_size_enc = Serialization::decode_i32(&ptr, &remain);
    m_size = Serialization::decode_i32(&ptr, &remain);
    m_cell_revs = Serialization::decode_i32(&ptr, &remain);
    m_cells_remain = cells_count = Serialization::decode_i32(&ptr, &remain);
    m_data_checksum = Serialization::decode_i32(&ptr, &remain);

    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         buf.base, header_extlen-4)) {  
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }
    m_cells_offset = HEADER_SIZE+header_extlen;
    break;
  }

  if(close_after) {
    int tmperr = Error::OK;
    fs_if->close(tmperr, m_smartfd);
  }
}

void Fragment::load() {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();

  bool header_again;
  {
    Mutex::scope lock(m_mutex);
    header_again = m_err;
  }
  int err = Error::OK;
  while(err != Error::FS_EOF) {
    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s", 
               err, Error::get_text(err), to_string().c_str());
      fs_if->close(err, m_smartfd);
      err = Error::OK;
    }

    if(header_again)
      load_header(err, false);
    else if(!fs_if->exists(err, m_smartfd->filepath()))
      err = Error::FS_PATH_NOT_FOUND;
    if(err)
      break;
 
    if(!m_smartfd->valid() && !fs_if->open(err, m_smartfd) && err)
      break;
    if(err)
      continue;
    
    m_buffer.free();
    if(fs->pread(err, m_smartfd, m_cells_offset, &m_buffer, m_size_enc) 
        != m_size_enc)
      continue;
    
    if(!checksum_i32_chk(m_data_checksum, m_buffer.base, m_size_enc)) {
      err = Error::CHECKSUM_MISMATCH;
      header_again = true;
      continue;
    }

    if(m_encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(m_size);
      Encoder::decode(
        err, m_encoder, m_buffer.base, m_size_enc, decoded_buf.base, m_size);
      if(err) {
        header_again = true;
        continue;
      }
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