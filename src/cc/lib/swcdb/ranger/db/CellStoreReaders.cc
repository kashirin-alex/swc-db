/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/db/CellStoreReaders.h"


namespace SWC { namespace Ranger { namespace CellStore {

Readers::Readers() { }

void Readers::init(const RangePtr& for_range) {
  range = for_range;
}

Readers::~Readers() { }


SWC_SHOULD_INLINE
void Readers::add(Read::Ptr cs) {
  push_back(cs);
}

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

SWC_SHOULD_INLINE
uint32_t Readers::get_cell_revs() const {
  return front()->cell_revs;
}

int64_t Readers::get_ts_earliest() const {
  int64_t ts = DB::Cells::AUTO_ASSIGN;
  for(auto cs : *this)
    if(cs->interval.ts_earliest.comp != Condition::NONE &&
       (ts == DB::Cells::AUTO_ASSIGN || cs->interval.ts_earliest.value < ts))
      ts = cs->interval.ts_earliest.value;
  return ts;
}

size_t Readers::blocks_count() const {
  size_t  sz = 0;
  for(auto cs : *this)
    sz += cs->blocks_count();
  return sz;
}

size_t Readers::release(size_t bytes) {
  size_t released = 0;
  for(auto cs : *this) {
    released += cs->release(bytes ? bytes-released : bytes);
    if(bytes && released >= bytes)
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

void Readers::get_blocks(int& err, std::vector<Block::Read::Ptr>& to) const {
  for(auto cs : *this) {
    cs->get_blocks(err, to);
    if(err)
      break;
  }
}

SWC_SHOULD_INLINE
void Readers::get_prev_key_end(uint32_t idx, DB::Cell::Key& key) const {
  key.copy((*(begin()+idx))->prev_key_end);
}

void Readers::get_key_end(DB::Cell::Key& key) const {
  return key.copy(back()->key_end);
}

bool Readers::need_compaction(size_t cs_sz, size_t blk_size) const {
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

size_t Readers::encoded_length() const {
  size_t sz = Serialization::encoded_length_vi32(size());
  for(auto cs : *this) {
    sz += Serialization::encoded_length_vi32(cs->csid)
        + cs->interval.encoded_length();
  }
  return sz;
}

void Readers::encode(uint8_t** ptr) const {
  Serialization::encode_vi32(ptr, size());
  for(auto cs : *this) {
    Serialization::encode_vi32(ptr, cs->csid);
    cs->interval.encode(ptr);
  }
}

void Readers::decode(int &err, const uint8_t** ptr, size_t* remain) {
  _close();
  _free();
  csid_t csid;
  uint32_t len = Serialization::decode_vi32(ptr, remain);
  for(size_t i=0; i<len; ++i) {
    csid = Serialization::decode_vi32(ptr, remain);
    push_back(
      Read::make(
        err, csid, range,
        DB::Cells::Interval(range->cfg->key_seq, ptr, remain)));
    //if(err == Error::FS_PATH_NOT_FOUND) ?without cs
  }
}

void Readers::load_from_path(int &err) {

  FS::DirentList dirs;
  Env::FsInterface::interface()->readdir(
    err, range->get_path(Range::CELLSTORES_DIR), dirs);

  FS::IdEntries_t entries;
  entries.reserve(dirs.size());
  for(auto& entry : dirs) {
    if(entry.name.find(".cs", entry.name.length()-3) != std::string::npos) {
      auto idn = entry.name.substr(0, entry.name.length()-3);
      entries.push_back(strtoll(idn.c_str(), nullptr, 0));
    }
  }

  _close();
  _free();

  std::sort(entries.begin(), entries.end());
  for(csid_t csid : entries) {
    push_back(
      Read::make(err, csid, range, DB::Cells::Interval(range->cfg->key_seq))
    );
    // add sorted by cs interval-key_end, for range & cellstores after merge
  }
}

void Readers::replace(int &err, Writers& w_cellstores) {
  const auto& fs = Env::FsInterface::interface();

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
    Vec cellstores;
    for(auto cs : w_cellstores) {
      cellstores.push_back(
        Read::make(err, cs->csid, range, cs->interval, true)
      );
      if(err)
        break;
    }
    if(err) {
      for(auto cs : cellstores)
        delete cs;
    } else {
      _free();
      assign(cellstores.begin(), cellstores.end());
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

void Readers::move_from(int &err, Readers::Vec& mv_css) {
  const auto& fs = Env::FsInterface::interface();

  Vec moved;
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
  if(!err) {
    for(auto cs : moved) {
      cellstores.push_back(
        Read::make(err, cs->csid, range, cs->interval, true)
      );
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
    assign(cellstores.begin(), cellstores.end());
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
}

void Readers::_close() {
  int err = Error::OK;
  for(auto cs : *this)
    cs->close(err);
}


}}}
