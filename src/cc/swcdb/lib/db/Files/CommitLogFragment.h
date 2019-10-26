/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLogFragment_h
#define swcdb_db_Files_CommitLogFragment_h

#include "CellStoreBlock.h"

namespace SWC { namespace Files { 
  
namespace CommitLog {
  
class Fragment: public std::enable_shared_from_this<Fragment> {

  /* file-format: 
        header:      i8(version), i32(header_ext-len), i32(checksum)
        header_ext:  interval, i8(encoder), i32(enc-len), i32(len), i32(cells), 
                     i32(data-checksum), i32(checksum)
        data:        [cell]
  */

  public:

  typedef std::shared_ptr<Fragment> Ptr;
  
  static const uint8_t     HEADER_SIZE = 9;
  static const uint8_t     VERSION = 1;
  static const uint8_t     HEADER_EXT_FIXED_SIZE = 21;
  
  enum State {
    CELLS_NONE,
    CELLS_LOADING,
    CELLS_LOADED,
    ERROR,
  };

  static const std::string to_string(State state) {
    switch(state) {
      case State::CELLS_NONE:
        return std::string("CELLS_NONE");
      case State::CELLS_LOADING:
        return std::string("CELLS_LOADING");
      case State::CELLS_LOADED:
        return std::string("CELLS_LOADED");
      case State::ERROR:
        return std::string("ERROR");
      default:
        return std::string("UKNONWN");
    }
  }

  DB::Cells::Interval  interval;

  Fragment(const std::string& filepath)
          : m_smartfd(
              FS::SmartFd::make_ptr(
                filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE)
            ), 
            m_state(State::CELLS_NONE), 
            m_size_enc(0), m_size(0), m_cells_count(0), m_cells_offset(0), 
            m_data_checksum(0) {
  }
  
  virtual ~Fragment(){}

  void write(int& err, int32_t replication, Types::Encoding encoder, 
             const DB::Cells::Interval& intval, 
             DynamicBuffer& cells, uint32_t cell_count) {
    m_version = VERSION;
    interval.copy(intval);
    
    uint32_t header_extlen = interval.encoded_length()+HEADER_EXT_FIXED_SIZE;
    m_cells_count = cell_count;
    m_size = cells.fill();
    m_cells_offset = HEADER_SIZE+header_extlen;

    DynamicBuffer output;
    m_size_enc = 0;
    err = Error::OK;
    Encoder::encode(encoder, cells.base, m_size, 
                    &m_size_enc, output, m_cells_offset, err);
    if(err)
      return;

    if(m_size_enc) {
      m_encoder = encoder;
    } else {
      m_size_enc = m_size;
      m_encoder = Types::Encoding::PLAIN;
    }
    m_buffer = std::make_shared<StaticBuffer>(cells);
                    
    uint8_t * ptr = output.base;
    Serialization::encode_i8(&ptr, m_version);
    Serialization::encode_i32(&ptr, header_extlen);
    checksum_i32(output.base, ptr, &ptr);

    uint8_t * header_extptr = ptr;
    interval.encode(&ptr);
    Serialization::encode_i8(&ptr, (uint8_t)m_encoder);
    Serialization::encode_i32(&ptr, m_size_enc);
    Serialization::encode_i32(&ptr, m_size);
    Serialization::encode_i32(&ptr, m_cells_count);
    
    checksum_i32(output.base+m_cells_offset, output.base+output.fill(), 
                 &ptr, m_data_checksum);
    checksum_i32(header_extptr, ptr, &ptr);

    StaticBuffer buff_write(output);

    do_again:
    Env::FsInterface::interface()->write(
      err,
      m_smartfd, 
      replication, m_cells_offset+m_size_enc, 
      buff_write
    );

    int tmperr = Error::OK;
    if(!err && Env::FsInterface::fs()->length(
        tmperr, m_smartfd->filepath()) != buff_write.size)
      err = Error::FS_EOF;

    if(err) {
      if(err == Error::FS_PATH_NOT_FOUND 
        //|| err == Error::FS_PERMISSION_DENIED
          || err == Error::SERVER_SHUTTING_DOWN)
        return;
      // if not overwriteflag:
      //   Env::FsInterface::fs()->remove(tmperr, m_smartfd->filepath()) 
      goto do_again;
    }

    m_state = State::CELLS_LOADED;
  }

  void load_header(bool close_after=true) {
    int err = Error::OK;
    load_header(err, close_after);
    if(err)
      m_state = State::ERROR;
  }

  void load_header(int& err, bool close_after=true) {
    
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
      
      StaticBuffer buf(HEADER_SIZE);
      const uint8_t *ptr = buf.base;
      if(Env::FsInterface::fs()->pread(
          err, m_smartfd, 0, buf.base, HEADER_SIZE) != HEADER_SIZE){
        if(err != Error::FS_EOF){
          Env::FsInterface::fs()->close(err, m_smartfd);
          continue;
        }
        return;
      }

      size_t remain = HEADER_SIZE;
      m_version = Serialization::decode_i8(&ptr, &remain);
      uint32_t header_extlen = Serialization::decode_i32(&ptr, &remain);
      if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), buf.base, HEADER_SIZE-4)){  
        err = Error::CHECKSUM_MISMATCH;
        return;
      }

