/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStore_h
#define swcdb_db_Files_CellStore_h

#include "swcdb/lib/core/Encoder.h"
#include "swcdb/lib/ranger/callbacks/RangeScan.h"



namespace SWC { namespace Files {

class Block {  

  /* file-format: 
        header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
        data:   [cell]
  */

  public:
  typedef std::shared_ptr<Block> Ptr;
  
  static const int HEADER_SIZE=17;

  Block(const size_t offset, DB::Cells::Interval::Ptr interval, 
        DB::Cells::Mutable::Ptr log_cells = nullptr)
        : offset(offset), loaded(false), interval(interval), log_cells(log_cells) { 
  }

  virtual ~Block(){}

  Ptr make_fresh() {
    return std::make_shared<Block>(offset, interval, log_cells);
  }

  void load(FS::SmartFdPtr smartfd, std::function<void()> call) {
    std::cout << "blk::load\n";
    if(loaded) {
      call();
      return;
    }

    int err;

    for(;;) {
      err = Error::OK;

      if(!smartfd->valid() && !Env::FsInterface::interface()->open(err, smartfd) && err)
        break;
      if(err)
        continue;
      
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
      Types::Encoding encoder = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
      uint32_t sz_enc = Serialization::decode_i32(&ptr, &remain);
      uint32_t sz = Serialization::decode_i32(&ptr, &remain);
      if(!sz_enc) 
        sz_enc = sz;
      uint32_t cells_count = Serialization::decode_i32(&ptr, &remain);//?

      if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), buf, HEADER_SIZE-4)){  
        err = Error::CHECKSUM_MISMATCH;
        break;
      }

      StaticBuffer read_buf(sz_enc);
      for(;;) {
        err = Error::OK;
        if(Env::FsInterface::fs()->pread(
                    err, smartfd, offset+HEADER_SIZE, read_buf.base, sz_enc)
                  != sz_enc){
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

      if(buffer == nullptr)
        buffer = std::make_shared<StaticBuffer>(sz);
      
      if(encoder != Types::Encoding::PLAIN) {
        Encoder::decode(encoder, read_buf.base, sz_enc, &buffer->base, sz, err);
        if(err) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, smartfd);
          break;
        }
        read_buf.free();
      } else {
        read_buf.own = false;
        buffer->base = read_buf.base;
      }
      break;
    }
    
    loaded = err == Error::OK;
    call();
  }
  
  void scan(server::Rgr::Callback::RangeScan::Ptr req) {
    std::cout << "blk::scan\n";
    int err;
    DB::Specs::Interval& spec = *(req->spec).get();
    DB::Cells::Mutable& cells = *(req->cells).get();

    log_cells->scan(spec, req->cells);
    if(spec.flags.limit == cells.size())
      return;

    DB::Cells::Cell cell;
    uint8_t* ptr = buffer->base;
    size_t remain = buffer->size; 
    
    while(remain) {
      cell.read(&ptr, &remain);
      if(!spec.is_matching(cell))
        continue;
      cells.add(cell);
      if(spec.flags.limit == cells.size())
        break;
    }
  }


  const std::string to_string() {
    std::string s("Block(offset=");
    s.append(std::to_string(offset));

    s.append(" loaded=");
    s.append(std::to_string(loaded));

    s.append(" ");
    s.append(interval->to_string());
    s.append(")");
    return s;
  }

  const size_t              offset;
  DB::Cells::Interval::Ptr  interval;
  std::atomic<bool>         loaded;
  StaticBufferPtr           buffer;
  DB::Cells::Mutable::Ptr   log_cells;

};



class CellStore : public std::enable_shared_from_this<CellStore> {

  /* file-format: 
      [blocks]: 
        header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
        data:   [cell]
      blocks-index:
        header: i8(encoder), i32(enc-len), vi32(len), vi32(blocks), i32(checksum)
        data:   [vi32(offset), interval, i32(checksum)]
      trailer : i8(version), i32(blocks-index-len), i32(checksum)
        (trailer-offset) = fileLength - TRAILER_SIZE
        (blocks-index-offset) = (trailer-offset) - (blocks-index-len)
  */

  public:

  static const int TRAILER_SIZE=9;
  static const int8_t VERSION=1;

  typedef std::shared_ptr<CellStore> Ptr;

  enum State {
    BLKS_IDX_NONE,
    BLKS_IDX_LOADING,
    BLKS_IDX_LOADED,
  };

  const uint32_t            id;
  FS::SmartFdPtr            smartfd;
  DB::Cells::Interval::Ptr  interval;

