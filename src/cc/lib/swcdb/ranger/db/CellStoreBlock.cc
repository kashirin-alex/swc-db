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
  const auto& fs_if = Env::FsInterface::interface();
  const auto& fs = Env::FsInterface::fs();
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

SWC_CAN_INLINE
Read::Read(Header&& a_header) noexcept
          : header(std::move(a_header)), cellstore(nullptr),
            m_state(header.size_plain ? State::NONE : State::LOADED),
            m_err(Error::OK), m_cells_remain(header.cells_count),
            m_processing(0) {
}

SWC_CAN_INLINE
void Read::init(CellStore::Read* _cellstore) noexcept {
  cellstore = _cellstore;
}

Read::~Read() noexcept {
  if(m_state == State::LOADED)
    Env::Rgr::res().less_mem_releasable(header.size_plain);
}

/*
size_t Read::size_of() const noexcept {
  return sizeof(*this) + sizeof(Ptr) +
         header.interval.size_of_internal();
}
*/

SWC_CAN_INLINE
bool Read::load(BlockLoader* loader) {
  m_processing.fetch_add(1);
  auto at(State::NONE);
  {
    Core::MutexSptd::scope lock(m_mutex);
    if(m_state.compare_exchange_weak(at, State::LOADING) ||
       at == State::LOADING)
      m_queue.push(loader);
  }
  switch(at) {
    case State::NONE: {
      //Env::Rgr::res().more_mem_future(header.size_plain);
      Env::Rgr::res().more_mem_releasable(header.size_plain);
      return true;
    }
    case State::LOADING:
      return false;
    default: //case State::LOADED:
      loader->loaded_blk();
      return false;
  }
}

SWC_CAN_INLINE
void Read::load() {
  struct Task {
    Read* ptr;
    SWC_CAN_INLINE
    Task(Read* a_ptr) noexcept : ptr(a_ptr) { }
    void operator()() { ptr->load_open(Error::OK); }
  };
  Env::Rgr::post(Task(this));
}

SWC_CAN_INLINE
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

SWC_CAN_INLINE
void Read::processing_decrement() noexcept {
  m_processing.fetch_sub(1);
}

SWC_CAN_INLINE
size_t Read::release() {
  size_t released = 0;
  bool support;
  if(header.size_plain && !m_processing && loaded() &&
     m_mutex.try_full_lock(support)) {
    State at = State::LOADED;
    if(m_queue.empty() && !m_processing &&
       m_state.compare_exchange_weak(at, State::NONE)) {
      released += header.size_plain;
      m_buffer.free();
      m_cells_remain.store(header.cells_count);
    }
    m_mutex.unlock(support);
  }
  if(released)
    Env::Rgr::res().less_mem_releasable(header.size_plain);
  return released;
}

SWC_CAN_INLINE
bool Read::processing() noexcept {
  bool support;
  bool busy = m_processing ||
              m_state == State::LOADING ||
              !m_mutex.try_full_lock(support);
  if(!busy) {
    busy = m_processing ||
           m_state == State::LOADING ||
           !m_queue.empty();
    m_mutex.unlock(support);
  }
  return busy;
}


SWC_CAN_INLINE
bool Read::loaded() const noexcept {
  return m_state == State::LOADED;
}

SWC_CAN_INLINE
bool Read::loaded(int& err) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return !(err = m_err) && loaded();
}

SWC_CAN_INLINE
size_t Read::size_bytes(bool only_loaded) const noexcept {
  return only_loaded && !loaded() ? 0 : header.size_plain;
}

SWC_CAN_INLINE
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
    if(m_err)
      Error::print(out << ' ', m_err);
  }
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
          [this](int) { load_open(Error::OK); },
          cellstore->smartfd
        );
        return;
      }
      //std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }
  }

  cellstore->smartfd->valid()
    ? Env::FsInterface::fs()->pread(
        [ptr=this](int _err, const StaticBuffer::Ptr& buff) {
          struct Task {
            Read*             ptr;
            int               error;
            StaticBuffer::Ptr buffer;
            SWC_CAN_INLINE
            Task(Read* a_ptr, int a_error, const StaticBuffer::Ptr& a_buffer)
                noexcept : ptr(a_ptr), error(a_error), buffer(a_buffer) { }
            ~Task() noexcept { }
            void operator()() { ptr->load_read(error, buffer); }
          };
          Env::Rgr::post(Task(ptr, _err, buff));
        },
        cellstore->smartfd, header.offset_data, header.size_enc
      )
    : Env::FsInterface::fs()->open(
        [this](int _err) { load_open(_err); },
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
        StaticBuffer decoded_buf(static_cast<size_t>(header.size_plain));
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
  //Env::Rgr::res().less_mem_future(header.size_plain);
  if(err) {
    SWC_LOG_OUT(LOG_ERROR,
      Error::print(SWC_LOG_OSTREAM << "CellStore::Block load ", err);
      cellstore->smartfd->print(SWC_LOG_OSTREAM << ' ');
      print(SWC_LOG_OSTREAM << ' ');
    );
    if(err == Error::FS_PATH_NOT_FOUND)
      err = Error::OK;

    m_buffer.free();
  }

  {
    Core::MutexSptd::scope lock(m_mutex);
    m_state.store(err ? State::NONE : State::LOADED);
    m_err = err;
  }
  if(err)
    Env::Rgr::res().less_mem_releasable(header.size_plain);

  struct Task {
    Read* ptr;
    SWC_CAN_INLINE
    Task(Read* a_ptr) noexcept : ptr(a_ptr) { }
    void operator()() { ptr->cellstore->_run_queued(); }
  };
  Env::Rgr::post(Task(this));

  for(BlockLoader* loader;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_queue.empty())
        break;
      loader = m_queue.front();
      m_queue.pop();
    }
    loader->loaded_blk();
  }
}





SWC_CAN_INLINE
Write::Write(Header&& a_header) noexcept : header(std::move(a_header)) { }

SWC_CAN_INLINE
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

  uint8_t* ptr = output.base;
  header.encode(&ptr);
}

void Write::print(std::ostream& out) const {
  header.print(out << "Block(");
  out << ')';
}



}}}} //  namespace SWC::Ranger::CellStore::Block

