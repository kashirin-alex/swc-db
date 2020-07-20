/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_Mutable_h
#define swcdb_db_Cells_Mutable_h

#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {


class Mutable final {
  
  public:

  typedef std::shared_ptr<Mutable>                 Ptr;
  typedef std::vector<Cell*>                       Bucket;
  typedef std::vector<Bucket*>                     Buckets;
  typedef std::function<bool(const Cell&, bool&)>  Selector_t;

  static const uint8_t  _cell_sz      = sizeof(Cell*);
  static const uint8_t  _bucket_sz    = sizeof(Bucket*);

  static const uint16_t bucket_size   = 4096;
  static const uint16_t bucket_max    = 6144;
  static const uint16_t bucket_split  = 2048;
  
  const Types::KeySeq key_seq;
  Types::Column       type;
  uint32_t            max_revs;
  uint64_t            ttl;


  static Bucket* make_bucket(uint16_t reserve = bucket_size);


  class ConstIterator final {
    const Buckets*            buckets;

    public:
    Buckets::const_iterator   bucket;
    Bucket::const_iterator    item;

    ConstIterator(const Buckets* buckets, size_t offset = 0);

    ConstIterator(const ConstIterator& other);

    ConstIterator(const ConstIterator&&) = delete;

    ConstIterator& operator=(const ConstIterator&);

    ~ConstIterator();

    bool avail() const;

    operator bool() const;

    void operator++();

  };


  class Iterator final {
    Buckets*            buckets;

    public:
    Buckets::iterator   bucket;
    Bucket::iterator    item;

    Iterator();

    Iterator(Buckets* buckets, size_t offset = 0);

    Iterator(const Iterator& other);

    Iterator(const Iterator&&) = delete;

    Iterator& operator=(const Iterator& other);

    ~Iterator();

    operator bool() const;

    bool avail() const;

    void operator++();

    bool avail_begin() const;

    void operator--();

    void push_back(Cell*& value);

    void insert(Cell*& value);

    void remove();

    void remove(size_t number);

  };


  static Ptr make(const Types::KeySeq key_seq,
                  const uint32_t max_revs=1, 
                  const uint64_t ttl_ns=0, 
                  const Types::Column type=Types::Column::PLAIN);

  explicit Mutable(const Types::KeySeq key_seq,
                   const uint32_t max_revs=1, const uint64_t ttl_ns=0, 
                   const Types::Column type=Types::Column::PLAIN);

  explicit Mutable(Mutable& other);

  Mutable(const Mutable&&) = delete;

  Mutable& operator=(const Mutable& other) = delete;

  void take_sorted(Mutable& other);

  ~Mutable();

  void free();

  void reset(const uint32_t revs=1, const uint64_t ttl_ns=0, 
             const Types::Column typ=Types::Column::PLAIN);

  void configure(const uint32_t revs=1, const uint64_t ttl_ns=0,
                 const Types::Column typ=Types::Column::PLAIN);

  ConstIterator ConstIt(size_t offset = 0) const;

  Iterator It(size_t offset = 0);

  size_t size() const;
  
  size_t size_bytes() const;

  size_t size_of_internal() const;
  
  bool empty() const;

  Cell*& front();

  Cell*& back();
  
  Cell*& front() const;

  Cell*& back() const;

  Cell*& operator[](size_t idx);

  bool has_one_key() const;


  void add_sorted(const Cell& cell, bool no_value=false);
  
  void add_sorted_no_cpy(Cell* cell);
  
  size_t add_sorted(const uint8_t* ptr, size_t remain);


  void add_raw(const DynamicBuffer& cells);
  
  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& upto_key,
                                           const DB::Cell::Key& from_key);
  
  void add_raw(const Cell& e_cell);
  
  void add_raw(const Cell& e_cell, size_t* offsetp);

  Cell* takeout(size_t idx);
  
  Cell* takeout_begin(size_t idx);

  Cell* takeout_end(size_t idx);


  void write_and_free(DynamicBuffer& cells, uint32_t& cell_count,
                      Interval& intval, uint32_t threshold, 
                      uint32_t max_cells);

  bool write_and_free(const DB::Cell::Key& key_start, 
                      const DB::Cell::Key& key_finish,
                      DynamicBuffer& cells, uint32_t threshold);


  std::string to_string(bool with_cells=false) const;
  

  void get(int32_t idx, DB::Cell::Key& key) const;
  
  bool get(const DB::Cell::Key& key, Condition::Comp comp, 
           DB::Cell::Key& res) const;

  void write(DynamicBuffer& cells) const;


  void scan(ReqScan* req) const;

  void scan_version_single(ReqScan* req) const;

  void scan_version_multi(ReqScan* req) const;

  void scan_test_use(const Specs::Interval& specs, DynamicBuffer& result, 
                     size_t& count, size_t& skips) const;

  bool scan_after(const DB::Cell::Key& after, const DB::Cell::Key& to, 
                  Mutable& cells) const;

  void expand(Interval& interval) const;

  void expand_begin(Interval& interval) const;

  void expand_end(Interval& interval) const;
  

  void split(Mutable& cells, DB::Cell::Key& end_1st, DB::Cell::Key& end_2nd, 
             bool loaded);

  void split(Mutable& cells);

  private:

  void _add_remove(const Cell& e_cell, size_t* offsetp);

  void _add_plain(const Cell& e_cell, size_t* offsetp);

  void _add_counter(const Cell& e_cell, size_t* offsetp);
  

  size_t _narrow(const DB::Cell::Key& key, size_t offset_hint = 0) const;

  void _add(Cell* cell);

  void _remove(Cell* cell);

  void _push_back(Cell* cell);

  Cell* _insert(Iterator& it, const Cell& cell);
  
  void _remove(Iterator& it);

  void _remove(Iterator& it, size_t number, bool wdel = true);

  void _remove_overhead(Iterator& it, const DB::Cell::Key& key, uint32_t revs);


  Buckets               buckets;
  size_t                _bytes;
  size_t                _size;
  static const uint8_t  narrow_sz = 20;
};


}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Mutable.cc"
#endif 

#endif // swcdb_db_Cells_Mutable_h