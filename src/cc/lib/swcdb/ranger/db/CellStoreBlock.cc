/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/CellStoreBlock.h"
#include "swcdb/core/Encoder.h"


namespace SWC { namespace Ranger { namespace CellStore { namespace Block {



std::string Read::to_string(const Read::State state) {
  switch(state) {
    case State::LOADED:
      return "LOADED";
    case State::LOADING:
      return "LOADING";
    default:
      return "NONE";
  }
}

Read::Ptr Read::make(const uint64_t offset, 
                     const DB::Cells::Interval& interval, 
                     uint32_t cell_revs) {
  return new Read(offset, interval, cell_revs);
}

Read::Read(const uint64_t offset, const DB::Cells::Interval& interval, 
           uint32_t cell_revs)
          : offset(offset), interval(interval), cell_revs(cell_revs),
            m_state(State::NONE), m_processing(0), 
            m_loaded_header(false),
            m_size(0), m_sz_enc(0), 
            m_cells_remain(0), m_cells_count(0), 
            m_checksum_data(0),
            m_err(Error::OK) {
}

Read::Ptr Read::ptr() {
  return this;
}

Read::~Read() { 
}

bool Read::load(const QueueRunnable::Call_t& cb) {
  {
    Mutex::scope lock(m_mutex);
    ++m_processing;
    if(m_state == State::NONE) {
      m_state = State::LOADING;
      return true;
    }
    if(m_state != State::LOADED) {
      m_queue.push(cb);
      return false;
    }
  }
  cb();
  return false;
}

void Read::load(FS::SmartFd::Ptr smartfd, const QueueRunnable::Call_t& cb) {
  int err = Error::OK;
  load(err, smartfd);
  if(err)
    SWC_LOGF(LOG_ERROR, "CellStore::Block load %s", to_string().c_str());

  cb();
  run_queued();
}

void Read::load_cells(int& err, Ranger::Block::Ptr cells_block) {
  bool was_splitted = false;
  if(m_buffer.size) {
    m_cells_remain -= cells_block->load_cells(
      m_buffer.base, m_buffer.size, 
      cell_revs, m_cells_count, 
      was_splitted,
      true
    );
  }

  processing_decrement();

  if(!was_splitted 
     && (!m_cells_remain.load() || Env::Resources.need_ram(m_size)))
    release();
}

void Read::processing_decrement() {
  Mutex::scope lock(m_mutex);
  --m_processing; 
}

size_t Read::release() {    
  size_t released = 0;
  Mutex::scope lock(m_mutex);

  if(m_processing || m_state != State::LOADED)
    return released; 

  released += m_buffer.size;    
  m_state = State::NONE;
  m_buffer.free();
  m_cells_remain = m_cells_count;
  return released;
}

bool Read::processing() {
  Mutex::scope lock(m_mutex);
  return m_processing;
}

int Read::error() {
  Mutex::scope lock(m_mutex);
  return m_err;
}

bool Read::loaded() {
  Mutex::scope lock(m_mutex);
  return m_state == State::LOADED;
}

bool Read::loaded(int& err) {
  Mutex::scope lock(m_mutex);
  err = m_err;
  return !err && m_state == State::LOADED;
}

size_t Read::size_bytes(bool only_loaded) {
  Mutex::scope lock(m_mutex);
  if(only_loaded && m_state != State::LOADED)
    return 0;
  return m_size;
}

std::string Read::to_string() {
  Mutex::scope lock(m_mutex);
  std::string s("Block(offset=");
  s.append(std::to_string(offset));
  s.append(" state=");
  s.append(to_string(m_state));
  if(m_size) {
    s.append(" encoder=");
    s.append(Types::to_string(m_encoder));
    s.append(" enc/size=");
    s.append(std::to_string(m_sz_enc));
    s.append("/");
    s.append(std::to_string(m_size));
  }
  s.append(" ");
  s.append(interval.to_string());
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

void Read::load(int& err, FS::SmartFd::Ptr smartfd) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();
  err = Error::OK;
  while(err != Error::FS_EOF) {
    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s %s", 
               err, Error::get_text(err), 
               smartfd->to_string().c_str(), to_string().c_str());
      fs_if->close(err, smartfd);
      err = Error::OK;
    }

