/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CommitLogSplitter_h
#define swcdb_ranger_db_CommitLogSplitter_h


namespace SWC { namespace Ranger { namespace CommitLog {

  
class Splitter final {
  public:
  

  Splitter(const DB::Cell::Key& key, Fragments::Vec& fragments,
           Fragments::Ptr log_left, Fragments::Ptr log_right) 
          : m_sem(Fragments::MAX_PRELOAD), m_fragments(fragments), 
            key(key), log_left(log_left), log_right(log_right) {
  }

  Splitter(const Splitter&) = delete;

  Splitter(const Splitter&&) = delete;
  
  Splitter& operator=(const Splitter&) = delete;

  ~Splitter() { }

  void run () {
    SWC_LOGF(LOG_DEBUG, 
      "COMPACT-SPLIT commitlog START "
      "from(%lu/%lu) to(%lu/%lu) fragments=%lu",
      log_left->range->cfg->cid, log_left->range->rid,
      log_right->range->cfg->cid, log_right->range->rid,
      m_fragments.size()
    );
    
    int err;
    size_t skipped = 0;
    size_t moved = 0;
    size_t splitted = 0;
    Fragment::Ptr frag;
    for(auto it = m_fragments.begin(); it< m_fragments.end();) {
      frag = *it;
      if(DB::KeySeq::compare(frag->interval.key_seq, 
          key, frag->interval.key_end) != Condition::GT) {    
        m_fragments.erase(it);
        ++skipped;

      } else if(DB::KeySeq::compare(frag->interval.key_seq, 
                  key, frag->interval.key_begin) == Condition::GT &&
                log_right->take_ownership(err= Error::OK, frag)) {
        log_left->remove(err, frag, false);
        m_fragments.erase(it);
        ++moved;

      } else {
        m_sem.acquire();
        frag->load([this, frag]() { loaded(frag); } );
        ++splitted;
        ++it;
      }
    }
    m_sem.wait_all();

    SWC_LOGF(LOG_DEBUG,
      "COMPACT-SPLIT commitlog FINISH "
      "from(%lu/%lu) to(%lu/%lu) skipped=%lu moved=%lu splitted=%lu",
      log_left->range->cfg->cid, log_left->range->rid,
      log_right->range->cfg->cid, log_right->range->rid,
      skipped, moved, splitted
    );
  }

  private:

  void loaded(Fragment::Ptr frag) {
    int err;
    if(!frag->loaded(err)) {
      frag->processing_decrement();
      SWC_LOG_OUT(LOG_WARN,
        Error::print(
          SWC_LOG_OSTREAM << "COMPACT-SPLIT fragment retrying to ", err);
        frag->print(SWC_LOG_OSTREAM << ' ');
      );
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      frag->load([this, frag]() { loaded(frag); } );

    } else {
      frag->split(err, key, log_left, log_right);
      m_sem.release();
    }
  }

  Core::Semaphore     m_sem;
  Fragments::Vec&     m_fragments;

  const DB::Cell::Key key;
  Fragments::Ptr      log_left;
  Fragments::Ptr      log_right;
};


}}} // namespace SWC::Ranger::CommitLog

#endif // swcdb_ranger_db_CommitLogSplitter_h