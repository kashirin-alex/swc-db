/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStoreBlock_h
#define swcdb_db_Files_CellStoreBlock_h

#include "swcdb/lib/core/Encoder.h"
#include "swcdb/lib/db/Cells/ReqScan.h"



namespace SWC { namespace Files { namespace CellStore {


namespace Block {

/* file-format: 
      header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
      data:   [cell]
*/

static const uint8_t HEADER_SIZE=17;



class Read {  
  public:
  typedef std::shared_ptr<Read> Ptr;

  enum State {
    NONE,
    CELLS_LOADED,
    LOGS_LOADING,
    LOGS_LOADED
  };

  inline static const std::string to_string(const State state) {
    switch(state) {
      case State::CELLS_LOADED:
        return "CELLS_LOADED";
      case State::LOGS_LOADED:
        return "LOGS_LOADED";
      case State::LOGS_LOADING:
        return "LOGS_LOADING";
      default:
        return "NONE";
    }
  }
  
  
  Read(const size_t offset, const DB::Cells::Interval& interval, 
       const DB::SchemaPtr s)
      : offset(offset),  
        cells(DB::Cells::Mutable(
          0, s->cell_versions, s->cell_ttl, s->col_type)),
        interval(interval), state(State::NONE) {
  }

  virtual ~Read(){}

  Ptr make_fresh(const DB::SchemaPtr schema) {
    return std::make_shared<Read>(offset, interval, schema);
  }

  void load(FS::SmartFdPtr smartfd, std::function<void(int)> call) {
    int err = Error::OK;
    if(state != State::NONE) {
      call(err);
      return;
    }
    //std::cout << "blk::load\n";

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
      Types::Encoding encoder 
        = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
      uint32_t sz_enc = Serialization::decode_i32(&ptr, &remain);
      uint32_t sz = Serialization::decode_i32(&ptr, &remain);
      if(!sz_enc) 
        sz_enc = sz;
      uint32_t cells_count = Serialization::decode_i32(&ptr, &remain);

      if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), buf, HEADER_SIZE-4)){  
        err = Error::CHECKSUM_MISMATCH;
        break;
      }
      if(sz_enc == 0)
        break;

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

      
      if(encoder != Types::Encoding::PLAIN) {
        StaticBuffer buffer(sz);
        Encoder::decode(encoder, read_buf.base, sz_enc, buffer.base, sz, err);
        if(err) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, smartfd);
          break;
        }
        read_buf.free();
        read_buf.base = buffer.base;
        buffer.own = false;
      }

      cells.free();
      cells.ensure(cells_count);

      load_cells(read_buf.base, read_buf.size);
      break;
    }
  
    if(err == Error::OK)
      state = State::CELLS_LOADED;
  
    call(err);
  }

  void load_cells(const uint8_t* ptr, size_t remain) {
    DB::Cells::Cell cell;

    auto ts = Time::now_ns();

    while(remain) {
      cell.read(&ptr, &remain);
      cells.push_back(cell);
    }

    auto took = Time::now_ns()-ts;
    std::cout << " cs-block-took=" << took
              << " avg=" << took / cells.size()
              << " " << cells.to_string() << "\n";
  }
  
  void add_logged(const DB::Cells::Cell& cell) {
    cells.add(cell);
  }

  void scan(DB::Cells::ReqScan::Ptr req) {
    //std::cout << "blk::scan 1 " << req->to_string() << "\n";

    int err;
    DB::Specs::Interval& spec = *(req->spec).get();
    size_t skips = 0;
    cells.scan(spec, req->cells, &req->offset, skips, req->selector);
    //req->adjust();
  }

  const size_t cell_count() {
    return cells.size();
  }

  void pending_logs_load(std::function<void()> cb) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pending_q.push(cb);
    std::cout << "pending_logs_load add, sz=" << m_pending_q.size() << "\n";
  }

  void pending_logs_load() {
    std::function<void()> call;
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_pending_q.empty())
          return;
        call = m_pending_q.front();
        
        std::cout << "pending_logs_load call, sz=" << m_pending_q.size() << "\n";
      }

      call();
      
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pending_q.pop();
      }
    }
  }

  const std::string to_string() {
    std::string s("Block(offset=");
    s.append(std::to_string(offset));
    s.append(" ");
    s.append(interval.to_string());

    s.append(" state=");
    s.append(to_string(state));

    s.append(" ");
    s.append(cells.to_string());
      
    if(cells.size() > 0) {
      DB::Cells::Cell cell;
      s.append(" range=(");
      cells.get(0, cell);
      s.append(cell.to_string());
      s.append(")<=cell<=(");
      cells.get(-1, cell);
      s.append(cell.to_string());
      s.append(")");
    }

    s.append(")");
    return s;
  }

  const size_t                    offset;
  const DB::Cells::Interval       interval;
  DB::Cells::Mutable              cells;
  std::atomic<State>              state;
  
  std::mutex                         m_mutex;
  std::queue<std::function<void()>>  m_pending_q;
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