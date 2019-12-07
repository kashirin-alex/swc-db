/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStoreReaders_h
#define swcdb_db_Files_CellStoreReaders_h

#include "swcdb/db/Files/CellStore.h"


namespace SWC { namespace Files { namespace CellStore {

class Readers final {
  public:
  typedef Readers*  Ptr;

  DB::RangeBase::Ptr range;

  Readers() {}

  void init(DB::RangeBase::Ptr for_range) {
    range = for_range;
  }

  ~Readers() {
    _free();
  }

  void add(Read::Ptr cs) {
    std::lock_guard lock(m_mutex);
    m_cellstores.push_back(cs);
  }

  void expand(DB::Cells::Interval& interval) {
    std::shared_lock lock(m_mutex);
    for(const auto& cs : m_cellstores)
      interval.expand(cs->interval);
  }

  const bool empty() {
    std::shared_lock lock(m_mutex);
    return m_cellstores.empty();
  }

  const size_t size() {
    std::shared_lock lock(m_mutex);
    return m_cellstores.size();
  }

  const size_t size_bytes(bool only_loaded=false) {
    std::shared_lock lock(m_mutex);
    return _size_bytes(only_loaded);
  }

  const size_t blocks_count() {
    size_t  sz = 0;
    std::shared_lock lock(m_mutex);
    for(auto& cs : m_cellstores)
      sz += cs->blocks_count();
    return sz;
  }

  const size_t release(size_t bytes) {    
    size_t released = 0;
    std::shared_lock lock(m_mutex);

    for(auto& cs : m_cellstores) {
      released += cs->release(bytes ? bytes-released : bytes);
      if(bytes && released >= bytes)
        break;
    }
    return released;
  }

  const bool processing() {
    std::shared_lock lock(m_mutex);
    return _processing();
  }

