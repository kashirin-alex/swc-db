/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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
  Header header(interval.key_seq);
  header.interval.copy(interval);
  header.offset_data = offset;
  load_header(err, smartfd, header);  
  return err ? nullptr : new Read(cell_revs, header);
}

void Read::load_header(int& err, FS::SmartFd::Ptr& smartfd, 
                       Header& header) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();
  err = Error::OK;
  while(err != Error::FS_EOF) {

    if(err) {
      SWC_LOGF(LOG_WARN, "Retrying to err=%d(%s) %s blk-offset=%lu", 
        err, Error::get_text(err), smartfd->to_string().c_str(),
        header.offset_data);
      fs_if->close(err, smartfd);
      err = Error::OK;
    }

    if(!smartfd->valid() && !fs_if->open(err, smartfd) && err)
      return;
    if(err)
      continue;
    
    uint8_t buf[Header::SIZE];
    const uint8_t *ptr = buf;
    if(fs->pread(err, smartfd, header.offset_data, buf, 
                 Header::SIZE) != Header::SIZE)
      continue;

    size_t remain = Header::SIZE;
    header.decode(&ptr, &remain);
    if(!checksum_i32_chk(
      Serialization::decode_i32(&ptr, &remain), buf, Header::SIZE - 4)) {
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }
    header.offset_data += Header::SIZE;
    return;
  }
}

Read::Read(const uint32_t cell_revs, const Header& header)
          : cell_revs(cell_revs), header(header),
            m_state(State::NONE), m_processing(0),
            m_cells_remain(header.cells_count), m_err(Error::OK) {
  RangerEnv::res().more_mem_usage(size_of());
}

Read::~Read() { 
  RangerEnv::res().less_mem_usage(
    size_of() + 
    (m_buffer.size && m_state != State::NONE ? header.size_plain : 0)
  );
}

size_t Read::size_of() const {
  return sizeof(*this) + header.interval.size_of_internal();
}

bool Read::load(const QueueRunnable::Call_t& cb) {
  {
    Mutex::scope lock(m_mutex);
    ++m_processing;
    if(m_state == State::NONE) {
      if(header.size_enc) {
        m_state = State::LOADING;
        RangerEnv::res().more_mem_usage(header.size_plain);
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

void Read::load_cells(int&, Ranger::Block::Ptr cells_block) {
  bool was_splitted = false;
  if(m_buffer.size) {
    m_cells_remain -= cells_block->load_cells(
      m_buffer.base, m_buffer.size, 
      cell_revs, header.cells_count, 
      was_splitted,
      true
    );
  }

  processing_decrement();

  if(!was_splitted && 
     (!m_cells_remain || RangerEnv::res().need_ram(header.size_plain)))
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
      m_cells_remain = header.cells_count;
    }
    m_mutex.unlock(support);
  }
  if(released)
    RangerEnv::res().less_mem_usage(header.size_plain);
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
  return only_loaded && !loaded() ? 0 : header.size_plain;
}

size_t Read::size_bytes_enc(bool only_loaded) {
  return only_loaded && !loaded() ? 0 : header.size_enc;
}

std::string Read::to_string() {
  std::string s("Block(");
  s.append(header.to_string());
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
    if(fs->pread(err, smartfd, header.offset_data, &m_buffer, 
                 header.size_enc) != header.size_enc)
      continue;
    
    if(!checksum_i32_chk(header.checksum_data, 
                         m_buffer.base, header.size_enc)) {
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }

    if(header.encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(header.size_plain);
      Encoder::decode(
        err, header.encoder, 
        m_buffer.base, header.size_enc, 
        decoded_buf.base, header.size_plain
      );
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
    RangerEnv::res().less_mem_usage(header.size_plain);
  }
  if(m_err) {
    m_state = State::NONE;
    m_buffer.free();
    RangerEnv::res().less_mem_usage(header.size_plain);
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



Write::Write(const Header& header)
            : header(header), released(false) {
  RangerEnv::res().more_mem_usage(
    sizeof(Write::Ptr) + sizeof(Write)
    + header.interval.size_of_internal()
  );
}

Write::~Write() { 
  RangerEnv::res().less_mem_usage(
    sizeof(Write::Ptr) + sizeof(Write)
    + header.interval.size_of_internal()
    + (released ? 0 : header.size_enc)
  );
}

void Write::encode(int& err, DynamicBuffer& cells, DynamicBuffer& output, 
                   Header& header) {
  header.size_plain = cells.fill();
  size_t len_enc = 0;
  Encoder::encode(err, header.encoder, cells.base, header.size_plain, 
                  &len_enc, output, Header::SIZE);
  if(err)
    return;
  if(!len_enc) {
    header.encoder = Types::Encoding::PLAIN;
    header.size_enc = header.size_plain;
  } else {
    header.size_enc = len_enc;
  }
  RangerEnv::res().more_mem_usage(header.size_enc);

  uint8_t* ptr = output.base;
  header.encode(&ptr);
}

std::string Write::to_string() {
  std::string s("Block(");
  s.append(header.to_string());
  s.append(")");
  return s;
}



}}}} //  namespace SWC::Ranger::CellStore::Block

