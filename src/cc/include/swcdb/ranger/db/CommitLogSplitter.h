/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLogSplitter_h
#define swc_ranger_db_CommitLogSplitter_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Splitter final {
  public:
  
  static constexpr const uint8_t MAX_LOAD = 3;

  Splitter(const DB::Cell::Key& key, std::vector<Fragment::Ptr>& fragments,
           Fragments::Ptr log_left, Fragments::Ptr log_right) 
          : m_fragments(fragments), key(key), 
            log_left(log_left), log_right(log_right) {
  }

  ~Splitter() { }

  void run () {
    Fragment::Ptr frag;
    for(auto it = m_fragments.begin(); it< m_fragments.end();) {
      frag = *it; 
      if(frag->interval.key_comp->compare(key, frag->interval.key_end)
                                  != Condition::GT) {    
        m_fragments.erase(it);
        continue;
      }
      if(frag->interval.key_comp->compare(key, frag->interval.key_begin) 
                                  == Condition::GT) {
        int err = Error::OK;
        log_right->take_ownership(err, frag);
        if(!err) {   
          log_left->remove(err, frag, false);
          m_fragments.erase(it);
          continue;
        }
      }
      if(m_queue.size() == MAX_LOAD) {
        std::unique_lock lock_wait(m_mutex);
        m_cv.wait(lock_wait, [this]() { return m_queue.size() < MAX_LOAD; });
      }
      m_queue.push(frag);
      frag->load([this]() { loaded(); });
      ++it;
    }
    
    std::unique_lock lock_wait(m_mutex);
    if(!m_queue.empty())
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

      if(!err && !loaded) {
        m_queue.deactivate();
        return;
      }
      if(err)
        SWC_LOGF(LOG_ERROR, 
          "COMPACT fragment-split err=%d(%s) %s", 
          err, Error::get_text(err), frag->to_string().c_str()
        );
      m_cv.notify_one();

    } while(!m_queue.deactivating());
    
    m_cv.notify_one();
  }

  std::mutex                        m_mutex;
  std::condition_variable           m_cv;
  std::vector<Fragment::Ptr>&       m_fragments;
  QueueSafeStated<Fragment::Ptr>    m_queue;

  const DB::Cell::Key key;
  Fragments::Ptr log_left;
  Fragments::Ptr log_right;
};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogSplitter_h