    if(!smartfd->valid() && !fs_if->open(err, smartfd) && err)
      break;
    if(err)
      continue;
    
    if(!m_loaded_header) { // load header once
      uint8_t buf[HEADER_SIZE];
      const uint8_t *ptr = buf;
      if(fs->pread(err, smartfd, offset, buf, HEADER_SIZE) != HEADER_SIZE)
        continue;

      size_t remain = HEADER_SIZE;
      m_encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
      m_sz_enc = Serialization::decode_i32(&ptr, &remain);
      m_size = Serialization::decode_i32(&ptr, &remain);
      if(!m_sz_enc) 
        m_sz_enc = m_size;
      m_cells_remain = m_cells_count= Serialization::decode_i32(&ptr, &remain);
      m_checksum_data = Serialization::decode_i32(&ptr, &remain);
      if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), buf, HEADER_SIZE-4)) {
        err = Error::CHECKSUM_MISMATCH;
        continue;
      }
      m_loaded_header = true;
    }
    if(!m_sz_enc) // a zero cells type cs (initial of any to any block)
      break;


    m_buffer.free();
    if(fs->pread(err, smartfd, offset+HEADER_SIZE, &m_buffer, m_sz_enc) 
        != m_sz_enc)
      continue;
    
    if(!checksum_i32_chk(m_checksum_data, m_buffer.base, m_sz_enc)) {
      err = Error::CHECKSUM_MISMATCH;
      m_loaded_header = false;
      continue;
    }

    if(m_encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(m_size);
      Encoder::decode(
        err, m_encoder, m_buffer.base, m_sz_enc, decoded_buf.base, m_size);
      if(err) {
        m_loaded_header = false;
        continue;
      }
      m_buffer.set(decoded_buf);
    }
    break;
  }

  Mutex::scope lock(m_mutex);
  if((m_err = err) == Error::FS_PATH_NOT_FOUND) {
    m_err = Error::OK;
    m_buffer.free();
  }
  if(m_err) {
    m_state = State::NONE;
    m_loaded_header = false;
    m_buffer.free();
  } else {
    m_state = State::LOADED;
  }
}

void Read::run_queued() {
  if(m_queue.need_run())
    asio::post(
      *Env::IoCtx::io()->ptr(), 
      [this](){ m_queue.run(); }
    );
}



Write::Write(const uint64_t offset, const DB::Cells::Interval& interval, 
             const uint32_t cell_count)
            : offset(offset), interval(interval), cell_count(cell_count) { 
}

Write::~Write() {
}

void Write::write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
                  DynamicBuffer& output) {
  
  size_t len_enc = 0;
  output.set_mark();
  Encoder::encode(err, encoder, cells.base, cells.fill(), 
                  &len_enc, output, HEADER_SIZE);
  if(err)
    return;
                  
  uint8_t * ptr = output.mark;
  *ptr++ = (uint8_t)(len_enc? encoder: Types::Encoding::PLAIN);
  Serialization::encode_i32(&ptr, len_enc);
  Serialization::encode_i32(&ptr, cells.fill());
  Serialization::encode_i32(&ptr, cell_count);
  if(len_enc || (len_enc = cells.fill()))
    checksum_i32(output.base+HEADER_SIZE, len_enc, &ptr);
  else
    Serialization::encode_i32(&ptr, 0);

  checksum_i32(output.mark, ptr, &ptr);
}

std::string Write::to_string() {
  std::string s("Block(offset=");
  s.append(std::to_string(offset));
  s.append(" cell_count=");
  s.append(std::to_string(cell_count));
  s.append(" ");
  s.append(interval.to_string());
  s.append(")");
  return s;
}



}}}} //  namespace SWC::Ranger::CellStore::Block

