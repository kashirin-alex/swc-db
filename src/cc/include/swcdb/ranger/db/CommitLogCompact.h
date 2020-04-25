/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLogCompact_h
#define swc_ranger_db_CommitLogCompact_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Compact final {
  public:

  Compact(RangePtr range, std::atomic<bool>& stop) 
          : range(range), stop(stop), m_remain(0) {
  }

  ~Compact() { }

  void run(const std::vector<Fragment::Ptr>& sorted) {
    range->blocks.commitlog.compacting = true;
    m_remain = sorted.size();
    SWC_LOGF(LOG_INFO, 
      "COMPACT-FRAGMENTS-START %d/%d fragments=%lld",
      range->cfg->cid, range->rid, sorted.size()
    );

    uint64_t ts = Time::now_ns();

    Fragment::Ptr frag;
    for(auto it = sorted.begin(); it< sorted.end(); ++it) {
      if(m_queue.size() == Fragments::MAX_PRELOAD) {
        std::unique_lock lock_wait(m_mutex);
        m_cv.wait(lock_wait, [this]() { 
          return m_queue.size() < Fragments::MAX_PRELOAD; 
        });
      }
      if(stop)
        break;
      m_queue.push(*it);
      (*it)->load([this]() { loaded(); });
    }
    
    std::unique_lock lock_wait(m_mutex);
    if(!m_queue.empty())
      m_cv.wait(lock_wait, [this]() { 
        loaded();
        return m_queue.empty(); 
      });
    range->blocks.commitlog.compacting = false;
    
    ts = Time::now_ns() - ts;
    SWC_LOGF(LOG_INFO, 
      "COMPACT-FRAGMENTS-FINISH %d/%d fragments=%lld avg=%lldns took=%lld",
      range->cfg->cid, range->rid, sorted.size(), ts / sorted.size(), ts
    );
  }

  private:

  void loaded() {
    if(m_queue.activating())
      asio::post(*Env::IoCtx::io()->ptr(), [this](){ split(); });
  }

  void split() {
    bool loaded;
    int err;
    Fragment::Ptr frag;
    uint64_t ts;
    do {
      err = Error::OK;
      if(stop) {
        do {
          (frag = m_queue.front())->processing_decrement();
          frag->release();
        } while(!m_queue.deactivating());
        break;
      }

      if(loaded = (frag = m_queue.front())->loaded(err)) {
        ts = Time::now_ns();
        frag->load_cells(err, &range->blocks.commitlog); 
        --m_remain;
        ts = Time::now_ns() - ts;

        SWC_LOGF(LOG_INFO, 
          "COMPACT-FRAGMENTS-PROGRESS %d/%d remain=%lld "
          "cells=%lld avg=%lldns took=%lld log-cells=%lld begin-%s end-%s",
          range->cfg->cid, range->rid, m_remain,
          frag->cells_count, frag->cells_count? ts/frag->cells_count: 0, 
          ts, range->blocks.commitlog.cells_count(true),
          frag->interval.key_begin.to_string().c_str(),
          frag->interval.key_end.to_string().c_str()
        );

        range->blocks.commitlog.remove(err, frag, true);
      }

      if(!err && !loaded) {
        m_queue.deactivate();
        return;
      }
      if(err)
        SWC_LOGF(LOG_ERROR, 
          "COMPACT-FRAGMENTS fragment err=%d(%s) %s", 
          err, Error::get_text(err), frag->to_string().c_str()
        );
      m_cv.notify_one();

    } while(!m_queue.deactivating());
    
    m_cv.notify_one();
  }

  RangePtr                          range;
  std::atomic<bool>&                stop;
  std::mutex                        m_mutex;
  std::condition_variable           m_cv;
  QueueSafeStated<Fragment::Ptr>    m_queue;
  size_t                            m_remain;

};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogCompact_h