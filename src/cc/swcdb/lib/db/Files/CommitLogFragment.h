/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLogFragment_h
#define swcdb_db_Files_CommitLogFragment_h

#include "CellStore.h"

namespace SWC { namespace Files { 
  
namespace CommitLog {
  
class Fragment: public std::enable_shared_from_this<Fragment> {

  /* file-format: 
        header:      i8(version), i32(header_ext-len), i32(checksum)
        header_ext:  interval, i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
        data:        [cell]
  */

  public:
  typedef std::shared_ptr<Fragment> Ptr;
  
  static const int8_t     HEADER_SIZE = 9;
  static const int8_t     VERSION = 1;
  
  enum State {
    CELLS_NONE,
    CELLS_LOADING,
    CELLS_LOADED,
  };

  DB::Cells::Interval::Ptr  interval;

  Fragment(const std::string& filepath)
          : m_smartfd(
              FS::SmartFd::make_ptr(
                filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE)
            ), 
            m_state(State::CELLS_NONE), 
            m_size_enc(0), m_size(0), m_cells_count(0), m_cells_offset(0) {
  }
  
  virtual ~Fragment(){}

  void write(int& err, int32_t replication, Types::Encoding encoder, 
             DB::Cells::Interval::Ptr intval, 
             DynamicBuffer& cells, uint32_t cell_count) {
    
    
    uint32_t header_extlen = intval->encoded_length()+17;
    m_cells_count = cell_count;
    interval = intval;
    m_size = cells.fill();
    m_cells_offset = HEADER_SIZE+header_extlen;

    DynamicBuffer output;
    m_size_enc = 0;
    output.set_mark();
    err = Error::OK;
    Encoder::encode(encoder, cells.base, m_size, 
                    &m_size_enc, output, m_cells_offset, err);
    if(err)
      return;
                    
    uint8_t * ptr = output.mark;
    *ptr++ = VERSION;
    Serialization::encode_i32(&ptr, header_extlen);
    checksum_i32(output.mark, ptr, &ptr);
    
    uint8_t * header_extptr = ptr;
    interval->encode(&ptr);
    *ptr++ = (uint8_t)(m_size_enc? encoder: Types::Encoding::PLAIN);
    if(!m_size_enc)
      m_size_enc = m_size;
    Serialization::encode_i32(&ptr, m_size_enc);
    Serialization::encode_i32(&ptr, m_size);
    Serialization::encode_i32(&ptr, m_cells_count);
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
      int8_t version = Serialization::decode_i8(&ptr, &remain);
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
      interval = std::make_shared<DB::Cells::Interval>(&ptr, &remain);
      m_encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
      m_size_enc = Serialization::decode_i32(&ptr, &remain);
      m_size = Serialization::decode_i32(&ptr, &remain);
      m_cells_count = Serialization::decode_i32(&ptr, &remain);

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

  void load_cells(CellStore::ReadersPtr cellstores, 
                  std::function<void(int)> cb) {
    bool run_loader;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::CELLS_LOADED) {
        cb(Error::OK);
        return;
      }
      m_queue_load.push(cb);
      run_loader = m_queue_load.size() == 1;
    }

    if(run_loader){
      asio::post(*Env::IoCtx::io()->ptr(), 
        [cellstores, ptr=shared_from_this()](){
          ptr->load_cells(cellstores);
        }
      );
    }
  }

  void load_cells(CellStore::ReadersPtr cellstores) {
    int err;
    StaticBuffer read_buf(m_size_enc);

    for(;;) {
      err = Error::OK;
    
      if(!m_smartfd->valid() 
         && !Env::FsInterface::interface()->open(err, m_smartfd) && err)
        break;
      if(err)
        continue;
      
      if(Env::FsInterface::fs()->pread(
            err, m_smartfd, m_cells_offset, read_buf.base, m_size_enc) 
          != m_size_enc){
        if(m_smartfd->valid()) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, m_smartfd);
        }
        if(err == Error::FS_EOF)
            break;
        continue;
      }
      if(err)
        continue;
      
      if(m_encoder != Types::Encoding::PLAIN) {
        StaticBuffer buffer(m_size);
        Encoder::decode(
          m_encoder, read_buf.base, m_size_enc, buffer.base, m_size, err);
        if(err) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, m_smartfd);
          break;
        }
        read_buf.free();
        read_buf.base = buffer.base;
        buffer.own=false;
      }
      
      DB::Cells::Cell cell;
      const uint8_t* ptr = read_buf.base;
      size_t remain = m_size; 

      while(remain) {
        cell.read(&ptr, &remain);

        for(auto cs : *cellstores.get()) {
          if(cs->add_logged(cell) && !cell.on_fraction)
            break;
        }
      }
      break;
    }

    if(m_smartfd->valid()) {
      int tmperr = Error::OK;
      Env::FsInterface::fs()->close(tmperr, m_smartfd);
    }

    if(!err) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = State::CELLS_LOADED;
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
    std::cout << "LogFragment, load_cells-complete" << "\n";
  }

  bool loaded() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::CELLS_LOADED;
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("Fragment(state=");
    s.append(std::to_string((uint8_t)m_state));

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
    s.append(interval->to_string());

    s.append(" ");
    s.append(m_smartfd->to_string());
    s.append(")");
    return s;
  }

  private:
  std::mutex      m_mutex;
  State           m_state;
  FS::SmartFdPtr  m_smartfd;
  Types::Encoding m_encoder;
  size_t          m_size_enc;
  size_t          m_size;
  uint32_t        m_cells_count;
  uint32_t        m_cells_offset;

  std::queue<std::function<void(int)>> m_queue_load;
  

};

}

}}

#endif