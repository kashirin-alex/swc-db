/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CommitLogFragment.h"

namespace SWC { namespace Ranger { namespace CommitLog {


const char* Fragment::to_string(Fragment::State state) noexcept {
  switch(state) {
    case State::NONE:
      return "NONE";
    case State::LOADING:
      return "LOADING";
    case State::LOADED:
      return "LOADED";
    case State::WRITING:
      return "WRITING";
    default:
      return "UKNOWN";
  }
}


SWC_CAN_INLINE
Fragment::Ptr Fragment::make_read(int& err, std::string&& filepath,
                                  const DB::Types::KeySeq key_seq) {
  auto smartfd = FS::SmartFd::make_ptr(std::move(filepath), 0);
  return make_read(err, smartfd, key_seq);
}

Fragment::Ptr Fragment::make_read(int& err, FS::SmartFd::Ptr& smartfd,
                                  const DB::Types::KeySeq key_seq) {
  uint8_t                     version = 0;
  DB::Cells::Interval         interval(key_seq);
  DB::Types::Encoder          encoder = DB::Types::Encoder::UNKNOWN;
  size_t                      size_plain = 0;
  size_t                      size_enc = 0;
  uint32_t                    cell_revs = 0;
  uint32_t                    cells_count = 0;
  uint32_t                    data_checksum = 0;
  uint32_t                    offset_data = 0;

  load_header(
    err, smartfd,
    version, interval,
    encoder, size_plain, size_enc,
    cell_revs, cells_count, data_checksum, offset_data
  );

  return Fragment::Ptr(err ? nullptr : new Fragment(
    smartfd,
    version, std::move(interval),
    encoder, size_plain, size_enc,
    cell_revs, cells_count, data_checksum, offset_data,
    State::NONE
  ));
}

SWC_CAN_INLINE
void Fragment::load_header(int& err, FS::SmartFd::Ptr& smartfd,
                           uint8_t& version,
                           DB::Cells::Interval& interval,
                           DB::Types::Encoder& encoder,
                           size_t& size_plain, size_t& size_enc,
                           uint32_t& cell_revs, uint32_t& cells_count,
                           uint32_t& data_checksum, uint32_t& offset_data) {
  const auto& fs_if = Env::FsInterface::interface();
  const auto& fs = Env::FsInterface::fs();

  while(err != Error::FS_PATH_NOT_FOUND &&
        err != Error::SERVER_SHUTTING_DOWN) {
    if(err) {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        smartfd->print(SWC_LOG_OSTREAM);
      );
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
    if(!Core::checksum_i32_chk(Serialization::decode_i32(&ptr, &remain),
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
    encoder = DB::Types::Encoder(Serialization::decode_i8(&ptr, &remain));
    size_enc = Serialization::decode_i32(&ptr, &remain);
    size_plain = Serialization::decode_i32(&ptr, &remain);
    cell_revs = Serialization::decode_i32(&ptr, &remain);
    cells_count = Serialization::decode_i32(&ptr, &remain);
    data_checksum = Serialization::decode_i32(&ptr, &remain);

    if(!Core::checksum_i32_chk(Serialization::decode_i32(&ptr, &remain),
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


Fragment::Ptr Fragment::make_write(int& err, std::string&& filepath,
                                   DB::Cells::Interval&& interval,
                                   DB::Types::Encoder encoder,
                                   const uint32_t cell_revs,
                                   const uint32_t cells_count,
                                   DynamicBuffer& cells,
                                   StaticBuffer::Ptr& buffer) {
  auto smartfd = FS::SmartFd::make_ptr(
    std::move(filepath),
    FS::OpenFlags::OPEN_FLAG_OVERWRITE | FS::OpenFlags::WRITE_VALIDATE_LENGTH
  );

  const uint8_t version = VERSION;
  const size_t  size_plain = cells.fill();
  size_t        size_enc = 0;
  uint32_t      data_checksum = 0;
  uint32_t      offset_data = 0;

  write(
    err,
    version, interval,
    encoder, size_plain, size_enc,
    cell_revs, cells_count, data_checksum, offset_data,
    cells, buffer
  );
  if(err)
    return nullptr;

  auto frag = new Fragment(
    smartfd,
    version, std::move(interval),
    encoder, size_plain, size_enc,
    cell_revs, cells_count, data_checksum, offset_data,
    State::WRITING
  );
  frag->m_buffer.set(cells);
  return Fragment::Ptr(frag);
}

SWC_CAN_INLINE
void Fragment::write(int& err,
                     const uint8_t version,
                     const DB::Cells::Interval& interval,
                     DB::Types::Encoder& encoder,
                     const size_t size_plain, size_t& size_enc,
                     const uint32_t cell_revs, const uint32_t cells_count,
                     uint32_t& data_checksum, uint32_t& offset_data,
                     DynamicBuffer& cells, StaticBuffer::Ptr& buffer) {
  uint32_t header_extlen = interval.encoded_length()+HEADER_EXT_FIXED_SIZE;
  offset_data += HEADER_SIZE;
  offset_data += header_extlen;

  DynamicBuffer output;
  err = Error::OK;
  Core::Encoder::encode(err, encoder, cells.base, size_plain,
                        &size_enc, output, offset_data);
  if(err)
    return;

  if(!size_enc) {
    size_enc = size_plain;
    encoder = DB::Types::Encoder::PLAIN;
  }

  uint8_t * bufp = output.base;
  Serialization::encode_i8(&bufp, version);
  Serialization::encode_i32(&bufp, header_extlen);
  Core::checksum_i32(output.base, bufp, &bufp);

  uint8_t * header_extptr = bufp;
  interval.encode(&bufp);
  Serialization::encode_i8(&bufp, uint8_t(encoder));
  Serialization::encode_i32(&bufp, size_enc);
  Serialization::encode_i32(&bufp, size_plain);
  Serialization::encode_i32(&bufp, cell_revs);
  Serialization::encode_i32(&bufp, cells_count);

  Core::checksum_i32(output.base+offset_data, output.base+output.fill(),
                     &bufp, data_checksum);
  Core::checksum_i32(header_extptr, bufp, &bufp);

  buffer->set(output);
}


SWC_CAN_INLINE
Fragment::Fragment(const FS::SmartFd::Ptr& smartfd,
                   const uint8_t a_version,
                   DB::Cells::Interval&& a_interval,
                   const DB::Types::Encoder a_encoder,
                   const size_t a_size_plain, const size_t a_size_enc,
                   const uint32_t a_cell_revs,  const uint32_t a_cells_count,
                   const uint32_t a_data_checksum,
                   const uint32_t a_offset_data,
                   Fragment::State state) noexcept
                  : version(a_version), interval(std::move(a_interval)),
                    encoder(a_encoder),
                    size_plain(a_size_plain), size_enc(a_size_enc),
                    cell_revs(a_cell_revs), cells_count(a_cells_count),
                    data_checksum(a_data_checksum),
                    offset_data(a_offset_data),
                    m_mutex(),
                    m_state(state), m_marked_removed(false), m_err(Error::OK),
                    m_processing(m_state == State::WRITING),
                    m_cells_remain(cells_count),
                    m_smartfd(smartfd), m_buffer(), m_queue() {
  if(m_state != State::NONE)
    Env::Rgr::res().more_mem_releasable(size_plain);
}

SWC_CAN_INLINE
Fragment::~Fragment() noexcept {
  if(m_state == State::LOADED && !m_marked_removed)
    Env::Rgr::res().less_mem_releasable(size_plain);
}

SWC_CAN_INLINE
Fragment::Ptr Fragment::ptr() {
  return shared_from_this();
}

SWC_CAN_INLINE
const std::string& Fragment::get_filepath() const noexcept {
  return m_smartfd->filepath();
}

void Fragment::write(int err, uint8_t blk_replicas,
                     const StaticBuffer::Ptr& buff_write,
                     Core::Semaphore* sem) {
  if(err && err != Error::SERVER_SHUTTING_DOWN) {
    if(err != Error::UNPOSSIBLE)
      SWC_LOG_OUT(LOG_WARN,
        Error::print(SWC_LOG_OSTREAM << "Retrying write to ", err);
        print(SWC_LOG_OSTREAM << ' ');
      );

    Env::FsInterface::fs()->write(
      [frag=ptr(), blk_replicas, buff_write, sem] (int _err) mutable {
        struct Task {
          Ptr               frag;
          StaticBuffer::Ptr buff_write;
          Core::Semaphore*  sem;
          int               error;
          uint8_t           blk_replicas;
          SWC_CAN_INLINE
          Task(Ptr&& a_frag, uint8_t a_blk_replicas,
               const StaticBuffer::Ptr& a_buff_write,
               Core::Semaphore* a_sem, int a_error) noexcept
               : frag(std::move(a_frag)),
                 buff_write(a_buff_write), sem(a_sem),
                 error(a_error),
                 blk_replicas(a_blk_replicas) {
          }
          SWC_CAN_INLINE
          Task(Task&&) = default;
          Task(const Task&) = delete;
          Task& operator=(const Task&) = delete;
          Task& operator=(Task&&) = delete;
          ~Task() noexcept { }
          void operator()() {
            frag->write(error, blk_replicas, buff_write, sem);
          }
        };
        Env::Rgr::post(
          Task(std::move(frag), blk_replicas, buff_write, sem, _err));
      },
      m_smartfd, blk_replicas, *buff_write.get()
    );
    return;
  }

  //if(err) remains Error::SERVER_SHUTTING_DOWN -- write local dump

  m_smartfd->flags(0);
  sem->release();
  buff_write->own = true;

  if(err) {
    m_buffer.free();
    Env::Rgr::res().less_mem_releasable(size_plain);
  }

  bool keep;
  {
    Core::MutexSptd::scope lock(m_mutex);
    keep = m_processing.fetch_sub(1) > 1 || !m_queue.empty();
    m_state.store(err ? State::NONE : State::LOADED);
    m_err = err;
  }

  if(keep) {
    struct Task {
      Ptr frag;
      SWC_CAN_INLINE
      Task(Ptr&& a_frag) noexcept : frag(std::move(a_frag)) { }
      ~Task() noexcept { }
      void operator()() { frag->run_queued(); }
    };
    Env::Rgr::post(Task(ptr()));
  } else {
    release();
  }
}

struct Fragment::TaskLoadRead {
  Ptr               frag;
  int               err;
  StaticBuffer::Ptr buffer;
  SWC_CAN_INLINE
  TaskLoadRead(Ptr&& a_frag, int a_err,
               const StaticBuffer::Ptr& a_buffer) noexcept
              : frag(std::move(a_frag)), err(a_err), buffer(a_buffer) { }
  ~TaskLoadRead() noexcept { }
  void operator()() { frag->load_read(err, buffer); }
};

void Fragment::load(Fragment::LoadCallback* cb) {
  auto at(State::NONE);
  {
    Core::MutexSptd::scope lock(m_mutex);
    m_state.compare_exchange_weak(at, State::LOADING);
    if(at != State::LOADED)
      m_queue.push(cb);
  }
  switch(at) {
    case State::NONE: {
      Env::Rgr::res().more_mem_future(size_plain);
      Env::Rgr::res().more_mem_releasable(size_plain);
      struct Task {
        Ptr frag;
        SWC_CAN_INLINE
        Task(Ptr&& a_frag) noexcept : frag(std::move(a_frag)) { }
        ~Task() noexcept { }
        void operator()() {
          Env::FsInterface::fs()->combi_pread(
            [frag=frag] (int err, const StaticBuffer::Ptr& buffer) mutable {
              Env::Rgr::post(TaskLoadRead(std::move(frag), err, buffer));
            },
            frag->m_smartfd, frag->offset_data, frag->size_enc
          );
        }
      };
      Env::Rgr::post(Task(ptr()));
      return;
    }
    case State::WRITING:
    case State::LOADING:
      return;
    default: // case State::LOADED:
      return cb->loaded(ptr());
  }
}

SWC_CAN_INLINE
void Fragment::load_cells(int&, Ranger::Block::Ptr cells_block) {
  ssize_t remain_hint(0);
  if(!marked_removed()) {
    if(m_buffer.size) {
      bool was_splitted;
      remain_hint = m_cells_remain.sub_rslt(
        cells_block->load_cells(
          m_buffer.base, m_buffer.size,
          cell_revs, cells_count,
          was_splitted
        )
      );
    } else {
      SWC_LOG_OUT(LOG_WARN,
        print(SWC_LOG_OSTREAM << "Fragment::load_cells empty buf ");
      );
    }
  }

  if(m_processing.fetch_sub(1) == 1 &&
     (remain_hint <= 0 || Env::Rgr::res().need_ram(size_plain)))
    release();
}

SWC_SHOULD_NOT_INLINE
void Fragment::load_cells(int&, DB::Cells::MutableVec& cells) {
  if(!marked_removed()) {
    if(m_buffer.size) {
      size_t count = 0;
      bool synced = cells.empty();
      size_t offset_hint = 0;
      size_t offset_it = 0;
      const uint8_t* buf = m_buffer.base;
      size_t remain = m_buffer.size;
      try { for(DB::Cells::Cell cell; remain; ++count) {
        cell.read(&buf, &remain);
        synced
          ? cells.add_sorted(cell)
          : cells.add_raw(cell, &offset_it, &offset_hint, false);

      } } catch(...) {
        SWC_LOG_OUT(LOG_ERROR,
          SWC_LOG_OSTREAM
            << "Cell trunclated at count=" << count << '/' << cells_count
            << " remain=" << remain << ' ';
          print(SWC_LOG_OSTREAM);
          SWC_LOG_OSTREAM << ' ' << SWC_CURRENT_EXCEPTION("");
        );
      }
    } else {
      SWC_LOG_OUT(LOG_WARN,
        print(SWC_LOG_OSTREAM << "Fragment::load_cells empty buf ");
      );
    }
  }
  m_processing.fetch_sub(1);
}

SWC_CAN_INLINE
void Fragment::split(int&, const DB::Cell::Key& key,
                     Fragments::Ptr log_left, Fragments::Ptr log_right) {
  if(!marked_removed()) {
    if(m_buffer.size) {
      size_t count = 0;
      const uint8_t* buf = m_buffer.base;
      size_t remain = m_buffer.size;

      try { for(DB::Cells::Cell cell; remain; ++count) {
        cell.read(&buf, &remain);
        DB::KeySeq::compare(interval.key_seq, key, cell.key) == Condition::GT
          ? log_right->add(cell)
          : log_left->add(cell);

      } } catch(...) {
        SWC_LOG_OUT(LOG_ERROR,
          SWC_LOG_OSTREAM
            << "Cell trunclated at count=" << count << '/' << cells_count
            << " remain=" << remain << ' ';
          print(SWC_LOG_OSTREAM);
          SWC_LOG_OSTREAM << ' ' << SWC_CURRENT_EXCEPTION("");
        );
      }
    } else {
      SWC_LOG_OUT(LOG_WARN,
        print(SWC_LOG_OSTREAM << "Fragment::load_cells empty buf ");
      );
    }
  }
  if(m_processing.fetch_sub(1) == 1)
    release();
}

SWC_CAN_INLINE
void Fragment::processing_increment() noexcept {
  m_processing.fetch_add(1);
}

SWC_CAN_INLINE
void Fragment::processing_decrement() noexcept {
  m_processing.fetch_sub(1);
}

size_t Fragment::release() {
  size_t released = 0;
  bool support;
  if(!m_processing && !marked_removed() && loaded() &&
      m_mutex.try_full_lock(support)) {
    State at = State::LOADED;
    if(!marked_removed() && m_queue.empty() && !m_processing &&
       m_state.compare_exchange_weak(at, State::NONE)) {
      released += size_plain;
      m_buffer.free();
      m_cells_remain.store(cells_count);
    }
    m_mutex.unlock(support);
  }
  if(released)
    Env::Rgr::res().less_mem_releasable(size_plain);
  return released;
}

SWC_CAN_INLINE
bool Fragment::loaded() const noexcept {
  return m_state == State::LOADED;
}

SWC_CAN_INLINE
bool Fragment::loaded(int& err) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return !(err = m_err) && loaded();
}

SWC_CAN_INLINE
size_t Fragment::size_bytes() const noexcept {
  return size_plain;
}

SWC_CAN_INLINE
size_t Fragment::size_bytes(bool only_loaded) const noexcept {
  return only_loaded && !loaded() ? 0 : size_plain;
}

SWC_CAN_INLINE
size_t Fragment::size_bytes_encoded() const noexcept {
  return size_enc;
}

SWC_CAN_INLINE
bool Fragment::processing() noexcept {
  bool support;
  bool busy = m_processing ||
              m_state == State::WRITING ||
              m_state == State::LOADING ||
              !m_mutex.try_full_lock(support);
  if(!busy) {
    busy = m_state == State::WRITING ||
           m_state == State::LOADING ||
           !m_queue.empty() ||
           m_processing;
    m_mutex.unlock(support);
  }
  return busy;
}

SWC_CAN_INLINE
bool Fragment::marked_removed() const noexcept {
  return m_marked_removed;
}

bool Fragment::mark_removed() noexcept {
  bool do_remove = !m_marked_removed.exchange(true);
  State state;
  {
    Core::MutexSptd::scope lock(m_mutex);
    state = m_state.exchange(State::LOADED);
  }
  if(do_remove && state != State::NONE)
    Env::Rgr::res().less_mem_releasable(size_plain);
  return do_remove;
}

void Fragment::remove(int &err) {
  if(mark_removed()) {
    if(m_smartfd->valid()) {
      int tmperr = Error::OK;
      Env::FsInterface::interface()->close(tmperr, m_smartfd);
    }
    Env::FsInterface::interface()->remove(err, m_smartfd->filepath());
  }
}

void Fragment::remove(int&, Core::Semaphore* sem) {
  if(m_smartfd->valid()) {
    Env::FsInterface::interface()->close(
      [sem, frag=ptr()](int err) {
        frag->remove(err=Error::OK, sem);
      },
      m_smartfd
    );
  } else if(mark_removed()) {
    Env::FsInterface::interface()->remove(
      [sem, frag=ptr()](int) noexcept { sem->release(); },
      m_smartfd->filepath()
    );
  }
}

void Fragment::print(std::ostream& out) {
  out << "Fragment(version=" << int(version)
      << " count=" << cells_count
      << " offset=" << offset_data
      << " encoder=" << Core::Encoder::to_string(encoder)
      << " enc/size=" << size_enc << '/' << size_plain
      << " state=" << to_string(m_state)
      << " processing=" << m_processing.load();
  {
    Core::MutexSptd::scope lock(m_mutex);
    out << " queue=" << m_queue.size();
    m_smartfd->print(out << ' ');
    if(m_err)
      Error::print(out << ' ', m_err);
  }
  if(m_marked_removed)
    out << " MARKED-REMOVED";
  out << ' ' << interval << ')';
}

void Fragment::load_read(int err, const StaticBuffer::Ptr& buffer) {
  do_load_read:

  if(marked_removed())
    return load_finish(err);

  switch(err) {
    case Error::FS_PATH_NOT_FOUND:
    case Error::SERVER_SHUTTING_DOWN:
      return load_finish(err);
    case Error::OK:
      break;
    default: {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(SWC_LOG_OSTREAM << "Retrying to ", err);
        print(SWC_LOG_OSTREAM << ' ');
      );
      Env::FsInterface::fs()->combi_pread(
        [frag=ptr()] (int _err, const StaticBuffer::Ptr& buff) mutable {
          Env::Rgr::post(TaskLoadRead(std::move(frag), _err, buff));
        },
        m_smartfd, offset_data, size_enc
      );
      //std::this_thread::sleep_for(std::chrono::microseconds(10000));
      return;
    }
  }
  if(!err) {
    if(!Core::checksum_i32_chk(data_checksum, buffer->base, size_enc)) {
      err = Error::CHECKSUM_MISMATCH;

    } else if(buffer->size != size_enc) {
      err = Error::FS_EOF;

    } else {
      if(encoder != DB::Types::Encoder::PLAIN) {
          StaticBuffer decoded_buf(size_plain);
        Core::Encoder::decode(
          err, encoder, buffer->base, size_enc, decoded_buf.base, size_plain);
        if(!err)
          m_buffer.set(decoded_buf);
      } else {
        m_buffer.set(*buffer.get());
      }
    }
  }
  if(err) {
    buffer->free();
    goto do_load_read;
  } else {
    load_finish(err);
  }
}

void Fragment::load_finish(int err) {
  Env::Rgr::res().less_mem_future(size_plain);
  if(err)
    SWC_LOG_OUT(LOG_ERROR,
      Error::print(SWC_LOG_OSTREAM << "CommitLog::Fragment load ", err);
      print(SWC_LOG_OSTREAM << ' ');
    );

  if(m_marked_removed) {
    m_buffer.free();
  } else {
    if(err == Error::FS_PATH_NOT_FOUND) {
      m_buffer.free();
      err = Error::OK;
    }
    bool _marked_removed;
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(!(_marked_removed = m_state == State::LOADED))
        m_state.store(err ? State::NONE : State::LOADED);
      m_err = err;
    }
    if(err && !_marked_removed)
      Env::Rgr::res().less_mem_releasable(size_plain);
  }

  run_queued();
}


void Fragment::run_queued() {
  for(LoadCallback* cb;;) {
    {
      Core::MutexSptd::scope lock(m_mutex);
      if(m_queue.empty())
        return;
      cb = m_queue.front();
      m_queue.pop();
    }
    cb->loaded(ptr());
  }
}



}}} // namespace SWC::Ranger::CommitLog