  void wait_processing() {
    while(processing() > 0)  {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  void clear() {
    wait_processing();
    std::lock_guard lock(m_mutex);
    _close();
    _free();
  }
  
  void remove(int &err) {
    wait_processing();
    std::lock_guard lock(m_mutex);
    _close();
    for(auto& cs : m_cellstores)
      cs->remove(err);
    _free();
  }
  
  void load_cells(DB::Cells::Block::Ptr cells_block) {

    std::vector<Files::CellStore::Read::Ptr> cellstores;
    {
      std::shared_lock lock(m_mutex);
      for(auto& cs : m_cellstores) {
        if(cells_block->is_consist(cs->interval))
          cellstores.push_back(cs);
      }
    }

    if(cellstores.empty()) {
      cells_block->loaded_cellstores(Error::OK);
      return;
    }
    
    auto waiter = new AwaitingLoad(cellstores.size(), cells_block);
    for(auto& cs : cellstores)
      cs->load_cells(cells_block, [waiter](int err){ waiter->processed(err); });   
  }
  
  void get_blocks(int& err, std::vector<Block::Read::Ptr>& blocks) {
    std::lock_guard lock(m_mutex);
    for(auto& cs : m_cellstores) {
      if(cs->blocks.empty()) {
        cs->load_blocks_index(err, true);
        if(err) 
          return;
      }
      for(auto& blk : cs->blocks)
        blocks.push_back(blk);
    }
    return;
  }

  const bool need_compaction(size_t cs_sz, size_t blk_sz) {
    std::shared_lock lock(m_mutex);
    size_t  sz;
    for(auto& cs : m_cellstores) {
      sz = cs->size_bytes();
      if(!sz)
        continue;
      if(sz >= cs_sz || sz/cs->blocks_count() >= blk_sz) //or by max_blk_sz
        return true;
    }
    return false;
  }

  const size_t encoded_length() {
    std::shared_lock lock(m_mutex);
    size_t sz = Serialization::encoded_length_vi32(m_cellstores.size());
    for(auto& cs : m_cellstores) {
      sz += Serialization::encoded_length_vi32(cs->id)
          + cs->interval.encoded_length();
    }
    return sz;
  }

  void encode(uint8_t** ptr) {
    std::shared_lock lock(m_mutex);
    Serialization::encode_vi32(ptr, m_cellstores.size());
    for(auto& cs : m_cellstores) {
      Serialization::encode_vi32(ptr, cs->id);
      cs->interval.encode(ptr);
    }
  }

  void decode(const uint8_t** ptr, size_t* remain) {
    std::lock_guard lock(m_mutex);
    uint32_t id;
    uint32_t len = Serialization::decode_vi32(ptr, remain);
    for(size_t i=0;i<len;i++) {
      id = Serialization::decode_vi32(ptr, remain);
      m_cellstores.push_back(
        Read::make(id, range, DB::Cells::Interval(ptr, remain)));
    }
  }
  
  void load_from_path(int &err) {
    wait_processing();

    std::lock_guard lock(m_mutex);

    FS::IdEntries_t entries;
    Env::FsInterface::interface()->get_structured_ids(
      err, range->get_path(DB::RangeBase::cellstores_dir), entries);
  
    _free();
    for(auto id : entries){
      m_cellstores.push_back(Read::make(id, range, DB::Cells::Interval()));
    }

    //sorted
  }

  void replace(int &err, Files::CellStore::Writers& w_cellstores) {
    wait_processing();    

    auto fs = Env::FsInterface::interface();
    std::lock_guard lock(m_mutex);
    
    _close();

    fs->rename(
      err, 
      range->get_path(DB::RangeBase::cellstores_dir), 
      range->get_path(DB::RangeBase::cellstores_bak_dir)
    );
    if(err)
      return;

    fs->rename(
      err, 
      range->get_path(DB::RangeBase::cellstores_tmp_dir), 
      range->get_path(DB::RangeBase::cellstores_dir)
    );

    if(err) {
      fs->rmdir(err, range->get_path(DB::RangeBase::cellstores_dir));
      fs->rename(
        err, 
        range->get_path(DB::RangeBase::cellstores_bak_dir), 
        range->get_path(DB::RangeBase::cellstores_dir)
      );
      return;
    }
    fs->rmdir(err, range->get_path(DB::RangeBase::cellstores_bak_dir));

    _free();
    for(auto cs : w_cellstores) {
      m_cellstores.push_back(
        Files::CellStore::Read::make(cs->id, range, cs->interval)
      );
      // m_cellstores.back()->load_blocks_index(err, true);
      // cs can be not loaded and will happen with 1st scan-req
    }
    err = Error::OK;
  }
  
  void free() {
    wait_processing();
    std::lock_guard lock(m_mutex);
    _close();
    _free();
    range = nullptr;
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);

    std::string s("CellStores(count=");
    s.append(std::to_string(m_cellstores.size()));

    s.append(" cellstores=[");
    for(auto& cs : m_cellstores) {
      s.append(cs->to_string());
      s.append(", ");
    }
    s.append("] ");

    s.append(" processing=");
    s.append(std::to_string(_processing()));

    s.append(" used/actual=");
    s.append(std::to_string(_size_bytes(true)));
    s.append("/");
    s.append(std::to_string(_size_bytes()));

    s.append(")");
    return s;
  }

  private:
  

  void _free() {
    for(auto& cs : m_cellstores) {
      cs->free();
      delete cs;
    }
    m_cellstores.clear();
  }

  void _close() {
    int err = Error::OK;
    for(auto& cs : m_cellstores)
      cs->close(err);
  }

  const bool _processing() {
    for(auto& cs : m_cellstores)
      if(cs->processing())
        return true;
    return false;
  }
  
  const size_t _size_bytes(bool only_loaded=false) {
    size_t  sz = 0;
    for(auto& cs : m_cellstores)
      sz += cs->size_bytes(only_loaded);
    return sz;
  }


  struct AwaitingLoad final {
    public:
    
    AwaitingLoad(int32_t count, const DB::Cells::Block::Ptr& cells_block) 
                 : count(count), cells_block(cells_block) {
    }

    ~AwaitingLoad() { }

    void processed(int err) {      
      if(--count)
        return;
      cells_block->loaded_cellstores(err);
      delete this;
    }

    std::atomic<int32_t>        count;
    const DB::Cells::Block::Ptr cells_block;
  };

  std::shared_mutex       m_mutex;
  std::vector<Read::Ptr>  m_cellstores;
};

}}}

#endif