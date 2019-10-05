/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStoreWrite_h
#define swcdb_db_Files_CellStoreWrite_h

namespace SWC { namespace Files {

class BlockWrite {  

  /* file-format: 
        header: i8(encoder), i32(enc-len), i32(len), i32(cells), i32(checksum)
        data:   [cell]
  */

  public:
  typedef std::shared_ptr<BlockWrite> Ptr;

  BlockWrite(const size_t offset, DB::Cells::Interval::Ptr interval, 
             uint32_t cell_count)
            : offset(offset), 
              interval(std::make_shared<DB::Cells::Interval>(interval)), 
              cell_count(cell_count) { 
  }

  virtual ~BlockWrite(){}

  void write(int& err, Types::Encoding encoder, DynamicBuffer& cells, 
             uint32_t cell_count, DynamicBuffer& output) {
    
    size_t len_enc = 0;
    output.set_mark();
    Encoder::encode(encoder, cells.base, cells.fill(), 
                    &len_enc, output, Block::HEADER_SIZE, err);
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



class CellStoreWrite : public std::enable_shared_from_this<CellStoreWrite> {

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

  typedef std::shared_ptr<CellStoreWrite> Ptr;

  FS::SmartFdPtr            smartfd;
  Types::Encoding           encoder;
  size_t                    size;
  DB::Cells::Interval::Ptr  interval;
  std::atomic<uint32_t>     complition;

  CellStoreWrite(const std::string& filepath, 
                  Types::Encoding encoder=Types::Encoding::PLAIN)
                : smartfd(FS::SmartFd::make_ptr(filepath, 0)), 
                  encoder(encoder), size(0), 
                  interval(nullptr), complition(0) {
  }

  virtual ~CellStoreWrite(){}

  void create(int& err, 
              int32_t bufsz=-1, int32_t replication=-1, int64_t blksz=1) {
    while(
      Env::FsInterface::interface()->create(
        err, smartfd, bufsz, replication, blksz));
  }

  void block(int& err, DB::Cells::Interval::Ptr blk_intval, 
             DynamicBuffer& cells, uint32_t cell_count) {

    BlockWrite::Ptr blk = std::make_shared<BlockWrite>(
      size, blk_intval, cell_count);
    
    DynamicBuffer buff_raw;
    blk->write(err, encoder, cells, cell_count, buff_raw);
    cells.free();
    blk_intval->free();
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
    bool init_interval = false;
    for(auto blk : m_blocks) {
      len_data += Serialization::encoded_length_vi32(blk->offset) 
          + blk->interval->encoded_length()
          + 4;;
      
      if(interval == nullptr)
        interval = std::make_shared<DB::Cells::Interval>();
      interval->expand(blk->interval, init_interval);
      init_interval = true;
    }

    StaticBuffer raw_buffer(len_data);
    uint8_t * ptr = raw_buffer.base;
    uint8_t * chk_ptr;
    for(auto blk : m_blocks) {
      chk_ptr = ptr;
      Serialization::encode_vi32(&ptr, blk->offset);
      blk->interval->encode(&ptr);
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

    StaticBuffer buff_write(CellStore::TRAILER_SIZE);
    uint8_t* ptr = buff_write.base;

    *ptr++ = CellStore::VERSION;
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


  const std::string to_string(){
    std::string s("CellStore(v=");
    s.append(std::to_string(CellStore::VERSION));
    s.append(" size=");
    s.append(std::to_string(size));
    s.append(" encoder=");
    s.append(Types::to_string(encoder));
    if(interval != nullptr) {
      s.append(" ");
      s.append(interval->to_string());
    }
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
  std::vector<BlockWrite::Ptr>   m_blocks;

  std::condition_variable        m_cv;
};


} // Files namespace

}
#endif