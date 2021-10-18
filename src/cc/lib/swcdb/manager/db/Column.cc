/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/manager/db/Column.h"


namespace SWC { namespace Manager {



void Column::init(int &err) {
  FS::IdEntries_t entries;
  Env::FsInterface::interface()->get_structured_ids(
    err, DB::RangeBase::get_path(cfg->cid), entries);
  if(err) {
    if(err == ENOENT)
      create(err = Error::OK, cfg->cid);
    if(err)
      return;
  }
  if(entries.empty()) {
    SWC_LOGF(LOG_INFO, "Init. New Column(" SWC_FMT_LU ") Range(1)", cfg->cid);
    entries.push_back(1);
  }

  Core::ScopedLock lock(m_mutex);
  for(auto rid : entries) {
    Env::Mngr::columns()->assign_add(
      emplace_back(new Range(cfg, rid))
    );
  }
}



}}