  CellStore(uint32_t id) 
          : id(id), smartfd(nullptr), interval(nullptr), 
            m_state(State::BLKS_IDX_NONE) {            
  }

  virtual ~CellStore(){}

  State load_blocks_index(int& err, bool close_after=false) {
    std::cout << "cs::load_blocks_index\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state == State::BLKS_IDX_LOADED || m_state == State::BLKS_IDX_LOADING)
        return m_state;
      m_state = State::BLKS_IDX_LOADING;
    }

    _load_blocks_index(err, close_after);

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_state = err? State::BLKS_IDX_NONE : State::BLKS_IDX_LOADED;
      return m_state;
    }
  }

  void scan(server::Rgr::Callback::RangeScan::Ptr req) {
    std::cout << "cs::Scan\n";
    int err = Error::OK;

    State state = load_blocks_index(err);

    if(state == State::BLKS_IDX_NONE){
      if(!err)
        err = Error::RANGE_CS_BAD;
      req->response(err);
      return;
    }

    if(state == State::BLKS_IDX_LOADING) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_blocks_q.push(
        [req, ptr=shared_from_this()](){ptr->scan_block(0, req);}
      );
      return;
    }
    
    run_queued();
    
    scan_block(0, req);
  }

  void run_queued() {
    std::cout << "cs::run_queued\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_blocks_q_runs || m_blocks_q.empty())
        return;
      m_blocks_q_runs = true;
    }
    
    asio::post(*Env::IoCtx::io()->ptr(), 
      [ptr=shared_from_this()](){
        ptr->_run_queued();
      }
    );
  }

  void scan_block(uint32_t idx, server::Rgr::Callback::RangeScan::Ptr req) {
    std::cout << "cs::scan_block\n";
    Block::Ptr blk;
    for(; idx < m_blocks.size(); idx++) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        blk = m_blocks[idx];
      }

      if(!blk->interval->includes(req->spec))
        continue;
    
      if(!blk->loaded) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          std::cout << "cs::scan_block, shared_from_this 1\n";
          Ptr ptr= shared_from_this();
          std::cout << "cs::scan_block, shared_from_this 2\n";
          m_blocks_q.push(
            [blk, req, idx, ptr](){
              blk->load(
                ptr->smartfd, 
                [req, idx, ptr](){
                  ptr->scan_block(idx, req);
                }
              );
            }
          );
        }
        run_queued();
        return;  
      }

      blk->scan(req);
      if(req->spec->flags.limit == req->cells->size())
        break;
    }
    int err = Error::OK;
    req->response(err);
  }

  size_t release(size_t bytes) {    
    size_t released = 0;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state != State::BLKS_IDX_LOADED)
        return released;
    }

    size_t used;
    for(uint32_t idx = m_blocks.size(); --idx >= 0;) {
      std::lock_guard<std::mutex> lock(m_mutex);

      if(!m_blocks[idx]->loaded)
        continue;

      used = m_blocks[idx]->buffer->size;
      if(!used)
        continue;

      released += used;
      m_blocks[idx] = m_blocks[idx]->make_fresh();
      if(released >= bytes)
        break;
    }
    return released;
  }

  void close(int &err) {
    Env::FsInterface::fs()->close(err, smartfd); 
  }

  void remove(int &err) {
    Env::FsInterface::fs()->remove(err, smartfd->filepath()); 
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("CellStore(v=");
    s.append(std::to_string(VERSION));
    s.append(" id=");
    s.append(std::to_string(id));
    s.append(" state=");
    s.append(std::to_string((uint8_t) m_state));
    s.append(" ");
    if(interval != nullptr)
      s.append(interval->to_string());
    if(smartfd != nullptr){
      s.append(" ");
      s.append(smartfd->to_string());
    }

    s.append(" blocks=");
    s.append(std::to_string(m_blocks.size()));
    
    s.append(" blocks=[");
    for(auto blk : m_blocks) {
        s.append(blk->to_string());
       s.append(", ");
    }
    s.append("])");
    return s;
  } 

  private:

  bool load_trailer(int& err, size_t& blks_idx_size, 
                              size_t& blks_idx_offset, 
                              bool close_after=false) {
    bool state;
    for(;;) {
      err = Error::OK;
      state = false;
    
      if(!Env::FsInterface::fs()->exists(err, smartfd->filepath())) {
        if(err != Error::OK && err != Error::SERVER_SHUTTING_DOWN)
          continue;
        return state;
      }
      size_t length = Env::FsInterface::fs()->length(err, smartfd->filepath());
      if(err) {
        if(err == Error::FS_PATH_NOT_FOUND ||
           err == Error::FS_PERMISSION_DENIED ||
           err == Error::SERVER_SHUTTING_DOWN)
          return state;
        continue;
      }
      
      if(!Env::FsInterface::interface()->open(err, smartfd) && err)
        return state;
      if(err)
        continue;
      
      uint8_t buf[TRAILER_SIZE];
      const uint8_t *ptr = buf;
      if(Env::FsInterface::fs()->pread(
                err, smartfd, length-TRAILER_SIZE, buf, TRAILER_SIZE)
              != TRAILER_SIZE){
        if(err != Error::FS_EOF){
          Env::FsInterface::fs()->close(err, smartfd);
          continue;
        }
        return state;
      }

      size_t remain = TRAILER_SIZE;
      int8_t version = Serialization::decode_i8(&ptr, &remain);
      blks_idx_size = Serialization::decode_i32(&ptr, &remain);
      if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), buf, TRAILER_SIZE-4)){  
        err = Error::CHECKSUM_MISMATCH;
        break;
      }
      
      blks_idx_offset = length-blks_idx_size-TRAILER_SIZE;      
      state = true;
      break;
    }

    if(close_after && smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd);
    return state;
  }

  void _load_blocks_index(int& err, bool close_after=false) {
    std::cout << "cs::_load_blocks_index\n";
    m_blocks.clear();

    size_t length = 0;
    size_t offset = 0;
    if(!load_trailer(err, length, offset, false))
      return;

    StaticBuffer read_buf(length);
    for(;;) {
      err = Error::OK;
      if(Env::FsInterface::fs()->pread(
                          err, smartfd, offset, read_buf.base, length)
                != length){
        int tmperr = Error::OK;
        Env::FsInterface::fs()->close(tmperr, smartfd);
        if(err != Error::FS_EOF){
          if(!Env::FsInterface::interface()->open(err, smartfd))
            return;
          continue;
        }
        return;
      }
      break;
    }

    const uint8_t *ptr = read_buf.base;
    size_t remain = length;
    Types::Encoding encoder = (Types::Encoding)*ptr++;
    uint32_t sz_enc = Serialization::decode_i32(&ptr, &remain);
    uint32_t sz = Serialization::decode_vi32(&ptr, &remain);
    uint32_t blks_count = Serialization::decode_vi32(&ptr, &remain);
    
    if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), 
        read_buf.base, ptr-read_buf.base)
      ) {
      Env::FsInterface::fs()->close(err, smartfd);
      err = Error::CHECKSUM_MISMATCH;
      return;
    }
    
    StaticBuffer decoded_buf;
    if(encoder != Types::Encoding::PLAIN) {
      decoded_buf.reallocate(sz);
      Encoder::decode(encoder, ptr, sz_enc, &decoded_buf.base, sz, err);
      if(err) {
        int tmperr = Error::OK;
        Env::FsInterface::fs()->close(tmperr, smartfd);
        return;
      }
      read_buf.free();
      ptr = decoded_buf.base;
      remain = sz;
    }

    const uint8_t* chk_ptr;
    bool init=false;
    if(interval == nullptr)
      interval = std::make_shared<DB::Cells::Interval>();
    for(int n = 0; n < blks_count; n++){
      chk_ptr = ptr;
      uint32_t offset = Serialization::decode_vi32(&ptr, &remain);
      Block::Ptr blk = std::make_shared<Block>(
        offset, 
        std::make_shared<DB::Cells::Interval>(&ptr, &remain)
      );  
      if(!checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain), chk_ptr, ptr-chk_ptr)) {
        Env::FsInterface::fs()->close(err, smartfd);
        err = Error::CHECKSUM_MISMATCH;
        return;
      }

      m_blocks.push_back(blk);
      interval->expand(blk->interval, init);
      init = true;
    }
    
    if(close_after && smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd);
  }

  void _run_queued() {
    std::cout << "cs::_run_queued\n";
    std::function<void()> call;
    
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        call = m_blocks_q.front();
      }

      call();
      
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_blocks_q.pop();
        if(m_blocks_q.empty()){
          m_blocks_q_runs = false;
          return;
        }
      }
    }
  }

  std::mutex                m_mutex;
  std::atomic<State>        m_state;
  std::vector<Block::Ptr>   m_blocks;
  bool                      m_blocks_q_runs = false;
  std::queue<std::function<void()>>  m_blocks_q;

};
typedef std::vector<Files::CellStore::Ptr> CellStores;


} // Files namespace

}

#include "CellStoreWrite.h"

#endif