/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStore_h
#define swcdb_db_Files_CellStore_h

#include "swcdb/lib/db/Columns/Schema.h"
#include "CellStoreBlock.h"
#include "CommitLog.h"


namespace SWC { namespace Files {

namespace CellStore {

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


static const uint8_t  TRAILER_SIZE=9;
static const int8_t   VERSION=1;



class Read : public std::enable_shared_from_this<Read> {
  public:
  typedef std::shared_ptr<Read>   Ptr;

  enum State {
    BLKS_IDX_NONE,
    BLKS_IDX_LOADING,
    BLKS_IDX_LOADED,
  };

  const uint32_t            id;
  const DB::RangeBase::Ptr  range;
  DB::Cells::Interval       interval;
  FS::SmartFdPtr            smartfd;

  Read(const uint32_t id, const DB::RangeBase::Ptr& range, 
       const DB::Cells::Interval& interval) 
      : id(id), range(range), interval(interval),
        smartfd(FS::SmartFd::make_ptr(range->get_path_cs(id), 0)), 
        m_state(State::BLKS_IDX_NONE) {   
  }

  virtual ~Read(){}

  State load_blocks_index(int& err, bool close_after=false) {
    //std::cout << "cs::load_blocks_index\n";
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

  void scan(DB::Cells::ReqScan::Ptr req) {
    //std::cout << "cs::Scan\n";
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
    //std::cout << "cs::run_queued\n";
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
  
  void set(CommitLog::Fragments::Ptr log) {
    m_commit_log = log;
  }

  void scan_block(uint32_t idx, DB::Cells::ReqScan::Ptr req) {
    //std::cout << "cs::scan_block\n";
    Block::Read::Ptr blk;
    for(; idx < m_blocks.size(); idx++) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        blk = m_blocks[idx];
      }

      if(!blk->interval.includes(req->spec))
        continue;
    
      if(blk->state == Block::Read::State::NONE) {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_blocks_q.push(
            [blk, req, idx, ptr=shared_from_this()](){
              blk->load(
                ptr->smartfd, 
                [req, idx, ptr](int err){
                  if(err)
                    req->response(err);
                  else
                    ptr->scan_block(idx, req);
                }
              );
            }
          );
        }
        run_queued();
        return;  

      } else if(blk->state == Block::Read::State::CELLS_LOADED) {
        blk->state = Block::Read::State::LOGS_LOADING;
        m_commit_log->load_to_block(
          blk, 
          [req, idx, ptr=shared_from_this()](int err){
            if(err)
              req->response(err);
            else
              ptr->scan_block(idx, req);
          }
        );
        return; 

      } else if(blk->state == Block::Read::State::LOGS_LOADING) {
        blk->pending_logs_load(
          [req, idx, ptr=shared_from_this()](){
            ptr->scan_block(idx, req);
        });
        return; 
      }
      
      blk->scan(req);
      if(req->reached_limits())
        break;
    }
    
    req->response(Error::OK);
  }

  bool add_logged(const DB::Cells::Cell& cell) {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state != State::BLKS_IDX_LOADED)
        return false;
    }
    if(!interval.consist(cell.key))
      return false;

