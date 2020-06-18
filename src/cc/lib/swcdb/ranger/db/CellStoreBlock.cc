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

Read::Ptr Read::make(int& err, FS::SmartFd::Ptr& smartfd, 
                     const DB::Cells::Interval& interval, 
                     const uint64_t offset, 
                     const uint32_t cell_revs) {
  Types::Encoding encoder;
  size_t size_plain;
  size_t size_enc; 
  uint32_t cells_count; 
  uint32_t checksum_data;
  
  load_header(
    err, smartfd, offset, 
    encoder, size_plain, size_enc, cells_count, checksum_data
  );
  
  return err ? nullptr : new Read(
    interval, offset + HEADER_SIZE, cell_revs, 
    encoder, size_plain, size_enc, cells_count, checksum_data
  );
}

void Read::load_header(int& err, FS::SmartFd::Ptr& smartfd, 
                       const uint64_t offset, Types::Encoding& encoder,
                       size_t& size_plain, size_t& size_enc, 
                       uint32_t& cells_count, uint32_t& checksum_data) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();
  err = Error::OK;
  while(err != Error::FS_EOF) {

    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s blk-offset=%lld", 
        err, Error::get_text(err), smartfd->to_string().c_str(), offset);
      fs_if->close(err, smartfd);
      err = Error::OK;
    }

    if(!smartfd->valid() && !fs_if->open(err, smartfd) && err)
      break;
    if(err)
      continue;
    
    uint8_t buf[HEADER_SIZE];
    const uint8_t *ptr = buf;
    if(fs->pread(err, smartfd, offset, buf, HEADER_SIZE) != HEADER_SIZE)
      continue;

    size_t remain = HEADER_SIZE;
    encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
    size_enc = Serialization::decode_i32(&ptr, &remain);
    size_plain = Serialization::decode_i32(&ptr, &remain);
    if(!size_enc) 
      size_enc = size_plain;
    cells_count = Serialization::decode_i32(&ptr, &remain);
    checksum_data = Serialization::decode_i32(&ptr, &remain);

    if(!checksum_i32_chk(
      Serialization::decode_i32(&ptr, &remain), buf, HEADER_SIZE-4)) {
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }
    break;
  }
}

Read::Read(const DB::Cells::Interval& interval, const uint64_t offset_data,
           const uint32_t cell_revs, const Types::Encoding encoder,
           const size_t size_plain, const size_t size_enc, 
           const uint32_t cells_count, const uint32_t checksum_data)
          : interval(interval), offset_data(offset_data), 
            cell_revs(cell_revs), encoder(encoder), 
            size_plain(size_plain), size_enc(size_enc), 
            cells_count(cells_count), checksum_data(checksum_data),
            m_state(State::NONE), m_processing(0),
            m_cells_remain(cells_count), m_err(Error::OK) {
}

Read::~Read() { 
}

bool Read::load(const QueueRunnable::Call_t& cb) {
  {
    Mutex::scope lock(m_mutex);
    ++m_processing;
    if(m_state == State::NONE) {
      if(size_enc) {
        m_state = State::LOADING;
        return true;
      } else {
        // a zero cells type cs (initial of any to any block)  
        m_state = State::LOADED;
      }
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
      cell_revs, cells_count, 
      was_splitted,
      true
    );
  }

  processing_decrement();

  if(!was_splitted && 
     (!m_cells_remain || Env::Resources.need_ram(size_plain)))
    release();
}

void Read::processing_decrement() {
  Mutex::scope lock(m_mutex);
  --m_processing; 
}

size_t Read::release() {    
  size_t released = 0;
  bool support;
  if(m_mutex.try_full_lock(support)) {
    if(!m_processing && m_state == State::LOADED) {
      released += m_buffer.size;
      m_state = State::NONE;
      m_buffer.free();
      m_cells_remain = cells_count;
    }
    m_mutex.unlock(support);
  }
  return released;
}

bool Read::processing() {
  bool support;
  bool busy;
  if(!(busy = !m_mutex.try_full_lock(support))) {
    busy = m_processing;
    m_mutex.unlock(support);
  }
  return busy;
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
  return only_loaded && !loaded() ? 0 : size_plain;
}

std::string Read::to_string() {
  std::string s("Block(offset=");
  s.append(std::to_string(offset_data));
  if(size_plain) {
    s.append(" encoder=");
    s.append(Types::to_string(encoder));
    s.append(" enc/size=");
    s.append(std::to_string(size_enc));
    s.append("/");
    s.append(std::to_string(size_plain));
  }
  {
    Mutex::scope lock(m_mutex);
    s.append(" state=");
    s.append(to_string(m_state));
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

void Read::load(int& err, FS::SmartFd::Ptr smartfd) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();
  err = Error::OK;
  while(err != Error::FS_PATH_NOT_FOUND &&
        err != Error::SERVER_SHUTTING_DOWN) {
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

    m_buffer.free();
    if(fs->pread(err, smartfd, offset_data, &m_buffer, size_enc) 
        != size_enc)
      continue;
    
    if(!checksum_i32_chk(checksum_data, m_buffer.base, size_enc)) {
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

  Mutex::scope lock(m_mutex);
  if((m_err = err) == Error::FS_PATH_NOT_FOUND) {
    m_err = Error::OK;
    m_buffer.free();
  }
  if(m_err) {
    m_state = State::NONE;
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



Write::Write(const uint64_t offset, const DB::Cells::Interval& interval)
            : offset(offset), interval(interval) { 
}

Write::~Write() { }

void Write::encode(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
                   DynamicBuffer& output, const uint32_t cell_count) {
  size_t len_enc = 0;
  Encoder::encode(err, encoder, cells.base, cells.fill(), 
                  &len_enc, output, HEADER_SIZE);
  if(err)
    return;
  if(!len_enc)
    encoder = Types::Encoding::PLAIN;

  uint8_t * ptr = output.base;
  size_t data_fill = output.fill() - HEADER_SIZE;
  
  *ptr = (uint8_t)encoder;
  Serialization::encode_i32(&++ptr, len_enc);
  Serialization::encode_i32(&ptr, cells.fill());
  Serialization::encode_i32(&ptr, cell_count);
  if(data_fill)
    checksum_i32(output.base + HEADER_SIZE, data_fill, &ptr);
  else
    Serialization::encode_i32(&ptr, 0);

  checksum_i32(output.base, ptr, &ptr);
}

std::string Write::to_string() {
  std::string s("Block(offset=");
  s.append(std::to_string(offset));
  s.append(" ");
  s.append(interval.to_string());
  s.append(")");
  return s;
}



}}}} //  namespace SWC::Ranger::CellStore::Block

