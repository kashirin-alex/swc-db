/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStore_h
#define swcdb_db_Files_CellStore_h

#include "swcdb/db/Files/CellStoreBlock.h"


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



class Read final {
  public:
  typedef Read*  Ptr;

  inline static Ptr make(int& err, const uint32_t id, 
                         const DB::RangeBase::Ptr& range, 
                         const DB::Cells::Interval& interval) {
    auto smartfd = FS::SmartFd::make_ptr(range->get_path_cs(id), 0);
    DB::Cells::Interval interval_by_blks;
    std::vector<Block::Read::Ptr> blocks;
    
    load_blocks_index(err, smartfd, interval_by_blks, blocks);
    if(err)
      SWC_LOGF(LOG_ERROR, 
        "CellStore load_blocks_index err=%d(%s) %s (id=%d %s %s", 
        err, Error::get_text(err), id, range->to_string().c_str(), 
        interval.to_string().c_str());
    return new Read(
      id, 
      interval_by_blks.was_set ? interval_by_blks : interval, 
      blocks, 
      smartfd);
  }

  static bool load_trailer(int& err, FS::SmartFd::Ptr smartfd, 
                           size_t& blks_idx_size, 
                           size_t& blks_idx_offset, 
                           bool close_after=false) {
    bool loaded;
    for(;;) {
      err = Error::OK;
      loaded = false;
    
      if(!Env::FsInterface::fs()->exists(err, smartfd->filepath())) {
        if(err != Error::OK && err != Error::SERVER_SHUTTING_DOWN)
          continue;
        return loaded;
      }
      size_t length = Env::FsInterface::fs()->length(err, smartfd->filepath());
      if(err) {
        if(err == Error::FS_PATH_NOT_FOUND ||
           err == Error::FS_PERMISSION_DENIED ||
           err == Error::SERVER_SHUTTING_DOWN)
          return loaded;
        continue;
      }
      
      if(!Env::FsInterface::interface()->open(err, smartfd) && err)
        return loaded;
      if(err)
        continue;
      
      uint8_t buf[TRAILER_SIZE];
      const uint8_t *ptr = buf;
      if(Env::FsInterface::fs()->pread(
                err, smartfd, length-TRAILER_SIZE, buf, TRAILER_SIZE)
              != TRAILER_SIZE){
        if(err != Error::FS_EOF){
          Env::FsInterface::interface()->close(err, smartfd); 
          continue;
        }
        return loaded;
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
      loaded = true;
      break;
    }

    if(close_after)
      Env::FsInterface::interface()->close(err, smartfd); 
    return loaded;
  }

  static void load_blocks_index(int& err, FS::SmartFd::Ptr smartfd, 
                                DB::Cells::Interval& interval,
                                std::vector<Block::Read::Ptr>& blocks) {

    size_t blks_idx_size = 0;
    size_t offset = 0;
    if(!load_trailer(err, smartfd, blks_idx_size, offset, false))
      return;

    StaticBuffer read_buf;
    for(;;) {
      read_buf.free();
      err = Error::OK;
      if(Env::FsInterface::fs()->pread(
                          err, smartfd, offset, &read_buf, blks_idx_size)
                != blks_idx_size){
        int tmperr = Error::OK;
        Env::FsInterface::interface()->close(tmperr, smartfd); 
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

    size_t remain = blks_idx_size;
    Types::Encoding encoder 
      = (Types::Encoding)Serialization::decode_i8(&ptr, &remain);
    uint32_t sz_enc = Serialization::decode_i32(&ptr, &remain);
    uint32_t sz = Serialization::decode_vi32(&ptr, &remain);
    uint32_t blks_count = Serialization::decode_vi32(&ptr, &remain);
    
    if(!checksum_i32_chk(
        Serialization::decode_i32(&ptr, &remain), 
        read_buf.base, ptr-read_buf.base)
      ) {
      Env::FsInterface::interface()->close(err, smartfd); 
      err = Error::CHECKSUM_MISMATCH;
      return;
    }
    
    if(encoder != Types::Encoding::PLAIN) {
      StaticBuffer decoded_buf(sz);
      Encoder::decode(err, encoder, ptr, sz_enc, decoded_buf.base, sz);
      if(err) {
        int tmperr = Error::OK;
        Env::FsInterface::interface()->close(tmperr, smartfd); 
        return;
      }
      read_buf.set(decoded_buf);
      ptr = read_buf.base;
      remain = sz;
    }

    const uint8_t* chk_ptr;

    Block::Read::Ptr blk;
    for(int n = 0; n < blks_count; n++) {
      chk_ptr = ptr;

      uint32_t offset = Serialization::decode_vi32(&ptr, &remain);
      blk = Block::Read::make(
        offset, 
        DB::Cells::Interval(&ptr, &remain)
      );  
      if(!checksum_i32_chk(
          Serialization::decode_i32(&ptr, &remain), chk_ptr, ptr-chk_ptr)) {
        Env::FsInterface::interface()->close(err, smartfd); 
        err = Error::CHECKSUM_MISMATCH;
        return;
      }

      blocks.push_back(blk);
      interval.expand(blk->interval);
    }

    Env::FsInterface::interface()->close(err, smartfd); 
  }

  // 
  
  const uint32_t                      id;
  const DB::Cells::Interval           interval;
  const std::vector<Block::Read::Ptr> blocks;
  
  explicit Read(const uint32_t id,
                const DB::Cells::Interval& interval, 
                const std::vector<Block::Read::Ptr>& blocks,
                FS::SmartFd::Ptr smartfd) 
                : id(id), 
                  interval(interval), 
                  blocks(blocks), 
                  m_smartfd(smartfd) {       
  }

  Ptr ptr() {
    return this;
  }

  ~Read() {
    for(auto blk : blocks)
      delete blk;
  }

  void load_cells(DB::Cells::Block::Ptr cells_block, 
                  const std::function<void(int)>& cb) {

    std::vector<Block::Read::Ptr>  applicable;
    for(auto blk : blocks) {  
      if(cells_block->is_consist(blk->interval))
        applicable.push_back(blk);
      else if(!blk->interval.key_end.empty() && 
              !cells_block->is_in_end(blk->interval.key_end))
        break;
    }

    if(applicable.empty()) {
      cb(Error::OK);
      return;
    }
  
    auto waiter = new AwaitingLoad(applicable.size(), cells_block, cb);
    for(auto blk : applicable) {
      auto cb = [blk, waiter](){ waiter->processed(blk); };
      if(blk->load(cb)) {
        std::scoped_lock lock(m_mutex);
        m_queue.push([blk, cb, fd=m_smartfd](){ blk->load(fd, cb); });
      }
    }

    run_queued();
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

  void get_blocks(int& err, std::vector<Block::Read::Ptr>& to) const {
    for(auto blk : blocks)
      to.push_back(blk);
  }

  size_t release(size_t bytes) {   
    size_t released = 0;
    for(auto blk : blocks) {
      released += blk->release();
      if(bytes && released >= bytes)
        break;
    }
    return released;
  }

  void close(int &err) {
    Env::FsInterface::interface()->close(err, m_smartfd); 
  }

  void remove(int &err) {
    Env::FsInterface::interface()->remove(err, m_smartfd->filepath());
  } 

  const bool processing() {
    std::shared_lock lock(m_mutex);
    return _processing();
  }

  const size_t size_bytes(bool only_loaded=false) const {
    size_t size = 0;
    for(auto blk : blocks)
      size += blk->size_bytes(only_loaded);
    return size;
  }

  const size_t blocks_count() const {
    return blocks.size();
  }

  const std::string to_string() {
    std::scoped_lock lock(m_mutex);

    std::string s("Read(v=");
    s.append(std::to_string(VERSION));
    s.append(" id=");
    s.append(std::to_string(id));
    s.append(" ");
    
    s.append(interval.to_string());

    if(m_smartfd != nullptr) {
      s.append(" file=");
      s.append(m_smartfd->filepath());
    }

    s.append(" blocks=");
    s.append(std::to_string(blocks_count()));
    s.append(" blocks=[");
    for(auto blk : blocks) {
      s.append(blk->to_string());
      s.append(", ");
    }
    s.append("]");

    s.append(" queue=");
    s.append(std::to_string(m_queue.size()));

    s.append(" processing=");
    s.append(std::to_string(_processing()));

    s.append(" used/actual=");
    s.append(std::to_string(size_bytes(true)));
    s.append("/");
    s.append(std::to_string(size_bytes()));

    s.append(")");
    return s;
  } 

  private:

  const bool _processing() const {
    if(m_queue.size())
      return true;
    for(auto blk : blocks)
      if(blk->processing())
        return true;
    return false;
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
          int tmperr = Error::OK;
          close(tmperr);
          return;
        }
      }
    }
  }

  struct AwaitingLoad final {
    public:
    typedef std::function<void(int)>  Cb_t;
    
    AwaitingLoad(int32_t count, DB::Cells::Block::Ptr cells_block, 
                  const Cb_t& cb) 
                : m_count(count), cells_block(cells_block), cb(cb), 
                  m_err(Error::OK) {
    }

    ~AwaitingLoad() { }

    void processed(Block::Read::Ptr blk) {
      { 
        std::lock_guard<std::mutex> lock(m_mutex_await);
        m_count--;
        SWC_LOGF(LOG_DEBUG, " CellStore::AwaitingLoad count=%d", m_count);
        m_pending.push(blk);
        if(m_pending.size() > 1)
          return;
      }
      
      asio::post(
        *Env::IoCtx::io()->ptr(), 
        [this](){ _processed(); }
      );
    }

    void _processed() {
      int err;
      for(Block::Read::Ptr blk;;) {
        {
          std::lock_guard<std::mutex> lock(m_mutex_await);
          blk = m_pending.front();
        }

        blk->load_cells(err, cells_block);
        if(err) {
          std::scoped_lock<std::mutex> lock(m_mutex_await);
          m_err = err;
        }

        {
          std::lock_guard<std::mutex> lock(m_mutex_await);
          m_pending.pop();
          if(m_pending.empty()) {
            if(m_count)
              return;
            break;
          }
        }
      }
      cb(m_err);
      delete this;
    }

    std::mutex                    m_mutex_await;
    int32_t                       m_count;
    DB::Cells::Block::Ptr         cells_block;
    const Cb_t                    cb;
    int                           m_err;
    std::queue<Block::Read::Ptr>  m_pending;
  };

  std::shared_mutex                   m_mutex;
  bool                                m_q_runs = false;
  std::queue<std::function<void()>>   m_queue;
  FS::SmartFd::Ptr                    m_smartfd;

};



class Write : public std::enable_shared_from_this<Write> {
  public:
  typedef std::shared_ptr<Write>  Ptr;

