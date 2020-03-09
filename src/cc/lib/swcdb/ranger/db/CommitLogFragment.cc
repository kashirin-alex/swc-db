/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/Encoder.h"
#include "swcdb/ranger/db/CommitLogFragment.h"

namespace SWC { namespace Ranger { namespace CommitLog {


const std::string Fragment::to_string(Fragment::State state) {
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

Fragment::Ptr Fragment::make(const std::string& filepath, Fragment::State state) {
  return new Fragment(filepath, state);
}


Fragment::Fragment(const std::string& filepath, Fragment::State state)
                  : ts(Time::now_ns()),
                    m_smartfd(
                    FS::SmartFd::make_ptr(
                      filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE)
                    ), 
                    m_state(state), 
                    m_size_enc(0), m_size(0), 
                    m_cell_revs(0), m_cells_count(0), m_cells_offset(0), 
                    m_data_checksum(0), m_processing(0), m_cells_remain(0),
                    m_err(Error::OK) {
}

Fragment::Ptr Fragment::ptr() {
  return this;
}

Fragment::~Fragment() { }

void Fragment::write(int& err, uint8_t blk_replicas, 
                     Types::Encoding encoder, 
                     DynamicBuffer& cells, 
                     uint32_t cell_revs, uint32_t cell_count, 
                     std::atomic<int>& writing, 
                     std::condition_variable_any& cv) {

  m_version = VERSION;
  
  uint32_t header_extlen = interval.encoded_length()+HEADER_EXT_FIXED_SIZE;
  m_cell_revs = cell_revs;
  m_cells_remain = m_cells_count = cell_count;
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
  Serialization::encode_i32(&bufp, m_cells_count);
  
  checksum_i32(output.base+m_cells_offset, output.base+output.fill(), 
               &bufp, m_data_checksum);
  checksum_i32(header_extptr, bufp, &bufp);

  auto buff_write = std::make_shared<StaticBuffer>(output);
  buff_write->own = false;
  m_buffer.set(cells);
  ++writing;
  
  write(
    Error::UNPOSSIBLE, 
    m_smartfd, blk_replicas, m_cells_offset+m_size_enc, 
    buff_write,
    writing,
    cv
    );
}

void Fragment::write(int err, FS::SmartFd::Ptr smartfd, 
                     uint8_t blk_replicas, int64_t blksz, 
                     StaticBuffer::Ptr buff_write,
                     std::atomic<int>& writing, 
                     std::condition_variable_any& cv) {
  
  int tmperr = Error::OK;
  if(!err && Env::FsInterface::fs()->length(
      tmperr, m_smartfd->filepath()) != buff_write->size)
    err = Error::FS_EOF;

  if(err & err != Error::FS_PATH_NOT_FOUND &&
           err != Error::FS_PERMISSION_DENIED &&
           err != Error::SERVER_SHUTTING_DOWN) {
    
    if(err != Error::UNPOSSIBLE)
      SWC_LOGF(LOG_DEBUG, "write, retrying to err=%d(%s)", 
               err, Error::get_text(err));

    Env::FsInterface::fs()->write(
      [this, blk_replicas, blksz, buff_write, &writing, &cv]
      (int err, FS::SmartFd::Ptr smartfd) {
        write(err, smartfd, blk_replicas, blksz, buff_write, writing, cv);
      }, 
      smartfd, blk_replicas, blksz, *buff_write.get()
    );
    return;
  }

  //if(err) write local dump
    
  buff_write->own = true;

  bool keep;
  {
    std::scoped_lock lock(m_mutex);
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
  
  --writing;
  cv.notify_all();
}

void Fragment::load_header(bool close_after) {
  m_err = Error::OK;
  load_header(m_err, close_after);
  if(m_err)
    SWC_LOGF(LOG_ERROR, "CommitLog::Fragment load_header %s", 
             to_string().c_str());
}

void Fragment::load(const std::function<void()>& cb) {
  bool loaded;
  {
    std::scoped_lock lock(m_mutex);
    ++m_processing;
    loaded = m_state == State::LOADED;
    if(!loaded) {
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
      m_cell_revs, m_cells_count, 
      was_splitted
      ); 
  } else {
    SWC_LOGF(LOG_WARN, "Fragment::load_cells empty buf %s", 
             to_string().c_str());
  }
  
  processing_decrement();

  if(!was_splitted 
     && (!m_cells_remain.load() || Env::Resources.need_ram(m_size)))
    release();
}

void Fragment::processing_decrement() {
  std::scoped_lock lock(m_mutex);
  --m_processing; 
}

size_t Fragment::release() {
  size_t released = 0;     
  std::scoped_lock lock(m_mutex);

  if(m_processing || m_state != State::LOADED)
    return released; 
 
  m_state = State::NONE;
  released += m_buffer.size;   
  m_buffer.free();
  m_cells_remain = m_cells_count;
  return released;
}

const bool Fragment::loaded() {
  std::shared_lock lock(m_mutex);
  return m_state == State::LOADED;
}

const int Fragment::error() {
  std::shared_lock lock(m_mutex);
  return m_err;
}

const bool Fragment::loaded(int& err) {
  std::shared_lock lock(m_mutex);
  err = m_err;
  return !err && m_state == State::LOADED;
}

const uint32_t Fragment::cells_count() {
  std::shared_lock lock(m_mutex);
  return m_cells_count;
}

const size_t Fragment::size_bytes(bool only_loaded) {
  std::shared_lock lock(m_mutex);
  if(only_loaded && m_state != State::LOADED)
    return 0;
  return m_size;
}

const size_t Fragment::size_bytes_encoded() {
  std::shared_lock lock(m_mutex);
  return m_size_enc;
}

const bool Fragment::processing() {
  std::shared_lock lock(m_mutex);
  return m_processing;
}

void Fragment::remove(int &err) {
  std::scoped_lock lock(m_mutex);
  Env::FsInterface::fs()->remove(err, m_smartfd->filepath()); 
}

const std::string Fragment::to_string() {
  std::shared_lock lock(m_mutex);
  std::string s("Fragment(version=");
  s.append(std::to_string(m_version));

  s.append(" state=");
  s.append(to_string(m_state));

  s.append(" count=");
  s.append(std::to_string(m_cells_count));
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
  
  for(;;) {
    err = Error::OK;
  
    if(!Env::FsInterface::fs()->exists(err, m_smartfd->filepath())) {
      if(err != Error::OK && err != Error::SERVER_SHUTTING_DOWN)
        continue;
      return;
    }
    
    if(!Env::FsInterface::interface()->open(err, m_smartfd) && err)
      return;
    if(err)
      continue;
    
    StaticBuffer buf;
    if(Env::FsInterface::fs()->pread(
        err, m_smartfd, 0, &buf, HEADER_SIZE) != HEADER_SIZE){
      if(err != Error::FS_EOF){
        Env::FsInterface::fs()->close(err, m_smartfd);
        continue;
      }
      return;
    }
    const uint8_t *ptr = buf.base;

    size_t remain = HEADER_SIZE;
    m_version = Serialization::decode_i8(&ptr, &remain);
    uint32_t header_extlen = Serialization::decode_i32(&ptr, &remain);
    if(!checksum_i32_chk(
      Serialization::decode_i32(&ptr, &remain), buf.base, HEADER_SIZE-4)){  
      err = Error::CHECKSUM_MISMATCH;
      return;
    }
    buf.free();
    
    if(Env::FsInterface::fs()->pread(err, m_smartfd, HEADER_SIZE, 
                                     &buf, header_extlen) != header_extlen) {
      if(err != Error::FS_EOF){
        Env::FsInterface::fs()->close(err, m_smartfd);
        continue;
      }
      return;
    }
    ptr = buf.base;

    remain = header_extlen;
    interval.decode(&ptr, &remain, true);
    m_encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
    m_size_enc = Serialization::decode_i32(&ptr, &remain);
    m_size = Serialization::decode_i32(&ptr, &remain);
    m_cell_revs = Serialization::decode_i32(&ptr, &remain);
    m_cells_remain=m_cells_count = Serialization::decode_i32(&ptr, &remain);
    m_data_checksum = Serialization::decode_i32(&ptr, &remain);

    if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                         buf.base, header_extlen-4)) {  
      err = Error::CHECKSUM_MISMATCH;
      return;
    }
    m_cells_offset = HEADER_SIZE+header_extlen;
    break;
  }

  if(close_after && m_smartfd->valid()) {
    int tmperr = Error::OK;
    Env::FsInterface::fs()->close(tmperr, m_smartfd);
  }
}

void Fragment::load() {
  int err = Error::OK;
  {
    std::shared_lock lock(m_mutex);
    err = m_err;
  }
  if(err) // on err load header again
      load_header(err, false);
 
  if(!err) for(uint8_t tries = 0; ;err = Error::OK) {
  
    if(!m_smartfd->valid() 
       && !Env::FsInterface::interface()->open(err, m_smartfd) && err)
      break;
    if(err)
      continue;
    
    m_buffer.free();
    if(Env::FsInterface::fs()->pread(
          err, m_smartfd, m_cells_offset, &m_buffer, m_size_enc) 
        != m_size_enc) {
      err = Error::FS_IO_ERROR;
    }
    if(m_smartfd->valid()) {
      int tmperr = Error::OK;
      Env::FsInterface::fs()->close(tmperr, m_smartfd);
    }
    if(err) {
      if(err == Error::FS_EOF)
        break;
      continue;
    }
    
    if(!checksum_i32_chk(m_data_checksum, m_buffer.base, m_size_enc)){  
      if(++tries == 3) {
        SWC_LOGF(LOG_WARN, "CHECKSUM_MISMATCH try=%d %s", 
                  tries, m_smartfd->to_string().c_str());
        err = Error::CHECKSUM_MISMATCH;
        break;
      }
      continue;
    }

    if(m_encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(m_size);
      Encoder::decode(
        err, m_encoder, m_buffer.base, m_size_enc, decoded_buf.base, m_size);
      if(err) {
        int tmperr = Error::OK;
        Env::FsInterface::fs()->close(tmperr, m_smartfd);
        break;
      }
      m_buffer.set(decoded_buf);
    }
    break;
  }
  
  {
    std::scoped_lock lock(m_mutex);
    m_err = err;
    m_state = m_err ? State::NONE : State::LOADED;
  }
  if(err)
    SWC_LOGF(LOG_ERROR, "CommitLog::Fragment load %s", to_string().c_str());

  run_queued();
}


void Fragment::run_queued() {
  {
    std::scoped_lock lock(m_mutex);
    if(m_q_runs || m_queue.empty())
      return;
    m_q_runs = true;
  }
  
  asio::post(
    *Env::IoCtx::io()->ptr(), 
    [ptr=ptr()](){ ptr->_run_queued(); }
  );
}

void Fragment::_run_queued() {
  std::function<void()> cb;
  for(;;) {
    {
      std::shared_lock lock(m_mutex);
      cb = m_queue.front();
    }

    cb();
    
    {
      std::scoped_lock lock(m_mutex);
      m_queue.pop();
      if(m_queue.empty()) {
        m_q_runs = false;
        return;
      }
    }
  }
}


}}} // namespace SWC::Ranger::CommitLog