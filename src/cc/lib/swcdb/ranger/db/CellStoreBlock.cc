/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStoreBlock.h"


namespace SWC { namespace Ranger { namespace CellStore { namespace Block {



const char* Read::to_string(const Read::State state) noexcept {
  switch(state) {
    case State::LOADED:
      return "LOADED";
    case State::LOADING:
      return "LOADING";
    default:
      return "NONE";
  }
}

/*
Read::Ptr Read::make(int& err,
                     CellStore::Read* cellstore,
                     const DB::Cells::Interval& interval,
                     const uint64_t offset) {
  Header header(interval.key_seq);
  header.interval.copy(interval);
  header.offset_data = offset;
  load_header(err, cellstore->smartfd, header);
  return err ? nullptr : new Read(cellstore, header);
}
*/

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

Read::Read(const Header& header)
          : header(header), cellstore(nullptr),
            m_state(header.size_plain ? State::NONE : State::LOADED),
            m_err(Error::OK), m_cells_remain(header.cells_count), 
            m_processing(0) {
  Env::Rgr::res().more_mem_usage(size_of());
}

void Read::init(CellStore::Read* _cellstore) {
  cellstore = _cellstore;
}

Read::~Read() { 
  Env::Rgr::res().less_mem_usage(
    size_of() + 
    (m_buffer.size && m_state == State::NONE ? 0 : header.size_plain)
  );
}

size_t Read::size_of() const noexcept {
  return sizeof(*this) + header.interval.size_of_internal();
}

bool Read::load(BlockLoader* loader) {
  m_processing.fetch_add(1);

  State at = State::NONE;
  if(m_state.compare_exchange_weak(at, State::LOADING))
    Env::Rgr::res().more_mem_usage(header.size_plain);
  if(at != State::LOADED && !loaded()) {
    Core::MutexSptd::scope lock(m_mutex);
    m_queue.push(loader);
    return at == State::NONE;
  }
  loader->loaded_blk();
  return false;
}

void Read::load() {
  Env::Rgr::post([this](){ load_open(Error::OK); });
}

void Read::load_cells(int&, Ranger::Block::Ptr cells_block) {
  bool was_splitted = false;
  ssize_t remain_hint = m_buffer.size
    ? m_cells_remain.sub_rslt(
        cells_block->load_cells(
          m_buffer.base, m_buffer.size,
          cellstore->cell_revs, header.cells_count,
          was_splitted,
          true
        )
      )
    : 0;

  if(m_processing.fetch_sub(1) == 1 &&
     !was_splitted && 
     (remain_hint <= 0 || Env::Rgr::res().need_ram(header.size_plain)))
    release();
}

SWC_SHOULD_INLINE
void Read::processing_decrement() noexcept {
  m_processing.fetch_sub(1);
}

size_t Read::release() {
  size_t released = 0;
  bool support;
  if(header.size_plain &&
     !m_processing &&
     m_mutex.try_full_lock(support)) {
    State at = State::LOADED;
    if(!m_processing && m_state.compare_exchange_weak(at, State::NONE)) {
      released += m_buffer.size;
      m_buffer.free();
      m_cells_remain.store(header.cells_count);
    }
    m_mutex.unlock(support);
  }
  if(released)
    Env::Rgr::res().less_mem_usage(header.size_plain);
  return released;
}

bool Read::processing() {
  bool support;
  bool busy = m_processing;
  if(!busy && !(busy = !m_mutex.try_full_lock(support))) {
    busy = m_processing;
    m_mutex.unlock(support);
  }
  return busy;
}

SWC_SHOULD_INLINE
int Read::error() const noexcept {
  return m_err;
}

SWC_SHOULD_INLINE
bool Read::loaded() const noexcept {
  return m_state == State::LOADED;
}

SWC_SHOULD_INLINE
bool Read::loaded(int& err) const noexcept {
  return !(err = error()) && loaded();
}

SWC_SHOULD_INLINE
size_t Read::size_bytes(bool only_loaded) const noexcept {
  return only_loaded && !loaded() ? 0 : header.size_plain;
}

SWC_SHOULD_INLINE
size_t Read::size_bytes_enc(bool only_loaded) const noexcept {
  return only_loaded && !loaded() ? 0 : header.size_enc;
}

void Read::print(std::ostream& out) {
  out << "Block(";
  header.print(out);
  out << " state="  << to_string(m_state)
      << " processing=" << m_processing.load();
  {
    Core::MutexSptd::scope lock(m_mutex);
    out << " queue="    << m_queue.size();
  }
  if(m_err)
    Error::print(out, m_err);
  out << ')';
}

void Read::load_open(int err) {
  switch(err) {
    case Error::FS_PATH_NOT_FOUND:
    case Error::SERVER_SHUTTING_DOWN:
      return load_finish(err);
    case Error::OK:
      break;
    default: {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        cellstore->smartfd->print(SWC_LOG_OSTREAM << ' ');
        print(SWC_LOG_OSTREAM << ' ');
      );
      if(cellstore->smartfd->valid()) {
        Env::FsInterface::interface()->close(
          [this](int, FS::SmartFd::Ptr) { load_open(Error::OK); },
          cellstore->smartfd
        );
        return;
      }
      //std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }
  }

  cellstore->smartfd->valid()
    ? Env::FsInterface::fs()->pread(
        [this](int err, FS::SmartFd::Ptr, const StaticBuffer::Ptr& buffer) {
          Env::Rgr::post([this, err, buffer](){ load_read(err, buffer); });
        },
        cellstore->smartfd, header.offset_data, header.size_enc
      )
    : Env::FsInterface::fs()->open(
        [this](int err, FS::SmartFd::Ptr) { load_open(err); },
        cellstore->smartfd
      );
}

void Read::load_read(int err, const StaticBuffer::Ptr& buffer) {
  if(!err) {
    if(!Core::checksum_i32_chk(header.checksum_data, 
                               buffer->base, header.size_enc)) {
      err = Error::CHECKSUM_MISMATCH;

    } else if(buffer->size != header.size_enc) {
      err = Error::FS_EOF;

    } else {
      if(header.encoder != DB::Types::Encoder::PLAIN) {
        StaticBuffer decoded_buf((size_t)header.size_plain);
        Core::Encoder::decode(
          err, header.encoder,
          buffer->base, header.size_enc,
          decoded_buf.base, header.size_plain
        );
        if(!err)
          m_buffer.set(decoded_buf);
      } else {
        m_buffer.set(*buffer.get());
      }
    }
  }

  err ? load_open(err) : load_finish(Error::OK);
}

void Read::load_finish(int err) {
  if(err) {
    SWC_LOG_OUT(LOG_ERROR,
      Error::print(SWC_LOG_OSTREAM << "CellStore::Block load ", err);
      cellstore->smartfd->print(SWC_LOG_OSTREAM << ' ');
      print(SWC_LOG_OSTREAM << ' ');
    );
    if(err == Error::FS_PATH_NOT_FOUND)
      err = Error::OK;

    m_buffer.free();
    Env::Rgr::res().less_mem_usage(header.size_plain);
  }

  m_err.store(err);
  m_state.store(err ? State::NONE : State::LOADED);

  Env::Rgr::post([this](){ cellstore->_run_queued(); });

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

