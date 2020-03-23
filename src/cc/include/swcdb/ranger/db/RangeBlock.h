/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_RangeBlock_h
#define swc_ranger_db_RangeBlock_h

#include <shared_mutex>
#include <queue>

#include "swcdb/core/StatefullSharedMutex.h"
#include "swcdb/fs/Interface.h"
#include "swcdb/db/Cells/Mutable.h"



namespace SWC { namespace Ranger { 

 //Forawrd Declaration
class Blocks;
class BlockLoader;


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
  Blocks*     blocks;


  static Ptr make(const DB::Cells::Interval& interval,
                  Blocks* blocks, 
                  State state=State::NONE);
                  
  explicit Block(const DB::Cells::Interval& interval, 
                 Blocks* blocks, State state=State::NONE);

  ~Block();

  Ptr ptr();

  void schema_update();

  const bool is_consist(const DB::Cells::Interval& intval);
  
  const bool is_in_end(const DB::Cell::Key& key);

  const bool is_next(const DB::Specs::Interval& spec);

  const bool includes(const DB::Specs::Interval& spec);
  
  void preload();

  const bool add_logged(const DB::Cells::Cell& cell);
    
  void load_cells(const DB::Cells::Mutable& cells);

  const size_t load_cells(const uint8_t* buf, size_t remain, 
                          uint32_t revs, size_t avail, 
                          bool& was_splitted, bool synced=false);

  const bool splitter();

  const bool scan(ReqScan::Ptr req);
  
  void loaded(int err);
  
  Ptr split(bool loaded);
    
  Ptr _split(bool loaded);

  void _add(Ptr blk);

  void _set_prev_key_end(const DB::Cell::Key& key);
  
  const Condition::Comp _cond_key_end(const DB::Cell::Key& key) const;

  void _set_key_end(const DB::Cell::Key& key);
  
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

  const uint32_t size();

  const uint32_t _size() const;
  
  const size_t size_bytes();

  const size_t _size_bytes() const;

  const bool need_split();

  const bool _need_split() const;

  void free_key_begin();

  void free_key_end();

  const std::string to_string();
  
  private:

  const bool _scan(ReqScan::Ptr req, bool synced=false);

  void run_queue(int& err);

  StatefullSharedMutex      m_mutex; 
  DB::Cells::Interval       m_interval;
  DB::Cells::Mutable        m_cells;
  DB::Cell::Key             m_prev_key_end;

  std::shared_mutex         m_mutex_state;
  State                     m_state;
  uint8_t                   m_load_require;
  std::atomic<size_t>       m_processing;
  std::queue<ReqScan::Ptr>  m_queue;

};






}}

#endif // swc_ranger_db_RangeBlock_h