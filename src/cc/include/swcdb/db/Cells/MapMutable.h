/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#ifndef swcdb_db_Cells_MapMutable_h
#define swcdb_db_Cells_MapMutable_h

#include "swcdb/core/Mutex.h"
#include "swcdb/db/Columns/Schema.h"
#include "swcdb/db/Cells/Mutable.h"


namespace SWC { namespace DB { namespace Cells {


class ColCells final {

  public:

  typedef std::shared_ptr<ColCells> Ptr;

  const int64_t cid;

  static Ptr make(const int64_t cid, Types::KeySeq seq, 
                  uint32_t versions, uint32_t ttl, Types::Column type);

  static Ptr make(const int64_t cid, Mutable& cells);


  ColCells(const int64_t cid, Types::KeySeq seq, 
           uint32_t versions, uint32_t ttl, Types::Column type);

  ColCells(const int64_t cid, Mutable& cells);

  ~ColCells();

  Types::KeySeq get_sequence() const;

  DB::Cell::Key::Ptr get_first_key();
  
  DB::Cell::Key::Ptr get_key_next(const DB::Cell::Key& eval_key, 
                                  bool start_key=false);

  DynamicBuffer::Ptr get_buff(const DB::Cell::Key& key_start, 
                              const DB::Cell::Key& key_end, 
                              size_t buff_sz, bool& more);

  void add(const DB::Cells::Cell& cell);

  size_t add(const DynamicBuffer& cells);

  size_t add(const DynamicBuffer& cells, const DB::Cell::Key& upto_key,
                                         const DB::Cell::Key& from_key);

  size_t size();

  size_t size_bytes();

  std::string to_string();

  private:
  Mutex     m_mutex;
  Mutable   m_cells;

};



class MapMutable {
  public:
  
  typedef std::shared_ptr<MapMutable>                 Ptr;
  typedef std::unordered_map<int64_t, ColCells::Ptr>  Columns;
  
  MapMutable();

  virtual ~MapMutable();

  bool create(Schema::Ptr schema);

  bool create(const int64_t cid, Types::KeySeq seq, 
              uint32_t versions, uint32_t ttl, Types::Column type);

  bool create(const int64_t cid, Mutable& cells);

  bool exists(const int64_t cid);
  
  void add(const int64_t cid, const Cell& cell);

  ColCells::Ptr get_idx(size_t offset);

  ColCells::Ptr get_col(const int64_t cid);

  void pop(ColCells::Ptr& col);

  void pop(const int64_t cid, ColCells::Ptr& col);

  void remove(const int64_t cid);

  size_t size();

  size_t size(const int64_t cid);

  size_t size_bytes();

  size_t size_bytes(const int64_t cid);

  std::string to_string();

  private:
  Mutex       m_mutex;
  Columns     m_map;

};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/MapMutable.cc"
#endif 

#endif // swcdb_db_Cells_MapMutable_h