      buf.reallocate(header_extlen);
      ptr = buf.base;
      if(Env::FsInterface::fs()->pread(err, m_smartfd, HEADER_SIZE, 
                                    buf.base, header_extlen) != header_extlen){
        if(err != Error::FS_EOF){
          Env::FsInterface::fs()->close(err, m_smartfd);
          continue;
        }
        return;
      }

      remain = header_extlen;
      interval.decode(&ptr, &remain, true);
      m_encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
      m_size_enc = Serialization::decode_i32(&ptr, &remain);
      m_size = Serialization::decode_i32(&ptr, &remain);
      m_cells_count = Serialization::decode_i32(&ptr, &remain);
      m_data_checksum = Serialization::decode_i32(&ptr, &remain);

      if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), buf.base, header_extlen-4)){  
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

  void load() {
    int err;
    uint8_t tries = 0;
    for(;;) {
      err = Error::OK;
    
      if(!m_smartfd->valid() 
         && !Env::FsInterface::interface()->open(err, m_smartfd) && err)
        break;
      if(err)
        continue;
      
      StaticBuffer read_buf(m_size_enc);
      if(Env::FsInterface::fs()->pread(
            err, m_smartfd, m_cells_offset, read_buf.base, m_size_enc) 
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
      
      if(!checksum_i32_chk(m_data_checksum, read_buf.base, m_size_enc)){  
        if(++tries == 3) {
          err = Error::CHECKSUM_MISMATCH;
          break;
        }
        continue;
      }

      m_buffer = std::make_shared<StaticBuffer>(m_size);
      if(m_encoder != Types::Encoding::PLAIN) {
        StaticBuffer buffer(m_size);
        Encoder::decode(
          m_encoder, read_buf.base, m_size_enc, m_buffer->base, m_size, err);
        read_buf.free();
      } else {
        m_buffer->base = read_buf.base;
        read_buf.own=false;
      }
      break;
    }
    
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = err ? State::ERROR : State::CELLS_LOADED;
    }

    std::function<void(int)> cb;
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        cb = m_queue_load.front();
      }

      cb(err);
      
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue_load.pop();
        if(m_queue_load.empty())
          return;
      }
    }
    std::cout << "LogFragment, load-complete" << "\n";
  }

  void load(std::function<void(int)> cb) {
    bool state;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      state = m_state == State::CELLS_LOADED;
    }
    
    if(state) {
      cb(Error::OK);
      return;
    }

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_queue_load.push(cb);
      state = m_queue_load.size() == 1;
    }

    if(state) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_state == State::CELLS_LOADING)
          return;
        m_state = State::CELLS_LOADING;
      }
      asio::post(*Env::IoCtx::io()->ptr(), 
        [ptr=shared_from_this()](){
          ptr->load();
        }
      );
    }
  }
  
  void load_cells(const DB::Cells::Interval& intval, 
                  DB::Cells::Mutable& cells) {

    const uint8_t* ptr = m_buffer->base;
    size_t remain = m_size; 

    auto ts = Time::now_ns();
    int64_t ts_cells_add = 0;
    size_t count = 0;

    DB::Cells::Cell cell;
    while(remain) {
      try {
        cell.read(&ptr, &remain);

        if(++count == m_cells_count && remain != 0)
          throw Error::SERIALIZATION_INPUT_OVERRUN;
      } catch(std::exception) {
        HT_ERRORF(
          "Cell trunclated count=%llu remain=%llu %s, %s", 
          count, remain, cell.to_string().c_str(),  to_string().c_str());
        break;
      }

      if(!intval.consist(cell.key))
        continue;

      auto ts_cella = Time::now_ns();
      cells.add(cell);
      ts_cells_add += Time::now_ns()-ts_cella;
    }

    ts = Time::now_ns()-ts;
    std::cout << " frag-load_cells-took=" << ts
              << " add=" << ts_cells_add << " count=" << count 
              << " avg=" << (count>0? ts/count : 0) << "\n";
  }

  bool loaded() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::CELLS_LOADED;
  }

  bool errored() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::ERROR;
  }

  uint32_t cells_count() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cells_count;
  }

  size_t size_bytes() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_size;
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
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
    s.append(")");
    return s;
  }

  private:
  std::mutex      m_mutex;
  State           m_state;
  FS::SmartFdPtr  m_smartfd;
  uint8_t         m_version;
  Types::Encoding m_encoder;
  size_t          m_size_enc;
  size_t          m_size;
  StaticBufferPtr m_buffer;
  uint32_t        m_cells_count;
  uint32_t        m_cells_offset;
  uint32_t        m_data_checksum;

  std::queue<std::function<void(int)>> m_queue_load;
  

};

}

}}

#endif