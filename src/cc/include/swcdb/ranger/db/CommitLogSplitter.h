/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swc_ranger_db_CommitLogSplitter_h
#define swc_ranger_db_CommitLogSplitter_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Splitter final {
  public:
  

  Splitter(const DB::Cell::Key& key, Fragments::Vec& fragments,
           Fragments::Ptr log_left, Fragments::Ptr log_right) 
          : m_fragments(fragments), key(key), 
            log_left(log_left), log_right(log_right) {
  }

  Splitter(const Splitter&) = delete;

  Splitter(const Splitter&&) = delete;
  
  Splitter& operator=(const Splitter&) = delete;

  ~Splitter() { }

  void run () {
    Fragment::Ptr frag;
    for(auto it = m_fragments.begin(); it< m_fragments.end();) {
      frag = *it; 
      if(DB::KeySeq::compare(frag->interval.key_seq, 
          key, frag->interval.key_end) != Condition::GT) {    
        m_fragments.erase(it);
        continue;
      }
      if(DB::KeySeq::compare(frag->interval.key_seq, 
          key, frag->interval.key_begin) == Condition::GT) {
        int err = Error::OK;
        if(log_right->take_ownership(err, frag)) {
          log_left->remove(err, frag, false);
          m_fragments.erase(it);
          continue;
        }
      }
      if(m_queue.size() == Fragments::MAX_PRELOAD) {
        std::unique_lock lock_wait(m_mutex);
        m_cv.wait(lock_wait, [this]() { 
          return m_queue.size() < Fragments::MAX_PRELOAD; 
        });
      }
      m_queue.push(frag);
      frag->load([this]() { loaded(); });
      ++it;
    }
    
    std::unique_lock lock_wait(m_mutex);
    m_cv.wait(lock_wait, [this]() { 
      loaded();
      return m_queue.empty();
    });
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
    do {
      err = Error::OK;
      if(loaded = (frag = m_queue.front())->loaded(err))
        frag->split(err, key, log_left, log_right); 

      if(!err && !loaded)
        return m_queue.deactivate();
      
      if(err) {
        frag->processing_decrement();
        SWC_LOGF(LOG_ERROR, 
          "COMPACT fragment-split err=%d(%s) %s", 
          err, Error::get_text(err), frag->to_string().c_str()
        );
      }
      
      std::unique_lock lock_wait(m_mutex);
      m_cv.notify_one();

    } while(!m_queue.deactivating());
    
    std::unique_lock lock_wait(m_mutex);
    m_cv.notify_one();
  }

  std::mutex                        m_mutex;
  std::condition_variable           m_cv;
  Fragments::Vec&                   m_fragments;
  QueueSafeStated<Fragment::Ptr>    m_queue;

  const DB::Cell::Key key;
  Fragments::Ptr log_left;
  Fragments::Ptr log_right;
};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogSplitter_h