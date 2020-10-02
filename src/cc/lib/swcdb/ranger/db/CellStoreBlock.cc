/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStoreBlock.h"


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
      SWC_LOG_OUT(LOG_WARN, 
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        smartfd->print(SWC_LOG_OSTREAM << ' ');
        SWC_LOG_OSTREAM << " blk-offset=" << header.offset_data;
      );
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
    if(!Core::checksum_i32_chk(
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
  Env::Rgr::res().more_mem_usage(size_of());
}

Read::~Read() { 
  Env::Rgr::res().less_mem_usage(
    size_of() + 
    (m_buffer.size && m_state != State::NONE ? header.size_plain : 0)
  );
}

size_t Read::size_of() const {
  return sizeof(*this) + header.interval.size_of_internal();
}

bool Read::load(BlockLoader* loader) {
  {
    Core::MutexSptd::scope lock(m_mutex);
    ++m_processing;
    if(m_state == State::NONE) {
      if(header.size_enc) {
        m_state = State::LOADING;
        Env::Rgr::res().more_mem_usage(header.size_plain);
        return true;
      } else {
        // a zero cells type cs (initial of any to any block)  
        m_state = State::LOADED;
      }
    }
    if(m_state != State::LOADED) {
      m_queue.push(loader);
      return false;
    }
  }
  loader->loaded_blk();
  return false;
}

void Read::load(FS::SmartFd::Ptr smartfd, BlockLoader* loader) {
  int err = Error::OK;
  _load(err, smartfd);
  if(err)
    SWC_LOG_OUT(LOG_ERROR, 
      print(SWC_LOG_OSTREAM << "CellStore::Block load ");
    );

  loader->loaded_blk();
  Env::IoCtx::post([this](){ _run_queued(); });
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
     (!m_cells_remain || Env::Rgr::res().need_ram(header.size_plain)))
    release();
}

void Read::processing_decrement() {
  Core::MutexSptd::scope lock(m_mutex);
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
    Env::Rgr::res().less_mem_usage(header.size_plain);
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
  Core::MutexSptd::scope lock(m_mutex);
  return m_err;
}

bool Read::loaded() {
  Core::MutexSptd::scope lock(m_mutex);
  return m_state == State::LOADED;
}

bool Read::loaded(int& err) {
  Core::MutexSptd::scope lock(m_mutex);
  err = m_err;
  return !err && m_state == State::LOADED;
}

size_t Read::size_bytes(bool only_loaded) {
  return only_loaded && !loaded() ? 0 : header.size_plain;
}

size_t Read::size_bytes_enc(bool only_loaded) {
  return only_loaded && !loaded() ? 0 : header.size_enc;
}

void Read::print(std::ostream& out) {
  out << "Block(";
  header.print(out);
  {
    Core::MutexSptd::scope lock(m_mutex);
    out << " state="      << to_string(m_state)
        << " queue="      << m_queue.size()
        << " processing=" << m_processing;
    if(m_err)
      Error::print(out, m_err);
  }
  out << ')';
}

void Read::_load(int& err, FS::SmartFd::Ptr smartfd) {
  auto fs_if = Env::FsInterface::interface();
  auto fs = Env::FsInterface::fs();
  err = Error::OK;
  while(err != Error::FS_PATH_NOT_FOUND &&
        err != Error::SERVER_SHUTTING_DOWN) {
    if(err) {
      SWC_LOG_OUT(LOG_WARN, 
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        smartfd->print(SWC_LOG_OSTREAM << ' ');
        print(SWC_LOG_OSTREAM << ' ');
      );
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
    
    if(!Core::checksum_i32_chk(header.checksum_data, 
                         m_buffer.base, header.size_enc)) {
      err = Error::CHECKSUM_MISMATCH;
      continue;
    }

    if(header.encoder != DB::Types::Encoder::PLAIN) {
      StaticBuffer decoded_buf((size_t)header.size_plain);
      Core::Encoder::decode(
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

  Core::MutexSptd::scope lock(m_mutex);
  if((m_err = err) == Error::FS_PATH_NOT_FOUND) {
    m_err = Error::OK;
    m_buffer.free();
    Env::Rgr::res().less_mem_usage(header.size_plain);
  }
  if(m_err) {
    m_state = State::NONE;
    m_buffer.free();
    Env::Rgr::res().less_mem_usage(header.size_plain);
  } else {
    m_state = State::LOADED;
  }
}

void Read::_run_queued() {
  for(BlockLoader* loader;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_queue.empty())
        return;
      loader = m_queue.front();
      m_queue.pop();
    }
    loader->loaded_blk();
  }
}



Write::Write(const Header& header)
            : header(header), released(false) {
  Env::Rgr::res().more_mem_usage(
    sizeof(Write::Ptr) + sizeof(Write)
    + header.interval.size_of_internal()
  );
}

Write::~Write() { 
  Env::Rgr::res().less_mem_usage(
    sizeof(Write::Ptr) + sizeof(Write)
    + header.interval.size_of_internal()
    + (released ? 0 : header.size_enc)
  );
}

void Write::encode(int& err, DynamicBuffer& cells, DynamicBuffer& output, 
                   Header& header) {
  header.size_plain = cells.fill();
  size_t len_enc = 0;
  Core::Encoder::encode(err, header.encoder, cells.base, header.size_plain, 
                        &len_enc, output, Header::SIZE);
  if(err)
    return;
  if(!len_enc) {
    header.encoder = DB::Types::Encoder::PLAIN;
    header.size_enc = header.size_plain;
  } else {
    header.size_enc = len_enc;
  }
  Env::Rgr::res().more_mem_usage(header.size_enc);

  uint8_t* ptr = output.base;
  header.encode(&ptr);
}

void Write::print(std::ostream& out) const {
  header.print(out << "Block(");
  out << ')';
}



}}}} //  namespace SWC::Ranger::CellStore::Block

