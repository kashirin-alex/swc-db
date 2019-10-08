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
  
  Read(const size_t offset, DB::Cells::Interval::Ptr interval, 
        DB::Cells::Mutable::Ptr log_cells = nullptr)
        : offset(offset), loaded(false), interval(interval), log_cells(log_cells) { 
  }

  virtual ~Read(){}

  Ptr make_fresh() {
    return std::make_shared<Read>(offset, interval, log_cells);
  }

  void load(FS::SmartFdPtr smartfd, std::function<void()> call) {
    if(loaded) {
      call();
      return;
    }
    //std::cout << "blk::load\n";

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
      Types::Encoding encoder 
        = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
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

      if(buffer == nullptr)
        buffer = std::make_shared<StaticBuffer>(sz);
      
      if(encoder != Types::Encoding::PLAIN) {
        Encoder::decode(encoder, read_buf.base, sz_enc, buffer->base, sz, err);
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
  
  void scan(DB::Cells::ReqScan::Ptr req) {
    //std::cout << "blk::scan 1 " << req->to_string() << "\n";
    
    int err;
    DB::Specs::Interval& spec = *(req->spec).get();
    DB::Cells::Mutable& cells = *(req->cells).get();

    log_cells->scan(spec, req->cells);
    if(spec.flags.limit == cells.size() || buffer == nullptr)
      return;

    DB::Cells::Cell cell;
    const uint8_t* ptr = buffer->base;
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

    s.append(" size=");
    s.append(std::to_string(buffer==nullptr?0:buffer->size));

    if(log_cells != nullptr) {
      s.append(" log=");
      s.append(log_cells->to_string());
      
      if(log_cells->size() > 0) {
        DB::Cells::Cell cell;
        s.append(" range=(");
        log_cells->get(0, cell);
        s.append(cell.to_string());
        s.append(")<=cell<=(");
        log_cells->get(-1, cell);
        s.append(cell.to_string());
        s.append(")");
      }
    }
    
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



class Write {  
  public:
  typedef std::shared_ptr<Write> Ptr;

  Write(const size_t offset, DB::Cells::Interval::Ptr interval, 
        uint32_t cell_count)
        : offset(offset), 
          interval(std::make_shared<DB::Cells::Interval>(interval)), 
          cell_count(cell_count) { 
  }

  virtual ~Write(){}

  void write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
             uint32_t cell_count, DynamicBuffer& output) {
    
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
    s.append(interval->to_string());
    s.append(")");
    return s;
  }

  const size_t              offset;
  DB::Cells::Interval::Ptr  interval;
  uint32_t                  cell_count;

};



} // Block namespace


}}}

#endif