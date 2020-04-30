/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_ranger_db_RangeBlock_h
#define swc_ranger_db_RangeBlock_h

#include <shared_mutex>

#include "swcdb/fs/Interface.h"
#include "swcdb/db/Cells/Mutable.h"
#include "swcdb/db/Cells/MutableVec.h"



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
    
  Blocks*     blocks;
  Block::Ptr  next;
  Block::Ptr  prev;


  static Ptr make(const DB::Cells::Interval& interval,
                  Blocks* blocks, 
                  State state=State::NONE);
                  
  explicit Block(const DB::Cells::Interval& interval, 
                 Blocks* blocks, State state=State::NONE);

  ~Block();

  Ptr ptr();

  void schema_update();

  bool is_consist(const DB::Cells::Interval& intval) const;
  
  bool is_in_end(const DB::Cell::Key& key) const;

  bool is_next(const DB::Specs::Interval& spec);

  bool includes(const DB::Specs::Interval& spec);
  
  void preload();

  bool add_logged(const DB::Cells::Cell& cell);
    
  void load_cells(const DB::Cells::MutableVec& cells);

  size_t load_cells(const uint8_t* buf, size_t remain, 
                    uint32_t revs, size_t avail, 
                    bool& was_splitted, bool synced=false);

  bool splitter();

  bool scan(ReqScan::Ptr req);
  
  void loaded(int err);
  
  Ptr split(bool loaded);
    
  Ptr _split(bool loaded);

  void _add(Ptr blk);

  void _set_prev_key_end(const DB::Cell::Key& key);
  
  Condition::Comp _cond_key_end(const DB::Cell::Key& key) const;

  void _set_key_end(const DB::Cell::Key& key);
  
  /*
  void expand_next_and_release(DB::Cell::Key& key_begin);

  void merge_and_release(Ptr blk);
  */

  size_t release();

  void processing_increment();

  void processing_decrement();

  bool removed();

  bool loaded();

  bool processing() const;

  size_t size();

  size_t _size() const;
  
  size_t size_bytes();

  size_t _size_bytes() const;

  //bool need_split();

  bool _need_split() const;

  void free_key_begin();

  void free_key_end();

  std::string to_string();
  
  private:

  bool _scan(ReqScan::Ptr req, bool synced=false);

  void run_queue(int& err);

  Mutex                     m_mutex_intval;
  DB::Cells::Interval       m_interval;
  DB::Cell::Key             m_prev_key_end;

  std::shared_mutex         m_mutex;
  DB::Cells::Mutable        m_cells;

  Mutex                     m_mutex_state;
  State                     m_state;
  std::atomic<size_t>       m_processing;
  QueueSafe<ReqScan::Ptr>   m_queue;

};






}}

#endif // swc_ranger_db_RangeBlock_h