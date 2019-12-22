/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_RangeBlock_h
#define swcdb_db_Files_RangeBlock_h

#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace Files { namespace Range {

class Block final {
  public:
  
  typedef Block* Ptr;
  
  enum State {
    NONE,
    LOADING,
    LOADED,
    REMOVED
  };
    
  Block::Ptr  next;
  Block::Ptr  prev;


  static Ptr make(const DB::Cells::Interval& interval,
                  Blocks* blocks, 
                  State state=State::NONE);
                  
  explicit Block(const DB::Cells::Interval& interval, 
                 Blocks* blocks, State state=State::NONE);

  ~Block();

  Ptr ptr();

  const bool _is_gt_prev_end(const DB::Cell::Key& key);

  const bool is_consist(const DB::Cells::Interval& intval);
  
  const bool is_in_end(const DB::Cell::Key& key);

  const bool is_gt_end(const DB::Cell::Key& key);

  const bool is_next(const DB::Specs::Interval& spec);

  const bool includes(const DB::Specs::Interval& spec);
  
  void preload();

  const bool add_logged(const DB::Cells::Cell& cell);
    
  void load_cells(const DB::Cells::Mutable& cells);

  const size_t load_cells(const uint8_t* buf, size_t remain, 
                          size_t avail, bool& was_splitted);

  const bool splitter();

  void loaded_cellstores(int err);

  void loaded_logs(int err);

  const bool scan(DB::Cells::ReqScan::Ptr req);

  Ptr split(bool loaded);
    
  Ptr _split(bool loaded);

  void add(Ptr blk);

  /*
  void expand_next_and_release(DB::Cell::Key& key_begin);

  void merge_and_release(Ptr blk);
  */

  const size_t release();

  void processing_increment();

  void processing_decrement();

  const bool removed();

  const bool loaded();

  const bool processing() const;

  const size_t size();

  const size_t _size();
  
  const size_t size_bytes();

  void free_key_begin();

  void free_key_end();

  const std::string to_string();
  
  private:

  const bool _scan(DB::Cells::ReqScan::Ptr req, bool synced=false);

  void run_queue(int& err);

  std::shared_mutex       m_mutex;
  DB::Cells::Interval     m_interval;
  DB::Cells::Mutable      m_cells;
  Blocks*                 m_blocks;

  std::shared_mutex                    m_mutex_state;
  State                                m_state;
  std::atomic<size_t>                  m_processing;
  std::queue<DB::Cells::ReqScan::Ptr>  m_queue;

};






}}}

#endif