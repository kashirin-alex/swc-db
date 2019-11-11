/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStoreBlock_h
#define swcdb_db_Files_CellStoreBlock_h

#include "swcdb/lib/core/Encoder.h"
#include "swcdb/lib/db/Cells/ReqScan.h"



namespace SWC { namespace Files { 

struct CellsBlock {
  public:
  typedef CellsBlock* Ptr;
  
  inline static Ptr make(const DB::Cells::Interval& interval, 
                         const DB::Schema::Ptr s) {
    return new CellsBlock(interval, s);
  } 
  
  CellsBlock(const DB::Cells::Interval& interval, const DB::Schema::Ptr s) 
            : interval(interval),  
              cells(
                DB::Cells::Mutable(
                  0, s->cell_versions, s->cell_ttl, s->col_type)
              ) {
  }

  virtual ~CellsBlock() {
    //std::cout << " ~CellsBlock\n";
  }
  
  size_t load_cells(const uint8_t* ptr, size_t remain) {
    DB::Cells::Cell cell;
    size_t count = 0;
    bool synced = !cells.size();

    //auto ts = Time::now_ns();
    while(remain) {
      try {
        cell.read(&ptr, &remain);
        count++;
      } catch(std::exception) {
        HT_ERRORF(
          "Cell trunclated count=%llu remain=%llu %s, %s", 
          count, remain, cell.to_string().c_str(),  to_string().c_str());
        break;
      }
      
      if(!interval.consist(cell.key))
        continue;

      if(synced)
        cells.push_back(cell);
      else
        cells.add(cell);

      //if(splitter)
      //  splitter();
    }

    //auto took = Time::now_ns()-ts;
    //std::cout << "CellsBlock::load_cells took=" << took
    //          << " synced=" << synced
    //         << " avg=" << (count>0 ? took / count : 0)
    //          << " " << cells.to_string() << "\n";
    return count;
  }

  const std::string to_string(){
      std::string s("CellsBlock(");
      s.append(interval.to_string());
      s.append(" ");
      s.append(cells.to_string());
      s.append(")");
      return s;
    }
  //std::function<void()>     splitter=0;
  DB::Cells::Interval       interval;
  DB::Cells::Mutable        cells;
};

}}

namespace SWC { namespace Files { namespace CellStore {


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
      data:   [cell]
*/

static const uint8_t HEADER_SIZE=17;



class Read {  
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

  Read(const size_t offset, const DB::Cells::Interval& interval)
      : offset(offset), interval(interval), 
        m_state(State::NONE), m_processing(0), m_loaded_header(false),
        m_size(0), m_sz_enc(0) {
  }
  
  Ptr ptr() {
    return this;
  }

  virtual ~Read(){
    //std::cout << " ~CellStore::Block::Read\n";
  }
  
  State load() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_processing++;
    if(m_state == Block::Read::State::NONE) {
      m_state = Block::Read::State::LOADING;
      return Block::Read::State::NONE;
    }
    return m_state;
  }

  size_t processing() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_processing;
  }

  bool loaded() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_state == State::LOADED;
  }

  void load(FS::SmartFd::Ptr smartfd, std::function<void(int)> call) {
    int err = Error::OK;
    if(loaded()) {
      call(err);
      return;
    }
    
    load(err, smartfd);
    asio::post(*Env::IoCtx::io()->ptr(), [call, err](){ call(err); });
  }

  void load(int& err, FS::SmartFd::Ptr smartfd) {
    //std::cout << "CS::Read::load\n";

    for(;;) {
      err = Error::OK;

      if(!smartfd->valid() && !Env::FsInterface::interface()->open(err, smartfd) && err)
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
        m_cells_remain=m_cells_count = Serialization::decode_i32(&ptr, &remain);

        if(!checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain), buf, HEADER_SIZE-4)){  
          err = Error::CHECKSUM_MISMATCH;
          break;
        }
        m_loaded_header = true;
      }
      if(m_sz_enc == 0) // a zero cells type cs (initial of any to any block)
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
        StaticBuffer buffer(m_size);
        Encoder::decode(
          m_encoder, m_buffer.base, m_sz_enc, buffer.base, m_size, err);
        if(err) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, smartfd);
          break;
        }
        m_buffer.set(buffer);
      }

      break;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_state = !err ? State::LOADED : State::NONE;
  }

  void load_cells(CellsBlock::Ptr cells_block) {
    if(loaded()) {
      if(m_buffer.size)
        m_cells_remain -= cells_block->load_cells(
          m_buffer.base, m_buffer.size);
    } else {
      //err
    }

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_processing--; 
    }

    if(!m_cells_remain.load() || Env::Resources.need_ram(m_size))
      release();
  }
  
  size_t release() {    
    //std::cout << "CellStore::Block::release\n";  
    size_t released = 0;
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_processing || m_state != State::LOADED)
      return released; 

    released += m_buffer.size;    
    m_state = State::NONE;
    m_buffer.free();
    m_cells_remain = m_cells_count;
    return released;
  }

  void pending_load(std::function<void(int)> cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending_q.push(cb);
  }

  void pending_load(int& err) {
    std::function<void(int)> call;
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_pending_q.empty())
          return;
        call = m_pending_q.front();
      }

      call(err);
      
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending_q.pop();
      }
    }
  }

  size_t size_bytes(bool only_loaded=false) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(only_loaded && m_state != State::LOADED)
      return 0;
    return m_size;
  }

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string s("Block(offset=");
    s.append(std::to_string(offset));
    s.append(" state=");
    s.append(to_string(m_state));
    s.append(" size=");
    s.append(std::to_string(m_size));
    s.append(" ");
    s.append(interval.to_string());
    s.append(" queue=");
    s.append(std::to_string(m_pending_q.size()));
    s.append(" processing=");
    s.append(std::to_string(m_processing));
    s.append(")");
    return s;
  }

  private:
  
  std::mutex                            m_mutex;
  State                                 m_state;
  size_t                                m_processing;
  bool                                  m_loaded_header;
  Types::Encoding                       m_encoder;
  size_t                                m_size;
  size_t                                m_sz_enc;
  uint32_t                              m_cells_count;
  StaticBuffer                          m_buffer;
  std::queue<std::function<void(int)>>  m_pending_q;
  std::atomic<uint32_t>                 m_cells_remain;
};



class Write {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const size_t offset, const DB::Cells::Interval& interval, 
        const uint32_t cell_count)
        : offset(offset), interval(interval), cell_count(cell_count) { 
  }

  virtual ~Write(){}

  void write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
             DynamicBuffer& output) {
    
    size_t len_enc = 0;
    output.set_mark();
    Encoder::encode(encoder, cells.base, cells.fill(), 
                    &len_enc, output, HEADER_SIZE, err);
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