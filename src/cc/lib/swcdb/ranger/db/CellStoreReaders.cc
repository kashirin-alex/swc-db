/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStoreReaders.h"


namespace SWC { namespace Ranger { namespace CellStore {


SWC_CAN_INLINE
void Readers::load(int& err) {
  if(empty()) {
    err = Error::SERIALIZATION_INPUT_OVERRUN;
    return;
  }
}

void Readers::expand(DB::Cells::Interval& intval) const {
  for(auto cs : *this)
    intval.expand(cs->interval);
}

void Readers::expand_and_align(DB::Cells::Interval& intval) const {
  for(auto cs : *this) {
    intval.expand(cs->interval);
    intval.align(cs->interval);
  }
}

size_t Readers::size_bytes(bool only_loaded) const {
  size_t  sz = 0;
  for(auto cs : *this)
    sz += cs->size_bytes(only_loaded);
  return sz;
}

SWC_CAN_INLINE
uint32_t Readers::get_cell_revs() const {
  return front()->cell_revs;
}

int64_t Readers::get_ts_earliest() const {
  int64_t ts = DB::Cells::TIMESTAMP_AUTO;
  for(auto cs : *this)
    if(cs->interval.ts_earliest.comp != Condition::NONE &&
       (ts == DB::Cells::TIMESTAMP_AUTO || cs->interval.ts_earliest.value < ts))
      ts = cs->interval.ts_earliest.value;
  return ts;
}

SWC_CAN_INLINE
size_t Readers::blocks_count() const {
  size_t  sz = 0;
  for(auto cs : *this)
    sz += cs->blocks_count();
  return sz;
}

SWC_CAN_INLINE
size_t Readers::release(size_t bytes) {
  size_t released = 0;
  for(auto cs : *this) {
    released += cs->release(bytes - released);
    if(released >= bytes)
      break;
  }
  return released;
}

bool Readers::processing() const noexcept {
  for(auto cs : *this)
    if(cs->processing())
      return true;
  return false;
}

void Readers::remove(int &err) {
  _close();
  for(auto cs : *this)
    cs->remove(err);
  _free();
  range = nullptr;
}

void Readers::unload() {
  _close();
  _free();
  range = nullptr;
}

void Readers::clear() {
  _close();
  _free();
}

SWC_CAN_INLINE
void Readers::load_cells(BlockLoader* loader) {
  for(auto cs : *this) {
    if(loader->block->is_consist(cs->interval)) {
      cs->load_cells(loader);
    } else if(!cs->interval.key_end.empty() &&
              !loader->block->is_in_end(cs->interval.key_end)) {
      break;
    }
  }
}

SWC_CAN_INLINE
void Readers::get_blocks(int& err, Read::Blocks& to) const {
  to.reserve(blocks_count());
  for(auto cs : *this) {
    cs->get_blocks(err, to);
    if(err)
      break;
  }
}

SWC_CAN_INLINE
void Readers::get_prev_key_end(uint32_t idx, DB::Cell::Key& key) const {
  key.copy((*(cbegin()+idx))->prev_key_end);
}

SWC_CAN_INLINE
void Readers::get_key_end(DB::Cell::Key& key) const {
  return key.copy(back()->key_end);
}

bool Readers::need_compaction(size_t cs_max, size_t cs_sz,
                              size_t blk_size) const {
  if(cs_max > 1 && size() > cs_max) {
    if(get_cell_revs() == 1)
      return true;
    for(auto it = cbegin() + 1; it < cend(); ++it) {
      if(!(*it)->prev_key_end.equal((*it)->interval.key_begin))
        return true;
    }
  }
  size_t  sz;
  size_t  sz_enc;
  for(auto cs : *this) {
    if(!(sz_enc = cs->size_bytes_enc(false)) || !(sz = cs->size_bytes(false)))
      continue;
    if(sz_enc > cs_sz || sz/cs->blocks_count() > blk_size)
      return true;
  }
  return false;
}

uint32_t Readers::encoded_length() const {
  uint32_t sz = Serialization::encoded_length_vi32(size());
  for(auto cs : *this) {
    sz += Serialization::encoded_length_vi32(cs->csid);
  }
  return sz;
}

void Readers::encode(uint8_t** ptr) const {
  Serialization::encode_vi32(ptr, size());
  for(auto cs : *this) {
    Serialization::encode_vi32(ptr, cs->csid);
  }
}

void Readers::decode(int &err, const uint8_t** ptr, size_t* remain) {
  _close();
  _free();
  uint32_t len = Serialization::decode_vi32(ptr, remain);
  Vec::reserve(len);
  for(size_t i=0; i<len; ++i) {
    push_back(
      Read::make(err, Serialization::decode_vi32(ptr, remain), range));
    //if(err == Error::FS_PATH_NOT_FOUND) ?without cs
  }
}

SWC_CAN_INLINE
void Readers::load_from_path(int &err) {
  const auto& fs_if = Env::FsInterface::interface();

  const std::string& cs_path = range->get_path(DB::RangeBase::CELLSTORES_DIR);
  const std::string& bak_path = range->get_path(Range::CELLSTORES_BAK_DIR);

  if(fs_if->exists(err, bak_path)) {
    const std::string& rcvd_path = range->get_path(Range::CELLSTORES_RCVD_DIR);
    SWC_LOGF(LOG_WARN,
      "Backup dir='%s' has remained - Recovering from uncomplete Compaction, "
      "backing-up current to='%s'", bak_path.c_str(), rcvd_path.c_str());
    if(fs_if->exists(err, cs_path))
      fs_if->rename(err, cs_path, rcvd_path);
    if(err)
      return;
    fs_if->rename(err, bak_path, cs_path);
  }
  if(err)
    return;

  FS::DirentList dirs;
  fs_if->readdir(err, cs_path, dirs);
  if(err)
    return;

  FS::IdEntries_t entries;
  entries.reserve(dirs.size());
  for(auto& entry : dirs) {
    if(entry.name.find(".cs", entry.name.length()-3) != std::string::npos) {
      auto idn = entry.name.substr(0, entry.name.length()-3);
      entries.push_back(strtoull(idn.c_str(), nullptr, 0));
    }
  }

  _close();
  _free();

  std::sort(entries.begin(), entries.end());
  Vec::reserve(entries.size());
  for(csid_t csid : entries) {
    push_back(Read::make(err, csid, range));
  }
}

void Readers::replace(int &err, Writers& w_cellstores) {
  const auto& fs_if = Env::FsInterface::interface();

  _close();

  const std::string& cs_path = range->get_path(DB::RangeBase::CELLSTORES_DIR);
  const std::string& bak_path = range->get_path(Range::CELLSTORES_BAK_DIR);

  fs_if->rename(err, cs_path, bak_path);
  if(err)
    return;

  fs_if->rename(err, range->get_path(Range::CELLSTORES_TMP_DIR), cs_path);

  if(!err) {
    Vec cellstores;
    cellstores.reserve(w_cellstores.size());
    for(auto cs : w_cellstores) {
      cellstores.push_back(Read::make(err, cs->csid, range, true));
      if(err)
        break;
    }
    if(err) {
      for(auto cs : cellstores)
        delete cs;
    } else {
      _free();
      Vec::operator=(std::move(cellstores));
    }
  }

  if(err) {
    fs_if->rmdir(err, cs_path);
    fs_if->rename(err, bak_path, cs_path);
    return;
  }

  fs_if->rmdir(err, bak_path);

  err = Error::OK;

}

void Readers::move_from(int &err, Readers::Vec& mv_css) {
  const auto& fs = Env::FsInterface::interface();

  Vec moved;
  moved.reserve(mv_css.size());
  int tmperr;
  for(auto cs : mv_css) {
    cs->close(tmperr = Error::OK);
    fs->rename(
      err,
      cs->filepath(),
      range->get_path_cs(cs->csid)
    );
    if(err)
      break;
    moved.push_back(cs);
  }

  Vec cellstores;
  cellstores.reserve(moved.size());
  if(!err) {
    for(auto cs : moved) {
      cellstores.push_back(Read::make(err, cs->csid, range, true));
      if(err)
        break;
    }
  }

  if(err) {
    for(auto cs : cellstores)
      delete cs;

    for(auto& cs : moved) {
      fs->rename(
        tmperr = Error::OK,
        range->get_path_cs(cs->csid),
        cs->filepath()
      );
    }
    mv_css.clear();
  } else {
    Vec::operator=(std::move(cellstores));
  }
}

void Readers::print(std::ostream& out, bool minimal) const {
  out << "CellStores(count=" << size()
      << " cellstores=[";
  for(auto cs : *this) {
    cs->print(out, minimal);
    out << ", ";
  }
  out << "] processing=" << processing()
      << " used/actual=" << size_bytes(true) << '/' << size_bytes()
      << ')';
}


void Readers::_free() {
  for(auto cs : *this) {
    delete cs;
  }
  Vec::clear();
  Vec::shrink_to_fit();
}

void Readers::_close() {
  int err = Error::OK;
  for(auto cs : *this)
    cs->close(err);
}


}}}