    Block::Read::Ptr blk;
    for(uint32_t idx=0; idx < m_blocks.size(); idx++) {
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_blocks[idx]->state != Block::Read::State::LOGS_LOADED)
          continue;
        blk = m_blocks[idx];
      }
      if(blk->interval.consist(cell.key)) {
        blk->add_logged(cell);
        if(!cell.on_fraction)
          return true;
      }
    }
    return false;
  }

  size_t release(size_t bytes) {    
    size_t released = 0;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(m_state != State::BLKS_IDX_LOADED)
        return released;
    }

    size_t used;
    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);
    for(uint32_t idx = m_blocks.size(); --idx >= 0;) {
      std::lock_guard<std::mutex> lock(m_mutex);

      if(m_blocks[idx]->state == Block::Read::State::NONE)
        continue;

      used = m_blocks[idx]->cells.size();
      if(!used)
        continue;

      released += used;
      m_blocks[idx] = m_blocks[idx]->make_fresh(schema);
      if(released >= bytes)
        break;
    }
    return released;
  }

  void close(int &err) {
    if(smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd); 
  }

  void remove(int &err) {
    Env::FsInterface::fs()->remove(err, smartfd->filepath()); 
  }

  const size_t cell_count() {
    size_t count = 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto& blk : m_blocks)
      count += blk->cell_count();
    return count;
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("Read(v=");
    s.append(std::to_string(VERSION));
    s.append(" id=");
    s.append(std::to_string(id));
    s.append(" state=");
    s.append(std::to_string((uint8_t) m_state));
    s.append(" ");
    s.append(interval.to_string());

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
          close(err);
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

    if(close_after)
      close(err);
    return state;
  }

  void _load_blocks_index(int& err, bool close_after=false) {
    //std::cout << "cs::_load_blocks_index 1 \n";
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
        close(tmperr);
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
    Types::Encoding encoder 
      = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
    uint32_t sz_enc = Serialization::decode_i32(&ptr, &remain);
    uint32_t sz = Serialization::decode_vi32(&ptr, &remain);
    uint32_t blks_count = Serialization::decode_vi32(&ptr, &remain);
    
    if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), 
        read_buf.base, ptr-read_buf.base)
      ) {
      close(err);
      err = Error::CHECKSUM_MISMATCH;
      return;
    }
    
    if(encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(sz);
      Encoder::decode(encoder, ptr, sz_enc, decoded_buf.base, sz, err);
      if(err) {
        int tmperr = Error::OK;
        close(tmperr);
        return;
      }
      read_buf.free();
      ptr = decoded_buf.base;
      remain = sz;
    }

    const uint8_t* chk_ptr;
    interval.free();

    Block::Read::Ptr blk;
    DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);

    for(int n = 0; n < blks_count; n++){
      chk_ptr = ptr;

      uint32_t offset = Serialization::decode_vi32(&ptr, &remain);
      blk = std::make_shared<Block::Read>(
        offset, 
        DB::Cells::Interval(&ptr, &remain), 
        schema
      );  
      if(!checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain), chk_ptr, ptr-chk_ptr)) {
        close(err);
        err = Error::CHECKSUM_MISMATCH;
        return;
      }

      m_blocks.push_back(blk);
      interval.expand(blk->interval);
    }
    
    if(close_after)
      close(err);
  }

  void _run_queued() {
    //std::cout << "cs::_run_queued\n";
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
          int tmperr = Error::OK;
          close(tmperr);
          return;
        }
      }
    }
  }

  std::mutex                          m_mutex;
  std::atomic<State>                  m_state;
  std::vector<Block::Read::Ptr>       m_blocks;
  bool                                m_blocks_q_runs = false;
  std::queue<std::function<void()>>   m_blocks_q;
  CommitLog::Fragments::Ptr           m_commit_log;

};
typedef std::vector<Read::Ptr>    Readers;
typedef std::shared_ptr<Readers>  ReadersPtr;




class Write : public std::enable_shared_from_this<Write> {
  public:
  typedef std::shared_ptr<Write>  Ptr;

  FS::SmartFdPtr            smartfd;
  Types::Encoding           encoder;
  size_t                    size;
  DB::Cells::Interval       interval;
  std::atomic<uint32_t>     complition;

  Write(const std::string& filepath, 
        Types::Encoding encoder=Types::Encoding::PLAIN)
        : smartfd(FS::SmartFd::make_ptr(filepath, 0)), 
          encoder(encoder), size(0), complition(0) {
  }

  virtual ~Write(){}

  void create(int& err, 
              int32_t bufsz=-1, int32_t replication=-1, int64_t blksz=1) {
    while(
      Env::FsInterface::interface()->create(
        err, smartfd, bufsz, replication, blksz));
  }

