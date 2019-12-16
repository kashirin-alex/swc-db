/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_ranger_RangerEnv_h
#define swcdb_ranger_RangerEnv_h

#include "swcdb/fs/Interface.h"
#include "swcdb/db/Files/RgrData.h"

namespace SWC { 

namespace server { namespace Rgr {
class Compaction;
class Columns;
}}

class RangerEnv final {  
  public:

  static void init() {
    HT_ASSERT(m_env == nullptr);
    m_env = std::make_shared<RangerEnv>();
  }

  static void start();

  static void shuttingdown();

  static Files::RgrData* rgr_data() {
    return &m_env->m_rgr_data;
  }

  static bool is_shuttingdown(){
    return m_env->m_shuttingdown;
  }

  static int64_t in_process(){
    return m_env->m_in_process;
  }

  static void in_process(int64_t count){
    m_env->m_in_process += count;
  }

  static RangerEnv* get() {
    return m_env.get();
  }

  static IoContext* maintenance_io() {
    return m_env->mnt_io.get();
  }

  static server::Rgr::Compaction* compaction() {
    return m_env->_compaction;
  }

  static server::Rgr::Columns* columns() {
    return m_env->_columns;
  }

  const gInt32tPtr      cfg_cs_max;
  const gInt32tPtr      cfg_cs_sz;
  const gInt32tPtr      cfg_compact_percent;

  const gInt32tPtr      cfg_blk_size;
  const gInt32tPtr      cfg_blk_cells;
  const gEnumExtPtr     cfg_blk_enc;
  
  IoContext::Ptr            mnt_io;
  server::Rgr::Compaction*  _compaction;
  server::Rgr::Columns*     _columns;

  explicit RangerEnv();

  ~RangerEnv();

  private:
  inline static std::shared_ptr<RangerEnv>  m_env           = nullptr;
  Files::RgrData                            m_rgr_data;
  std::atomic<bool>                         m_shuttingdown  = false;
  std::atomic<int64_t>                      m_in_process    = 0;

  
};

}


#include "swcdb/db/Columns/Rgr/Columns.h"
#include "swcdb/db/Columns/Rgr/Compaction.h"



namespace SWC {

RangerEnv::RangerEnv() 
    : cfg_cs_max(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.rgr.Range.CellStore.count.max")), 
      cfg_cs_sz(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.rgr.Range.CellStore.size.max")), 
      cfg_compact_percent(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.rgr.Range.compaction.size.percent")),
        cfg_blk_size(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.rgr.Range.block.size")), 
      cfg_blk_cells(Env::Config::settings()->get_ptr<gInt32t>(
        "swc.rgr.Range.block.cells")),
      cfg_blk_enc(Env::Config::settings()->get_ptr<gEnumExt>(
        "swc.rgr.Range.block.encoding")), 
      mnt_io(IoContext::make("Maintenance", 
        Env::Config::settings()->get<int32_t>(
          "swc.rgr.maintenance.handlers"))),
      _compaction(nullptr),
      _columns(new server::Rgr::Columns()) {          
}

RangerEnv::~RangerEnv() {
  if(_compaction) 
    delete _compaction;
  delete _columns;
}

void RangerEnv::start() {
  HT_ASSERT(m_env != nullptr);

  m_env->_compaction = new server::Rgr::Compaction();
  m_env->_compaction->schedule();
}

void RangerEnv::shuttingdown() {
  m_env->m_shuttingdown = true;
  
  m_env->_compaction->stop();
  m_env->mnt_io->stop();
  
  m_env->_columns->unload_all(false);
}

}

#endif