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
  
  std::vector<CommitLog::Fragment::Ptr> fragments_old;
  std::atomic<size_t>                   total_cells = 0;

  CompactRange(const DB::Cells::ReqScan::Config& cfg, 
               Compaction::Ptr compactor, RangePtr range,
               const uint32_t cs_size, const uint8_t cs_replication,
               const uint32_t blk_size, const uint32_t blk_cells, 
               const Types::Encoding blk_encoding);

  virtual ~CompactRange();

  Ptr shared();

  void initialize();

  bool selector(const DB::Cells::Cell& cell, bool& stop) override;
  
  bool reached_limits() override;
  
  bool add_cell_and_more(const DB::Cells::Cell& cell) override;
  
  bool add_cell_set_last_and_more(const DB::Cells::Cell& cell) override;
                 
  bool matching_last(const DB::Cell::Key& key) override;

  void response(int &err) override;

  private:

  void progress_check_timer();

  void stop_check_timer();

  void request_more();

  void process();

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
  const uint8_t           cs_replication;
  const uint32_t          blk_size;
  const uint32_t          blk_cells;
  const Types::Encoding   blk_encoding;

  bool                            tmp_dir = false;
  CellStore::Write::Ptr           cs_writer = nullptr;
  CellStore::Writers              cellstores;
  InBlock*                        m_inblock = nullptr;

  Mutex                           m_mutex;
  bool                            m_writing = false;
  std::queue<InBlock*>            m_queue;
  std::atomic<bool>               m_stopped = false;
  bool                            m_getting = false;
  int64_t                         m_ts_start;

  int64_t                         m_req_ts;
  asio::high_resolution_timer     m_chk_timer;
  std::atomic<bool>               m_chk_final = false;
};





}}
#endif // swcdb_ranger_db_CompactRange_h