  const uint32_t            id;
  FS::SmartFd::Ptr          smartfd;
  Types::Encoding           encoder;
  size_t                    size;
  DB::Cells::Interval       interval;
  //std::atomic<uint32_t>     completion;
 
  Write(const uint32_t id, const std::string& filepath, 
        Types::Encoding encoder=Types::Encoding::PLAIN)
        : id(id), 
          smartfd(
            FS::SmartFd::make_ptr(filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE)
          ), 
          encoder(encoder), size(0) { //, completion(0)
  }

  virtual ~Write() { }

  void create(int& err, 
              int32_t bufsz=-1, uint8_t blk_replicas=0, int64_t blksz=-1) {
    while(
      Env::FsInterface::interface()->create(
        err, smartfd, bufsz, blk_replicas, blksz));
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

    Env::FsInterface::fs()->append(
      err,
      smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
  }

  uint32_t write_blocks_index(int& err) {
    uint32_t len_data = 0;
    interval.free();
    for(auto blk : m_blocks) {
      len_data += Serialization::encoded_length_vi32(blk->offset) 
                + blk->interval.encoded_length()
                + 4;
                
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
    Encoder::encode(err, encoder, raw_buffer.base, len_data, 
                    &len_enc, buffer_write, len_header);
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
    if(smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd); 
    Env::FsInterface::fs()->remove(err, smartfd->filepath()); 
  }

  const std::string to_string() {
    std::string s("Write(v=");
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

  std::vector<Block::Write::Ptr> m_blocks;
};

typedef std::vector<Write::Ptr>   Writers;
typedef std::shared_ptr<Writers>  WritersPtr;




inline static Read::Ptr create_init_read(int& err, Types::Encoding encoding, 
                                         DB::RangeBase::Ptr range) {
  Write writer(1, range->get_path_cs(1), encoding);
  writer.create(
    err, -1, range->cfg->block_replication(), range->cfg->block_size());
  if(err)
    return nullptr;
    
  DB::Cells::Interval interval;
  range->get_interval(interval);

  DynamicBuffer cells_buff;
  writer.block(err, interval, cells_buff, 0);
  if(!err) {
    writer.finalize(err);
    if(!err) {
      auto cs = Read::make(err, 1, range, writer.interval);
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