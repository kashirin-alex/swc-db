/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CommitLogSplitter_h
#define swcdb_ranger_db_CommitLogSplitter_h


namespace SWC { namespace Ranger { namespace CommitLog {


class Splitter final : private Fragment::LoadCallback {
  public:


  Splitter(const DB::Cell::Key& a_key, Fragments::Vec& fragments,
           Fragments::Ptr a_log_left, Fragments::Ptr a_log_right)
          : m_sem(4), m_fragments(fragments),
            key(a_key), log_left(a_log_left), log_right(a_log_right) {
  }

  Splitter(const Splitter&) = delete;

  Splitter(const Splitter&&) = delete;

  Splitter& operator=(const Splitter&) = delete;

  ~Splitter() noexcept { }

  void run () {
    SWC_LOGF(LOG_DEBUG,
      "COMPACT-SPLIT commitlog START "
      "from(" SWC_FMT_LU "/" SWC_FMT_LU ") to(" SWC_FMT_LU "/" SWC_FMT_LU
      ") fragments=" SWC_FMT_LD,
      log_left->range->cfg->cid, log_left->range->rid,
      log_right->range->cfg->cid, log_right->range->rid,
      int64_t(m_fragments.size())
    );

    int err;
    size_t skipped = 0;
    size_t moved = 0;
    size_t splitted = 0;
    Fragment::Ptr frag;
    for(auto it = m_fragments.cbegin(); it != m_fragments.cend();) {
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
        frag->processing_increment();
        m_sem.acquire();
        frag->load(this);
        ++splitted;
        ++it;
      }
    }
    m_sem.wait_all();

    SWC_LOGF(LOG_DEBUG,
      "COMPACT-SPLIT commitlog FINISH "
      "from(" SWC_FMT_LU "/" SWC_FMT_LU ") to(" SWC_FMT_LU "/" SWC_FMT_LU
      ") skipped=" SWC_FMT_LU " moved=" SWC_FMT_LU " splitted=" SWC_FMT_LU,
      log_left->range->cfg->cid, log_left->range->rid,
      log_right->range->cfg->cid, log_right->range->rid,
      skipped, moved, splitted
    );
  }

  void loaded(Fragment::Ptr&& frag) override {
    int err;
    if(!frag->loaded(err)) {
      SWC_LOG_OUT(LOG_WARN,
        Error::print(
          SWC_LOG_OSTREAM << "COMPACT-SPLIT fragment retrying to ", err);
        frag->print(SWC_LOG_OSTREAM << ' ');
      );
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      frag->load(this);

    } else if(m_splitting.push_and_is_1st(std::move(frag))) {
      struct Task {
        Splitter* ptr;
        SWC_CAN_INLINE
        Task(Splitter* a_ptr) noexcept : ptr(a_ptr) { }
        void operator()() { ptr->split(); }
      };
      Env::Rgr::post(Task(this));
    }
  }

  private:

  SWC_CAN_INLINE
  void split() {
    int err;
    bool more;
    do {
      m_splitting.front()->split(err, key, log_left, log_right);
      more = m_splitting.pop_and_more();
      m_sem.release();
    } while(more);
  }

  Core::Semaphore                m_sem;
  Fragments::Vec&                m_fragments;
  Core::QueueSafe<Fragment::Ptr> m_splitting;

  const DB::Cell::Key            key;
  Fragments::Ptr                 log_left;
  Fragments::Ptr                 log_right;
};


}}} // namespace SWC::Ranger::CommitLog

#endif // swcdb_ranger_db_CommitLogSplitter_h
