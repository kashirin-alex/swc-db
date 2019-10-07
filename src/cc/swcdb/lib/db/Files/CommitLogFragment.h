/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLogFragment_h
#define swcdb_db_Files_CommitLogFragment_h

#include "CellStore.h"

namespace SWC { namespace Files { 
  
namespace CommitLog {
  
class Fragment {

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
          : m_smartfd(FS::SmartFd::make_ptr(filepath, 0)), 
            m_state(State::CELLS_NONE), 
            m_size_enc(0), m_size(0), m_cells_count(0), m_cells_offset(0) {
  }
  
  virtual ~Fragment(){}

  void create(int& err, 
              int32_t bufsz=-1, int32_t replication=-1, int64_t blksz=1) {
    while(
      Env::FsInterface::interface()->create(
        err, m_smartfd, bufsz, replication, blksz));
  }

  void write(int& err, Types::Encoding encoder, 
             DB::Cells::Interval::Ptr intval, 
             DynamicBuffer& cells, uint32_t cell_count) {
    err = Error::OK;

    uint32_t header_extlen = intval->encoded_length()+17;
    m_cells_count = cell_count;
    interval = intval;
    m_size = cells.fill();
    m_cells_offset = HEADER_SIZE+header_extlen;

    DynamicBuffer output;
    m_size_enc = 0;
    output.set_mark();
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
    *ptr++ = (uint8_t)encoder;
    Serialization::encode_i32(&ptr, m_size_enc);
    Serialization::encode_i32(&ptr, m_size);
    Serialization::encode_i32(&ptr, m_cells_count);
    checksum_i32(header_extptr, ptr, &ptr);

    StaticBuffer buff_write(output);

    Env::FsInterface::fs()->append(
      err,
      m_smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
    
    if(m_smartfd->valid()) {
      int tmperr = Error::OK;
      Env::FsInterface::fs()->close(tmperr, m_smartfd);
    }

    if(Env::FsInterface::fs()->length(err, m_smartfd->filepath()) 
        != buff_write.size)
      err = Error::FS_EOF;
      // do-again

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

  void load_cells(int& err, 
                  const std::vector<CellStore::Read::Ptr>& cellstores) {
    
    for(;;) {
      err = Error::OK;
    
      if(!m_smartfd->valid() 
         && !Env::FsInterface::interface()->open(err, m_smartfd) && err)
        return;
      if(err)
        continue;
      
      StaticBuffer read_buf(m_size_enc);
      for(;;) {
        err = Error::OK;
        if(Env::FsInterface::fs()->pread(
                    err, m_smartfd, m_cells_offset, read_buf.base, m_size_enc)
                  != m_size_enc){
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, m_smartfd);
          if(err != Error::FS_EOF){
            if(!Env::FsInterface::interface()->open(err, m_smartfd))
              break;
            continue;
          }
        }
        break;
      }
      if(err)
        break;

      StaticBuffer buffer(m_size);
      
      if(m_encoder != Types::Encoding::PLAIN) {
        Encoder::decode(
          m_encoder, read_buf.base, m_size_enc, buffer.base, m_size, err);
        if(err) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, m_smartfd);
          break;
        }
        read_buf.free();
      } else {
        read_buf.own = false;
        buffer.base = read_buf.base;
      }

      
      DB::Cells::Cell cell;
      uint8_t* ptr = buffer.base;
      size_t remain = buffer.size; 

      while(remain) {
        cell.read(&ptr, &remain);
        for(auto cs : cellstores) {
          if(cs->add_logged(cell)) // add to all aplicable (.on_fraction)
            break;
        }
      }
      break;
    }

    if(m_smartfd->valid()) {
      int tmperr = Error::OK;
      Env::FsInterface::fs()->close(tmperr, m_smartfd);
    }

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = State::CELLS_LOADED;
    }
  }

  bool loaded() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::CELLS_LOADED;
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("Fragment(state=");
    s.append(std::to_string((uint8_t)m_state));

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
  

};

}

}}

#endif