  void block(int& err, const DB::Cells::Interval& blk_intval, 
             DynamicBuffer& cells_buff, uint32_t cell_count) {

    Block::Write::Ptr blk = std::make_shared<Block::Write>(
      size, blk_intval, cell_count);
    
    DynamicBuffer buff_raw;
    blk->write(err, encoder, cells_buff, buff_raw);
    if(err)
      return;
    
    m_blocks.push_back(blk);
    
    StaticBuffer buff_write(buff_raw);
    size += buff_write.size;

    complition++;
    Env::FsInterface::fs()->append(
      err,
      smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
    complition--;
  }

  uint32_t write_blocks_index(int& err) {
    if(complition > 0){
      std::unique_lock<std::mutex> lock_wait(m_mutex);
      m_cv.wait(lock_wait, [count=&complition]{return count == 0;});
    }

    uint32_t len_data = 0;
    interval.free();
    for(auto blk : m_blocks) {
      len_data += Serialization::encoded_length_vi32(blk->offset) 
          + blk->interval.encoded_length()
          + 4;;

      interval.expand(blk->interval);
    }

    StaticBuffer raw_buffer(len_data);
    uint8_t * ptr = raw_buffer.base;
    uint8_t * chk_ptr;
    for(auto blk : m_blocks) {
      chk_ptr = ptr;
      Serialization::encode_vi32(&ptr, blk->offset);
      blk->interval.encode(&ptr);
      checksum_i32(chk_ptr, ptr, &ptr);
    }
    
    uint32_t len_header = 9 + Serialization::encoded_length_vi32(len_data) 
                + Serialization::encoded_length_vi32(m_blocks.size());

    DynamicBuffer buffer_write;
    size_t len_enc = 0;
    Encoder::encode(encoder, raw_buffer.base, len_data, 
                    &len_enc, buffer_write, len_header, err);
    raw_buffer.free();
    if(err)
      return 0;

    uint8_t* header_ptr = buffer_write.base;
    *header_ptr++ = (uint8_t)(len_enc? encoder: Types::Encoding::PLAIN);
    Serialization::encode_i32(&header_ptr, len_enc);
    Serialization::encode_vi32(&header_ptr, len_data);
    Serialization::encode_vi32(&header_ptr, m_blocks.size());
    checksum_i32(buffer_write.base, header_ptr, &header_ptr);

    StaticBuffer buff_write(buffer_write);
    Env::FsInterface::fs()->append(
      err,
      smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
    if(err)
      return buff_write.size;

    size += buff_write.size;
    return buff_write.size;
  }

  void write_trailer(int& err) {
    uint32_t blk_idx_sz = write_blocks_index(err);
    if(err)
      return;

    StaticBuffer buff_write(TRAILER_SIZE);
    uint8_t* ptr = buff_write.base;

    *ptr++ = VERSION;
    Serialization::encode_i32(&ptr, blk_idx_sz);
    checksum_i32(buff_write.base, ptr, &ptr);
    
    Env::FsInterface::fs()->append(
      err,
      smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
    
    size += buff_write.size;
  }

  void close_and_validate(int& err) {
    Env::FsInterface::fs()->close(err, smartfd);

    if(Env::FsInterface::fs()->length(err, smartfd->filepath()) != size)
      err = Error::FS_EOF;
    // + trailer-checksum
  }

  void finalize(int& err) {
    write_trailer(err);
    if(!err) {
      close_and_validate(err);
      if(!err)
        return;
    }
    int tmperr = Error::OK;
    remove(tmperr);
  } 

  void remove(int &err) {
    Env::FsInterface::fs()->remove(err, smartfd->filepath()); 
  }

  const std::string to_string() {
    std::string s("CellStore(v=");
    s.append(std::to_string(CellStore::VERSION));
    s.append(" size=");
    s.append(std::to_string(size));
    s.append(" encoder=");
    s.append(Types::to_string(encoder));
    s.append(" ");
    s.append(interval.to_string());
    s.append(" ");
    s.append(smartfd->to_string());

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

  std::mutex                     m_mutex;
  std::vector<Block::Write::Ptr> m_blocks;

  std::condition_variable        m_cv;
};

typedef std::vector<Write::Ptr>   Writers;
typedef std::shared_ptr<Writers>  WritersPtr;




inline static Read::Ptr create_init_read(int& err, Types::Encoding encoding, 
                                         DB::RangeBase::Ptr range) {
  Write writer(range->get_path_cs(1), encoding);
  writer.create(err);
  if(err)
    return nullptr;
    
  DB::Cells::Interval interval;
  range->get_interval(interval);

  DynamicBuffer cells_buff;
  writer.block(err, interval, cells_buff, 0);
  if(!err) {
    writer.finalize(err);
    if(!err) {
      Read::Ptr cs = std::make_shared<Read>(1, range, interval);
      cs->load_blocks_index(err, true);
      if(!err)
        return cs;
    }
  }
  int errtmp;
  writer.remove(errtmp);
  return nullptr;
};

} // namespace CellStore

}}

#endif