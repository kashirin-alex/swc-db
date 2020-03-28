/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_CommitLogSplitter_h
#define swc_ranger_db_CommitLogSplitter_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Splitter : public std::vector<Fragment::Ptr> {
  public:
  using std::vector<Fragment::Ptr>::vector;


  Splitter(const DB::Cell::Key& key, 
           const std::vector<Fragment::Ptr>& fragments, 
           Fragment::AddCell_t& left, Fragment::AddCell_t& right) 
          : std::vector<Fragment::Ptr>(fragments),
            m_processing(false), m_completion(0), m_it(begin()),
            key(key), left(left), right(right) { 
  }

  ~Splitter() { }

  void run () {
    for(auto frag : *this) {
      {
        std::unique_lock lock_wait(m_mutex);
        if(m_completion == 3) 
          m_cv.wait(lock_wait, [this]() { return m_completion < 3; });
        ++m_completion;
      }
      frag->load([this]() { loaded(); });
    }

    std::unique_lock lock_wait(m_mutex);
    if(m_completion)
      m_cv.wait(lock_wait, [this]() { return m_completion == 0; });
  }

  private:

  void loaded() {
    {
      std::scoped_lock lock(m_mutex);
      if(m_processing) 
        return;
      m_processing = true;
    }
    asio::post(*Env::IoCtx::io()->ptr(), [this](){ split(); });
  }

  void split() {
    bool loaded;
    int err;
    Fragment::Ptr frag;
    while(m_it != end()) {

      err = Error::OK;
      if(loaded = (frag = *m_it)->loaded(err))
        frag->split(err, key, left, right); 

      if(!err && !loaded) 
        break;

      if(err)
        SWC_LOGF(LOG_ERROR, 
          "COMPACT fragment-split err=%d(%s) %s", 
          err, Error::get_text(err), frag->to_string().c_str()
        );
      
      ++m_it;
      {
        std::scoped_lock lock_wait(m_mutex);
        --m_completion;
      }
      m_cv.notify_one();
    }

    std::scoped_lock lock(m_mutex);
    m_processing = false;
  }

  std::mutex                        m_mutex;
  std::condition_variable           m_cv;
  bool                              m_processing;
  uint32_t                          m_completion;
  iterator                          m_it;

  const DB::Cell::Key key;
  Fragment::AddCell_t left;
  Fragment::AddCell_t right;
};


}}} // namespace SWC::Ranger::CommitLog

#endif // swc_ranger_db_CommitLogSplitter_h