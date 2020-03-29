/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swcdb_ranger_db_CompactRange_h
#define swcdb_ranger_db_CompactRange_h


namespace SWC { namespace Ranger {

class CompactRange : public ReqScan {
  public:

  typedef std::shared_ptr<CompactRange>  Ptr;
  
  std::vector<CommitLog::Fragment::Ptr> fragments_old;
  std::atomic<size_t>                   total_cells = 0;

  CompactRange(Compaction::Ptr compactor, RangePtr range,
               const uint32_t cs_size, const uint8_t cs_replication,
               const uint32_t blk_size, const uint32_t blk_cells, 
               const Types::Encoding blk_encoding,
               uint32_t cell_versions, uint64_t cell_ttl, 
               Types::Column col_type);

  virtual ~CompactRange();

  Ptr shared();

  bool reached_limits() override;

  const DB::Cells::Mutable::Selector_t selector() override;
  
  void response(int &err) override;

  private:

  void request_more();

  void process();

  uint32_t create_cs(int& err);

  void write_cells(int& err, DB::Cells::Result* selected_cells);

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
  const uint32_t          cell_versions;
  const uint32_t          cell_ttl; 
  const Types::Column     col_type;

  bool                            tmp_dir = false;
  CellStore::Write::Ptr           cs_writer = nullptr;
  CellStore::Writers              cellstores;
  DB::Cells::Cell*                last_cell = nullptr;

  std::mutex                      m_mutex;
  bool                            m_writing = false;
  std::queue<DB::Cells::Result*>  m_queue;
  std::atomic<bool>               m_stopped = false;
  bool                            m_getting = false;
  int64_t                         m_ts_start;
};





}}
#endif // swcdb_ranger_db_CompactRange_h