/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Rgr_Compaction_h
#define swcdb_lib_db_Columns_Rgr_Compaction_h

#include "Columns.h"


namespace SWC { namespace server { namespace Rgr {

class Compaction : public std::enable_shared_from_this<Compaction> {
  public:

  typedef std::shared_ptr<Compaction> Ptr;

  Compaction(uint32_t workers=Env::Config::settings()->get<int32_t>(
                                              "swc.rgr.maintenance.handlers"))
            : m_io(std::make_shared<IoContext>("RangerMaintenance", workers)), 
              m_check_timer(asio::high_resolution_timer(*m_io->ptr())),
              m_run(true), m_running(0), m_scheduled(false),
              m_idx_cid(0), m_idx_rid(0), 
              cfg_check_interval(Env::Config::settings()->get_ptr<gInt32t>(
                "swc.rgr.compaction.check.interval")) {
    m_io->run(m_io);
  }

  virtual ~Compaction(){}
 
  void stop() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_run = false;
    m_check_timer.cancel();
    m_io->stop();
  }

  void schedule() {
    schedule(cfg_check_interval->get());
  }
  
  void schedule(uint32_t t_ms) {
    std::lock_guard<std::mutex> lock(m_mutex);
    _schedule(t_ms);
  }
  
  void run() {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_scheduled = true;
    }

    ColumnPtr col     = nullptr;
    Range::Ptr range  = nullptr;

    for(;;) {
      col = Env::RgrColumns::get()->get_next(m_idx_cid);
      if(col == nullptr) {
        m_idx_cid = 0;
        break;
      }

      if(col->removing()){
        m_idx_cid++;
        continue;
      }

      range = col->get_next(m_idx_rid);    
      if(range == nullptr) {
        m_idx_cid++;
        continue;
      }

      if(!range->is_loaded())
        continue;
      
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_running;
          
        asio::post(
          *m_io->ptr(), 
          [range, ptr=shared_from_this()](){ 
            ptr->compact(range); 
            ptr->compacted();
          }
        );

        if(m_running == m_io->get_size())
          return;
      }

    }

  }

  void compact(Range::Ptr range) {
    if(!range->is_loaded())
      return;
      
  }


  private:
  
  void compacted() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_running-- == m_io->get_size()) {
      run();
    } else if(!m_running) {
      m_scheduled = false;
      _schedule(cfg_check_interval->get());
    }
  }

  void _schedule(uint32_t t_ms = 300000) {
    if(!m_run || m_running || m_scheduled)
      return;

    auto set_in = std::chrono::milliseconds(t_ms);
    auto set_on = m_check_timer.expires_from_now();
    if(set_on > std::chrono::milliseconds(0) && set_on < set_in)
      return;

    m_check_timer.cancel();
    m_check_timer.expires_from_now(set_in);

    m_check_timer.async_wait(
      [ptr=shared_from_this()](const asio::error_code ec) {
        if (ec != asio::error::operation_aborted){
          ptr->run();
        }
    }); 

    //if(t_ms > 10000) {
    //  std::cout << to_string() << "\n";
    //}
    HT_DEBUGF("Ranger compaction scheduled in ms=%d", t_ms);
  }

  std::mutex                   m_mutex;
  IoContext::Ptr               m_io;
  asio::high_resolution_timer  m_check_timer;
  bool                         m_run;
  uint32_t                     m_running;
  bool                         m_scheduled;
  
  size_t                       m_idx_cid;
  size_t                       m_idx_rid;

  const gInt32tPtr            cfg_check_interval;
};



}}}
#endif