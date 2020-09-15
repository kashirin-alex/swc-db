/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_Compaction_h
#define swcdb_ranger_db_Compaction_h


namespace SWC { namespace Ranger {
class Compaction;
}}

#include "swcdb/ranger/db/CompactRange.h"



namespace SWC { namespace Ranger {

class Compaction final {
  public:

  const Property::V_GUINT8::Ptr   cfg_read_ahead;
  const Property::V_GUINT8::Ptr   cfg_max_range;
  const Property::V_GINT32::Ptr   cfg_check_interval;

  explicit Compaction();

  Compaction(const Compaction&) = delete;

  Compaction(const Compaction&&) = delete;
  
  Compaction& operator=(const Compaction&) = delete;

  ~Compaction();

  bool available();

  void stop();

  void schedule();
  
  void schedule(uint32_t t_ms);

  bool stopped();
  
  void run(bool continuing=false);

  void compact(const RangePtr& range);

  void compacted(const CompactRange::Ptr req,
                 const RangePtr& range, bool all=false);

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
  std::vector<CompactRange::Ptr>  m_compacting;

};





}}


//#ifdef SWC_IMPL_SOURCE
#include "swcdb/ranger/db/Compaction.cc"
#include "swcdb/ranger/db/CompactRange.cc"
//#endif 

#endif // swcdb_ranger_db_Compaction_h