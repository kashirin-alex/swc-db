/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/db/CellStoreReaders.h"


namespace SWC { namespace Ranger { namespace CellStore {

Readers::Readers() { }

void Readers::init(RangePtr for_range) {
  range = for_range;
}

Readers::~Readers() { }


void Readers::add(Read::Ptr cs) {
  //std::scoped_lock lock(m_mutex);
  m_cellstores.push_back(cs);
}

void Readers::load(int& err) {
  //std::scoped_lock lock(m_mutex);
  if(m_cellstores.empty()) {
    err = Error::SERIALIZATION_INPUT_OVERRUN;
    return;
  }
}

void Readers::expand(DB::Cells::Interval& intval) {
  //std::shared_lock lock(m_mutex);
  for(auto cs : m_cellstores)
    intval.expand(cs->interval);
}

void Readers::expand_and_align(DB::Cells::Interval& intval) {
  //std::shared_lock lock(m_mutex);
  for(auto cs : m_cellstores) {
    intval.expand(cs->interval);
    intval.align(cs->interval);
  }
}

bool Readers::empty() {
  //std::shared_lock lock(m_mutex);
  return m_cellstores.empty();
}

size_t Readers::size() {
  //std::shared_lock lock(m_mutex);
  return m_cellstores.size();
}

size_t Readers::size_bytes(bool only_loaded) {
  //std::shared_lock lock(m_mutex);
  return _size_bytes(only_loaded);
}

size_t Readers::blocks_count() {
  size_t  sz = 0;
  //std::shared_lock lock(m_mutex);
  for(auto cs : m_cellstores)
    sz += cs->blocks_count();
  return sz;
}

size_t Readers::release(size_t bytes) {    
  size_t released = 0;
  //std::shared_lock lock(m_mutex);
  for(auto cs : m_cellstores) {
    released += cs->release(bytes ? bytes-released : bytes);
    if(bytes && released >= bytes)
      break;
  }
  return released;
}

bool Readers::processing() {
  //std::shared_lock lock(m_mutex);
  return _processing();
}

void Readers::remove(int &err) {
  //std::scoped_lock lock(m_mutex);
  _close();
  for(auto cs : m_cellstores)
    cs->remove(err);
  _free();
  range = nullptr;
}

void Readers::unload() {
  //std::scoped_lock lock(m_mutex);
  _close();
  _free();
  range = nullptr;
}

void Readers::clear() {
  //std::scoped_lock lock(m_mutex);
  _close();
  _free();
}

void Readers::load_cells(BlockLoader* loader) {
  //std::shared_lock lock(m_mutex);
  for(auto cs : m_cellstores) {
    if(loader->block->is_consist(cs->interval)) {
      cs->load_cells(loader);
    } else if(!cs->interval.key_end.empty() && 
              !loader->block->is_in_end(cs->interval.key_end)) {
      break;
    }
  }
}

void Readers::get_blocks(int& err, std::vector<Block::Read::Ptr>& to) {
  //std::shared_lock lock(m_mutex);
  for(auto cs : m_cellstores) {
    cs->get_blocks(err, to);
    if(err)
      break;
  }
}

void Readers::get_prev_key_end(uint32_t idx, DB::Cell::Key& key) {
  //std::shared_lock lock(m_mutex);
  key.copy((*(m_cellstores.begin()+idx))->prev_key_end);
}

bool Readers::need_compaction(size_t cs_sz, size_t blk_size) {
  //std::shared_lock lock(m_mutex);
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

size_t Readers::encoded_length() {
  //std::shared_lock lock(m_mutex);
  size_t sz = Serialization::encoded_length_vi32(m_cellstores.size());
  for(auto cs : m_cellstores) {
    sz += Serialization::encoded_length_vi32(cs->id)
        + cs->interval.encoded_length();
  }
  return sz;
}

void Readers::encode(uint8_t** ptr) {
  //std::shared_lock lock(m_mutex);
  Serialization::encode_vi32(ptr, m_cellstores.size());
  for(auto cs : m_cellstores) {
    Serialization::encode_vi32(ptr, cs->id);
    cs->interval.encode(ptr);
  }
}

void Readers::decode(int &err, const uint8_t** ptr, size_t* remain) {
  //std::scoped_lock lock(m_mutex);

  _close();
  _free();
  uint32_t id;
  uint32_t len = Serialization::decode_vi32(ptr, remain);
  for(size_t i=0;i<len;++i) {
    id = Serialization::decode_vi32(ptr, remain);
    m_cellstores.push_back(
      Read::make(err, id, range, DB::Cells::Interval(ptr, remain)));
    //if(err == Error::FS_PATH_NOT_FOUND) ?without cs
  }
}

void Readers::load_from_path(int &err) {
  //std::scoped_lock lock(m_mutex);

  FS::DirentList dirs;
  Env::FsInterface::interface()->readdir(
    err, range->get_path(Range::CELLSTORES_DIR), dirs);
  
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

void Readers::replace(int &err, CellStore::Writers& w_cellstores) {
  auto fs = Env::FsInterface::interface();

  //std::scoped_lock lock(m_mutex);

  _close();

  fs->rename(
    err, 
    range->get_path(Range::CELLSTORES_DIR), 
    range->get_path(Range::CELLSTORES_BAK_DIR)
  );
  if(err) 
    return;

  fs->rename(
    err, 
    range->get_path(Range::CELLSTORES_TMP_DIR), 
    range->get_path(Range::CELLSTORES_DIR)
  );

  if(!err) {
    std::vector<Read::Ptr> cellstores;
    for(auto cs : w_cellstores) {
      cellstores.push_back(
        CellStore::Read::make(err, cs->id, range, cs->interval, true)
      );
      if(err)
        break;
    }
    if(err) {
      for(auto cs : cellstores)
        delete cs;
    } else {
      _free();
      m_cellstores = cellstores;
    }
  }

  if(err) {
    fs->rmdir(err, range->get_path(Range::CELLSTORES_DIR));
    fs->rename(
      err, 
      range->get_path(Range::CELLSTORES_BAK_DIR), 
      range->get_path(Range::CELLSTORES_DIR)
    );
    return;
  }
  
  fs->rmdir(err, range->get_path(Range::CELLSTORES_BAK_DIR));

  err = Error::OK;

}

std::string Readers::to_string() {
  //std::shared_lock lock(m_mutex);

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


void Readers::_free() {
  for(auto cs : m_cellstores) {
    delete cs;
  }
  m_cellstores.clear();
}

void Readers::_close() {
  int err = Error::OK;
  for(auto cs : m_cellstores)
    cs->close(err);
}

bool Readers::_processing() {
  for(auto cs : m_cellstores)
    if(cs->processing())
      return true;
  return false;
}

size_t Readers::_size_bytes(bool only_loaded) {
  size_t  sz = 0;
  for(auto cs : m_cellstores)
    sz += cs->size_bytes(only_loaded);
  return sz;
}

}}}
