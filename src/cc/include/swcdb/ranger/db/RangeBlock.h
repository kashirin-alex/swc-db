/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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
    NONE    = 0x00,
    LOADING = 0x01,
    LOADED  = 0x02,
    REMOVED = 0x03,
  };

  enum ScanState {
    UKNOWN    = 0x00,
    QUEUED    = 0x01,
    RESPONDED = 0x02,
    SYNCED    = 0x03
  };
    
  Blocks*     blocks;
  Block::Ptr  next;
  Block::Ptr  prev;


  static Ptr make(const DB::Cells::Interval& interval,
                  Blocks* blocks, 
                  State state=State::NONE);
                  
  explicit Block(const DB::Cells::Interval& interval, 
                 Blocks* blocks, State state=State::NONE);

  Block(const Block&) = delete;

  Block(const Block&&) = delete;

  Block& operator=(const Block&) = delete;

  ~Block();

  size_t size_of() const;

  Ptr ptr();

  void schema_update();

  void set_prev_key_end(const DB::Cell::Key& key);

  Condition::Comp cond_key_end(const DB::Cell::Key& key) const;

  void set_key_end(const DB::Cell::Key& key);
  
  void free_key_end();
  
  void get_key_end(DB::Cell::Key& key) const;

  bool is_consist(const DB::Cells::Interval& intval) const;
  
  bool is_in_end(const DB::Cell::Key& key) const;

  bool _is_in_end(const DB::Cell::Key& key) const;

  bool is_next(const DB::Specs::Interval& spec) const;

  bool includes(const DB::Specs::Interval& spec) const;

  bool _includes_begin(const DB::Specs::Interval& spec) const;

  bool includes_end(const DB::Specs::Interval& spec) const;

  void preload();

  bool add_logged(const DB::Cells::Cell& cell);
    
  void load_cells(const DB::Cells::MutableVec& cells);

  size_t load_cells(const uint8_t* buf, size_t remain, 
                    uint32_t revs, size_t avail, 
                    bool& was_splitted, bool synced=false);

  bool splitter();

  ScanState scan(const ReqScan::Ptr& req);
  
  void loaded(int err, const BlockLoader* loader);
  
  Ptr split(bool loaded);
    
  Ptr _split(bool loaded);

  void _add(Ptr blk);

  size_t release();

  void processing_increment();

  void processing_decrement();

  bool removed();

  bool loaded();

  bool need_load();

  bool processing() const;

  size_t size();

  size_t _size() const;
  
  size_t size_bytes();

  size_t size_of_internal();

  bool _need_split() const;

  std::string to_string();
  
  private:

  ScanState _scan(const ReqScan::Ptr& req, bool synced=false);

  void run_queue(int& err);


  mutable std::shared_mutex   m_mutex;
  DB::Cells::Mutable          m_cells;

  mutable LockAtomic::Unique  m_mutex_intval;
  DB::Cell::Key               m_prev_key_end;
  DB::Cell::Key               m_key_end;

  Mutex                       m_mutex_state;
  State                       m_state;
  std::atomic<size_t>         m_processing;

  struct ReqQueue {
    ReqScan::Ptr  req;
    const int64_t ts;
  };
  QueueSafe<ReqQueue>         m_queue;

};






}}

#endif // swc_ranger_db_RangeBlock_h