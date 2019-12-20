/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStoreBlock_h
#define swcdb_db_Files_CellStoreBlock_h

#include "swcdb/core/Encoder.h"
#include "swcdb/db/Cells/Block.h"


namespace SWC { namespace Files { namespace CellStore {


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
      data:   [cell]
*/

static const uint8_t HEADER_SIZE=17;



class Read final {  
  public:
  typedef Read* Ptr;

  enum State {
    NONE,
    LOADING,
    LOADED
  };

  inline static const std::string to_string(const State state) {
    switch(state) {
      case State::LOADED:
        return "LOADED";
      case State::LOADING:
        return "LOADING";
      default:
        return "NONE";
    }
  }
  
  inline static Ptr make(const size_t offset, 
                         const DB::Cells::Interval& interval) {
    return new Read(offset, interval);
  }

  const size_t                          offset;
  const DB::Cells::Interval             interval;

  explicit Read(const size_t offset, const DB::Cells::Interval& interval)
                : offset(offset), interval(interval), 
                  m_state(State::NONE), m_processing(0), 
                  m_loaded_header(false),
                  m_size(0), m_sz_enc(0), m_cells_remain(0), m_cells_count(0), 
                  m_err(Error::OK) {
  }
  
  Ptr ptr() {
    return this;
  }

  ~Read() { }
  
  bool load(const std::function<void()>& cb) {
    {
      std::scoped_lock lock(m_mutex);
      m_processing++;
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

  void load(FS::SmartFd::Ptr smartfd, const std::function<void()>& cb) {
    int err = Error::OK;
    load(err, smartfd);
    if(err)
      SWC_LOGF(LOG_ERROR, "CellStore::Block load %s", to_string().c_str());

    cb();
    run_queued();
  }

  void load_cells(int& err, DB::Cells::Block::Ptr cells_block) {
    bool was_splitted = false;
    if(loaded(err)) {
      if(m_buffer.size)
        m_cells_remain -= cells_block->load_cells(
          m_buffer.base, m_buffer.size, m_cells_count, was_splitted);
    } else {
      SWC_LOGF(LOG_WARN, "CS-Block::load_cells at not loaded %s", 
               to_string().c_str());
    }

    {
      std::scoped_lock lock(m_mutex);
      m_processing--; 
    }

    if(!was_splitted 
       && (!m_cells_remain.load() || Env::Resources.need_ram(m_size)))
      release();
  }
  
  const size_t release() {    
    size_t released = 0;
    std::scoped_lock lock(m_mutex);

    if(m_processing || m_state != State::LOADED)
      return released; 

    released += m_buffer.size;    
    m_state = State::NONE;
    m_buffer.free();
    m_cells_remain = m_cells_count;
    return released;
  }

  const bool processing() {
    std::shared_lock lock(m_mutex);
    return m_processing;
  }

  const int error() {
    std::shared_lock lock(m_mutex);
    return m_err;
  }

  const bool loaded() {
    std::shared_lock lock(m_mutex);
    return m_state == State::LOADED;
  }
  
  const bool loaded(int& err) {
    std::shared_lock lock(m_mutex);
    err = m_err;
    return !err && m_state == State::LOADED;
  }

  const size_t size_bytes(bool only_loaded=false) {
    std::shared_lock lock(m_mutex);
    if(only_loaded && m_state != State::LOADED)
      return 0;
    return m_size;
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);
    std::string s("Block(offset=");
    s.append(std::to_string(offset));
    s.append(" state=");
    s.append(to_string(m_state));
    s.append(" size=");
    s.append(std::to_string(m_size));
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

  
  private:
  
  void load(int& err, FS::SmartFd::Ptr smartfd) {

    for(;;) {
      err = Error::OK;

      if(!smartfd->valid() 
        && !Env::FsInterface::interface()->open(err, smartfd) 
        && err)
        break;
      if(err)
        continue;
      
      if(!m_loaded_header) { // load header once
        uint8_t buf[HEADER_SIZE];
        const uint8_t *ptr = buf;
        if(Env::FsInterface::fs()->pread(err, smartfd, offset, buf, HEADER_SIZE)
                != HEADER_SIZE) {
          if(err != Error::FS_EOF){
            Env::FsInterface::fs()->close(err, smartfd);
            continue;
          }
          break;
        }

        size_t remain = HEADER_SIZE;
        m_encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
        m_sz_enc = Serialization::decode_i32(&ptr, &remain);
        m_size = Serialization::decode_i32(&ptr, &remain);
        if(!m_sz_enc) 
          m_sz_enc = m_size;
        m_cells_remain=m_cells_count= Serialization::decode_i32(&ptr, &remain);

        if(!checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain), buf, HEADER_SIZE-4)){  
          err = Error::CHECKSUM_MISMATCH;
          break;
        }
        m_loaded_header = true;
      }
      if(!m_sz_enc) // a zero cells type cs (initial of any to any block)
        break;

      for(;;) {
        m_buffer.free();
        err = Error::OK;
        if(Env::FsInterface::fs()->pread(
              err, smartfd, offset+HEADER_SIZE, &m_buffer, m_sz_enc)
            != m_sz_enc){
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, smartfd);
          if(err != Error::FS_EOF){
            if(!Env::FsInterface::interface()->open(err, smartfd))
              break;
            continue;
          }
        }
        break;
      }
      if(err)
        break;

      
      if(m_encoder != Types::Encoding::PLAIN) {
        StaticBuffer decoded_buf(m_size);
        Encoder::decode(
          err, m_encoder, m_buffer.base, m_sz_enc, decoded_buf.base, m_size);
        if(err) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, smartfd);
          break;
        }
        m_buffer.set(decoded_buf);
      }

      break;
    }

    std::scoped_lock lock(m_mutex);
    m_err = err;
    m_state = m_err ? State::NONE : State::LOADED;
  }

  void run_queued() {
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

  void _run_queued() {
    std::function<void()> call;
    for(;;) {
      {
        std::shared_lock lock(m_mutex);
        call = m_queue.front();
      }

      call();
      
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

  std::shared_mutex                  m_mutex;
  State                              m_state;
  size_t                             m_processing;
  bool                               m_loaded_header;
  Types::Encoding                    m_encoder;
  size_t                             m_size;
  size_t                             m_sz_enc;
  uint32_t                           m_cells_count;
  StaticBuffer                       m_buffer;
  bool                               m_q_runs = false;
  std::queue<std::function<void()>>  m_queue;
  std::atomic<uint32_t>              m_cells_remain;
  int                                m_err;
};



class Write final {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const size_t offset, const DB::Cells::Interval& interval, 
        const uint32_t cell_count)
        : offset(offset), interval(interval), cell_count(cell_count) { 
  }

  ~Write() { }

  void write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
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
    checksum_i32(output.mark, ptr, &ptr);
  }

  const std::string to_string() {
    std::string s("Block(offset=");
    s.append(std::to_string(offset));
    s.append(" cell_count=");
    s.append(std::to_string(cell_count));
    s.append(" ");
    s.append(interval.to_string());
    s.append(")");
    return s;
  }

  const size_t              offset;
  const DB::Cells::Interval interval;
  const uint32_t            cell_count;

};



} // Block namespace


}}}

#endif