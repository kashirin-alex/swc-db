/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_CompactRange_h
#define swcdb_ranger_db_CompactRange_h


namespace SWC { namespace Ranger {

class CompactRange final : public ReqScan {
  struct InBlock;
  public:

  typedef std::shared_ptr<CompactRange>  Ptr;
  
  CommitLog::Fragments::Vec              fragments_old;

  CompactRange(Compaction* compactor, const RangePtr& range,
               const uint32_t cs_size, const uint32_t blk_size);

  virtual ~CompactRange();

  Ptr shared();

  void initialize();

  bool with_block() override;

  bool selector(const Types::KeySeq key_seq, 
                const DB::Cells::Cell& cell, bool& stop) override;
  
  bool reached_limits() override;
  
  bool add_cell_and_more(const DB::Cells::Cell& cell) override;
  
  void response(int &err) override;

  private:

  void initial_commitlog(int tnum);

  void initial_commitlog_done(const CommitLog::Compact* compact);

  bool is_slow_req(int64_t& median) const;

  void commitlog(int tnum);

  void commitlog_done(const CommitLog::Compact* compact);

  void progress_check_timer();

  void stop_check_timer();

  void request_more();

  void process_interval();

  void process_encode();

  void process_write();

  csid_t create_cs(int& err);

  void write_cells(int& err, InBlock* inblock);

  void add_cs(int& err);

  ssize_t can_split_at();

  void finalize();

  void mngr_create_range(uint32_t split_at);

  void mngr_remove_range(const RangePtr& new_range);

  void split(rid_t new_rid, uint32_t split_at);

  void apply_new(bool clear = false);

  bool completion();
  
  void finished(bool clear);

  void quit();


  Compaction*             compactor;
  RangePtr                range;
  const uint32_t          cs_size;
  const uint32_t          blk_size;
  const uint32_t          blk_cells;
  const Types::Encoding   blk_encoding;
  DB::Cell::Key           m_required_key_last;

  bool                            tmp_dir = false;
  CellStore::Write::Ptr           cs_writer = nullptr;
  CellStore::Writers              cellstores;
  InBlock*                        m_inblock = nullptr;

  QueueSafe<InBlock*>             m_q_intval;
  QueueSafe<InBlock*>             m_q_encode;
  QueueSafe<InBlock*>             m_q_write;

  std::atomic<uint64_t>           total_cells = 0;
  std::atomic<uint64_t>           total_blocks = 0;

  std::atomic<uint64_t>           time_intval = 0;
  std::atomic<uint64_t>           time_encode = 0;
  std::atomic<uint64_t>           time_write = 0;

  std::atomic<uint8_t>            state_default;
  std::atomic<int64_t>            req_last_time;
  std::atomic<int64_t>            req_ts;

  std::atomic<bool>               m_stopped = false;
  std::atomic<bool>               m_chk_final = false;

  Mutex                           m_mutex;
  bool                            m_getting;
  size_t                          m_log_sz;
  asio::high_resolution_timer     m_chk_timer;

};





}}
#endif // swcdb_ranger_db_CompactRange_h