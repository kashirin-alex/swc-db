/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_ranger_db_CellStoreReaders_h
#define swc_ranger_db_CellStoreReaders_h

#include "swcdb/ranger/db/CellStore.h"


namespace SWC { namespace Ranger { namespace CellStore {

class Readers final {
  public:
  typedef Readers*  Ptr;

  DB::RangeBase::Ptr range;

  explicit Readers() { }

  void init(DB::RangeBase::Ptr for_range) {
    range = for_range;
  }

  ~Readers() { }


  void add(Read::Ptr cs) {
    std::scoped_lock lock(m_mutex);
    m_cellstores.push_back(cs);
  }

  void load(int& err) {
    std::scoped_lock lock(m_mutex);
    if(m_cellstores.empty()) {
      err = Error::SERIALIZATION_INPUT_OVERRUN;
      return;
    }
  }

  void expand(DB::Cells::Interval& intval) {
    std::shared_lock lock(m_mutex);
    for(auto cs : m_cellstores)
      intval.expand(cs->interval);
  }

  void expand_and_align(DB::Cells::Interval& intval) {
    std::shared_lock lock(m_mutex);
    for(auto cs : m_cellstores) {
      intval.expand(cs->interval);
      intval.align(cs->interval);
    }
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
    for(auto cs : m_cellstores)
      sz += cs->blocks_count();
    return sz;
  }

  const size_t release(size_t bytes) {    
    size_t released = 0;
    std::shared_lock lock(m_mutex);
    for(auto cs : m_cellstores) {
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
  
  void remove(int &err) {
    std::scoped_lock lock(m_mutex);
    _close();
    for(auto cs : m_cellstores)
      cs->remove(err);
    _free();
    range = nullptr;
  }

  void unload() {
    std::scoped_lock lock(m_mutex);
    _close();
    _free();
    range = nullptr;
  }

  void clear() {
    std::scoped_lock lock(m_mutex);
    _close();
    _free();
  }
  
  void load_cells(BlockLoader* loader) {
    std::shared_lock lock(m_mutex);
    for(auto cs : m_cellstores) {
      if(loader->block->is_consist(cs->interval)) {
        cs->load_cells(loader);
      } else if(!cs->interval.key_end.empty() && 
                !loader->block->is_in_end(cs->interval.key_end)) {
        break;
      }
    }
  }
  
  void get_blocks(int& err, std::vector<Block::Read::Ptr>& to) {
    std::shared_lock lock(m_mutex);
    for(auto cs : m_cellstores) {
      cs->get_blocks(err, to);
      if(err)
        break;
    }
  }

  void get_prev_key_end(uint32_t idx, DB::Cell::Key& key) {
    std::shared_lock lock(m_mutex);
    key.copy((*(m_cellstores.begin()+idx))->prev_key_end);
  }

  const bool need_compaction(size_t cs_sz, size_t blk_size) {
    std::shared_lock lock(m_mutex);
    size_t  sz;
    for(auto cs : m_cellstores) {
      sz = cs->size_bytes(true);
      if(!sz)
        continue;
      if(sz > cs_sz || sz/cs->blocks_count() > blk_size)
        return true;
    }
    return false;
  }

  const size_t encoded_length() {
    std::shared_lock lock(m_mutex);
    size_t sz = Serialization::encoded_length_vi32(m_cellstores.size());
    for(auto cs : m_cellstores) {
      sz += Serialization::encoded_length_vi32(cs->id)
          + cs->interval.encoded_length();
    }
    return sz;
  }

  void encode(uint8_t** ptr) {
    std::shared_lock lock(m_mutex);
    Serialization::encode_vi32(ptr, m_cellstores.size());
    for(auto cs : m_cellstores) {
      Serialization::encode_vi32(ptr, cs->id);
      cs->interval.encode(ptr);
    }
  }
  
  void decode(int &err, const uint8_t** ptr, size_t* remain) {
    std::scoped_lock lock(m_mutex);

    _close();
    _free();
    uint32_t id;
    uint32_t len = Serialization::decode_vi32(ptr, remain);
    for(size_t i=0;i<len;++i) {
      id = Serialization::decode_vi32(ptr, remain);
      m_cellstores.push_back(
        Read::make(err, id, range, DB::Cells::Interval(ptr, remain)));
    }
  }
  
  void load_from_path(int &err) {
    std::scoped_lock lock(m_mutex);

    FS::DirentList dirs;
    Env::FsInterface::interface()->readdir(
      err, range->get_path(DB::RangeBase::cellstores_dir), dirs);
    
    FS::IdEntries_t entries;
    for(auto id : dirs) {
      if(id.name.find(".cs", id.name.length()-3) != std::string::npos) {
        auto idn = id.name.substr(0, id.name.length()-3);
        entries.push_back( (int64_t)strtoll(idn.c_str(), NULL, 0) );
      }
    }
    
    _close();
    _free();

    std::sort(entries.begin(), entries.end());
    for(auto id : entries) {
      m_cellstores.push_back(
        Read::make(err, id, range, DB::Cells::Interval())
      );
    }
  }

  void replace(int &err, CellStore::Writers& w_cellstores) {
    auto fs = Env::FsInterface::interface();

    std::scoped_lock lock(m_mutex);

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
        CellStore::Read::make(err, cs->id, range, cs->interval)
      );
    }
    err = Error::OK;

  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);

    std::string s("CellStores(count=");
    s.append(std::to_string(m_cellstores.size()));

    s.append(" cellstores=[");
    for(auto cs : m_cellstores) {
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
    for(auto cs : m_cellstores) {
      delete cs;
    }
    m_cellstores.clear();
  }

  void _close() {
    int err = Error::OK;
    for(auto cs : m_cellstores)
      cs->close(err);
  }

  const bool _processing() {
    for(auto cs : m_cellstores)
      if(cs->processing())
        return true;
    return false;
  }
  
  const size_t _size_bytes(bool only_loaded=false) {
    size_t  sz = 0;
    for(auto cs : m_cellstores)
      sz += cs->size_bytes(only_loaded);
    return sz;
  }

  std::shared_mutex       m_mutex;
  std::vector<Read::Ptr>  m_cellstores;
};

}}}

#endif // swc_ranger_db_CellStoreReaders_h