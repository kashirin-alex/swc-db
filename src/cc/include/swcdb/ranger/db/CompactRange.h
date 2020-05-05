/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_CompactRange_h
#define swcdb_ranger_db_CompactRange_h


namespace SWC { namespace Ranger {

class CompactRange : public ReqScan {
  struct InBlock;
  public:

  typedef std::shared_ptr<CompactRange>  Ptr;
  
  CommitLog::Fragments::Vec              fragments_old;

  CompactRange(Compaction::Ptr compactor, RangePtr range,
               const uint32_t cs_size, const uint32_t blk_size);

  virtual ~CompactRange();

  Ptr shared();

  void initialize();

  bool with_block() override;

  bool selector(const Types::KeySeq key_seq, 
                const DB::Cells::Cell& cell, bool& stop) override;
  
  bool reached_limits() override;
  
  bool add_cell_and_more(const DB::Cells::Cell& cell) override;
  
  bool add_cell_set_last_and_more(const DB::Cells::Cell& cell) override;
                 
  bool matching_last(const DB::Cell::Key& key) override;

  void response(int &err) override;

  private:

  void initial_commitlog(int tnum);

  void initial_commitlog_done(CompactRange::Ptr ptr, 
                              const CommitLog::Compact* compact);

  void commitlog(int tnum, uint8_t state);

  void commitlog_done(const CommitLog::Compact* compact, uint8_t state);

  void progress_check_timer();

  void stop_check_timer();

  void request_more();

  void process_interval();

  void process_encode();

  void process_write();

  uint32_t create_cs(int& err);

  void write_cells(int& err, InBlock* inblock);

  void add_cs(int& err);

  void finalize();

  void mngr_create_range(uint32_t split_at);

  void mngr_remove_range(RangePtr new_range);

  void split(int64_t new_rid, uint32_t split_at);

  void apply_new(bool clear = false);

  void quit();


  Compaction::Ptr         compactor;
  RangePtr                range;
  const uint32_t          cs_size;
  const uint32_t          blk_size;
  const uint32_t          blk_cells;
  const Types::Encoding   blk_encoding;

  bool                            tmp_dir = false;
  CellStore::Write::Ptr           cs_writer = nullptr;
  CellStore::Writers              cellstores;
  InBlock*                        m_inblock = nullptr;

  QueueSafe<InBlock*>             m_q_intval;
  QueueSafe<InBlock*>             m_q_encode;
  QueueSafe<InBlock*>             m_q_write;

  const uint64_t                  ts_start;

  std::atomic<uint64_t>           total_cells = 0;
  std::atomic<uint64_t>           total_blocks = 0;

  std::atomic<uint64_t>           time_intval = 0;
  std::atomic<uint64_t>           time_encode = 0;
  std::atomic<uint64_t>           time_write = 0;
  std::atomic<uint64_t>           m_ts_req;

  std::atomic<bool>               m_stopped = false;
  std::atomic<bool>               m_chk_final = false;

  Mutex                           m_mutex;
  bool                            m_getting;
  asio::high_resolution_timer     m_chk_timer;

};





}}
#endif // swcdb_ranger_db_CompactRange_h