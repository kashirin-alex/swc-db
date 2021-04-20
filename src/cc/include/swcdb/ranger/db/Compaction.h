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

  const Config::Property::V_GUINT8::Ptr   cfg_read_ahead;
  const Config::Property::V_GUINT8::Ptr   cfg_max_range;
  const Config::Property::V_GUINT8::Ptr   cfg_max_log;
  const Config::Property::V_GINT32::Ptr   cfg_check_interval;

  explicit Compaction();

  Compaction(const Compaction&) = delete;

  Compaction(const Compaction&&) = delete;

  Compaction& operator=(const Compaction&) = delete;

  //~Compaction() { }

  bool log_compact_possible() noexcept;

  void log_compact_finished() noexcept;

  bool available() noexcept;

  void stop();

  void schedule();

  void schedule(uint32_t t_ms);

  bool stopped();

  void run(bool initial=true);

  void compacted(const CompactRange::Ptr req,
                 const RangePtr& range, bool all=false);

  private:

  uint8_t compact(const RangePtr& range);

  void compacted();

  void _schedule(uint32_t t_ms = 300000);


  Core::AtomicBool                m_run;
  Core::StateRunning              m_schedule;
  Core::Atomic<uint8_t>           m_running;
  Core::StateRunning              m_log_chk;
  Core::Atomic<uint8_t>           m_log_compactions;

  std::mutex                      m_mutex;
  asio::high_resolution_timer     m_check_timer;
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
