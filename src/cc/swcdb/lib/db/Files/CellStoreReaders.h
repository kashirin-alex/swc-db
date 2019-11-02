/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CellStoreReaders_h
#define swcdb_db_Files_CellStoreReaders_h

#include "CellStore.h"


namespace SWC { namespace Files { namespace CellStore {

class Readers {
  public:
  typedef Readers*  Ptr;

  inline static Ptr make(const DB::RangeBase::Ptr& range) { 
    return new Readers(range); 
  }

  DB::RangeBase::Ptr range;

  Readers(const DB::RangeBase::Ptr& range): range(range) {}

  virtual ~Readers() {
    //std::cout << " ~CellStore::Readers\n";
    wait_processing();
    free();
  }

  void add(Read::Ptr cs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cellstores.push_back(cs);
  }

  void expand(DB::Cells::Interval& interval) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for(const auto& cs : m_cellstores)
      interval.expand(cs->interval);
  }

  const bool empty() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cellstores.empty();
  }

  const size_t size() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cellstores.size();
  }

  const size_t size_bytes() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t  sz = 0;
    for(auto& cs : m_cellstores)
      sz += cs->size_bytes();
    return sz;
  }

  const size_t blocks_count() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t  sz = 0;
    for(auto& cs : m_cellstores)
      sz += cs->blocks_count();
    return sz;
  }

  const size_t release(size_t bytes) {
    size_t released = 0;
    return released;
  }

  const size_t processing() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return _processing();
  }

  void wait_processing() {
    while(processing() > 0)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  void clear() {
    wait_processing();
    std::lock_guard<std::mutex> lock(m_mutex);
    _close();
    free();
  }
  
  void remove(int &err) {
    wait_processing();
    std::lock_guard<std::mutex> lock(m_mutex);
    _close();
    for(auto& cs : m_cellstores)
      cs->remove(err);
    free();
  }
  
  template <class P>
  void iterate(P call) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::for_each(m_cellstores.begin(), m_cellstores.end(), call);
  }
  
  bool foreach(const DB::Cells::Interval& interval, 
               const std::function<void(const Read::Ptr&)>& call) {
    std::lock_guard<std::mutex> lock(m_mutex);
    bool expected = false;
    for(auto& cs : m_cellstores) {
      if(!cs->interval.consist(interval))
        continue;
      expected = true;
      call(cs);
    }
    return expected;
  }  
  
  void get_blocks(int& err, std::vector<Block::Read::Ptr>& blocks) {
    std::lock_guard<std::mutex> lock(m_mutex);
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
    std::lock_guard<std::mutex> lock(m_mutex);
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

  const std::string to_string() {
    std::lock_guard<std::mutex> lock(m_mutex);
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

    s.append(")");
    return s;
  }

  const size_t encoded_length() {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t sz = Serialization::encoded_length_vi32(m_cellstores.size());
    for(auto& cs : m_cellstores) {
      sz += Serialization::encoded_length_vi32(cs->id)
          + cs->interval.encoded_length();
    }
    return sz;
  }

  void encode(uint8_t** ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    Serialization::encode_vi32(ptr, m_cellstores.size());
    for(auto& cs : m_cellstores) {
      Serialization::encode_vi32(ptr, cs->id);
      cs->interval.encode(ptr);
    }
  }

  void decode(const uint8_t** ptr, size_t* remain) {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t id;
    uint32_t len = Serialization::decode_vi32(ptr, remain);
    for(size_t i=0;i<len;i++) {
      id = Serialization::decode_vi32(ptr, remain);
      m_cellstores.push_back(
        Read::make(id, range, DB::Cells::Interval(ptr, remain)));
    }
  }
  
  void load_from_path(int &err){
    wait_processing();

    std::lock_guard<std::mutex> lock(m_mutex);

    FS::IdEntries_t entries;
    Env::FsInterface::interface()->get_structured_ids(
      err, range->get_path(DB::RangeBase::cellstores_dir), entries);
  
    free();
    for(auto id : entries){
      m_cellstores.push_back(Read::make(id, range, DB::Cells::Interval()));
    }

    //sorted
  }

  void replace(int &err, Files::CellStore::Writers& w_cellstores) {
    wait_processing();    

    auto fs = Env::FsInterface::interface();
    std::lock_guard<std::mutex> lock(m_mutex);
    
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

    free();
    for(auto cs : w_cellstores) 
      m_cellstores.push_back(
        Files::CellStore::Read::make(cs->id, range, cs->interval)
      );
      // cs->load_blocks_index(err, true);
      // cs can be not loaded and will happen with 1st scan-req
    err = Error::OK;
  }
  
  private:
  

  void free() {
    for(auto& cs : m_cellstores)
      delete cs;
    m_cellstores.clear();
  }

  void _close() {
    int err = Error::OK;
    for(auto& cs : m_cellstores)
      cs->close(err);
  }

  const size_t _processing() {
    size_t size = 0;
    for(auto& cs : m_cellstores)
      size += cs->processing();
    return size;
  }

  std::mutex             m_mutex;
  std::vector<Read::Ptr> m_cellstores;
};

}}}

#endif