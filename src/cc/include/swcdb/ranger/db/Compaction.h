/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_Compaction_h
#define swcdb_ranger_db_Compaction_h



namespace SWC { namespace Ranger {

class Compaction final {
  public:

  typedef Compaction* Ptr;

  const Property::V_GUINT8::Ptr   cfg_read_ahead;
  const Property::V_GUINT8::Ptr   cfg_max_range;

  explicit Compaction();

  Compaction(const Compaction&) = delete;

  Compaction(const Compaction&&) = delete;
  
  Compaction& operator=(const Compaction&) = delete;

  virtual ~Compaction();
 
  Ptr ptr();

  bool available();

  void stop();

  void schedule();
  
  void schedule(uint32_t t_ms);

  bool stopped();
  
  void run(bool continuing=false);

  void compact(RangePtr range);

  void compacted(RangePtr range, bool all=false);

  private:
  
  void compacted();

  void _schedule(uint32_t t_ms = 300000);


  std::mutex                      m_mutex;
  asio::high_resolution_timer     m_check_timer;
  bool                            m_run;
  uint32_t                        m_running;
  bool                            m_scheduled;
  std::condition_variable         m_cv;
  
  size_t                          m_idx_cid;
  size_t                          m_idx_rid;

  const Property::V_GINT32::Ptr   cfg_check_interval;
};





}}


//#ifdef SWC_IMPL_SOURCE
#include "swcdb/ranger/db/Compaction.cc"
#include "swcdb/ranger/db/CompactRange.cc"
//#endif 

#endif // swcdb_ranger_db_Compaction_h