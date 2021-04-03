/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Cells_Mutable_h
#define swcdb_db_Cells_Mutable_h

#include "swcdb/db/Cells/ReqScan.h"


namespace SWC { namespace DB { namespace Cells {


class Mutable final {

  public:

  typedef std::shared_ptr<Mutable> Ptr;
  typedef std::vector<Cell*>       Bucket;
  typedef std::vector<Bucket*>     Buckets;

  static const uint16_t bucket_size   = 4096;
  static const uint16_t bucket_max    = 6144;
  static const uint16_t bucket_split  = 2048;
  static const uint8_t  narrow_size   = 20;

  const Types::KeySeq key_seq;
  Types::Column       type;
  uint32_t            max_revs;
  uint64_t            ttl;


  static Bucket* make_bucket(uint16_t reserve = bucket_size);


  struct ConstIterator final {
    const Buckets*            buckets;
    Buckets::const_iterator   bucket;
    Bucket::const_iterator    item;

    SWC_CAN_INLINE
    ConstIterator(const Buckets* buckets, size_t offset = 0) noexcept
                  : buckets(buckets), bucket(buckets->cbegin()) {
      if(offset) {
        for(; bucket != buckets->cend(); ++bucket) {
          if(offset < (*bucket)->size()) {
            item = (*bucket)->cbegin() + offset;
            break;
          }
          offset -= (*bucket)->size();
        }
      } else if(bucket != buckets->cend()) {
        item = (*bucket)->cbegin();
      }
    }

    SWC_CAN_INLINE
    ConstIterator(const ConstIterator& other) noexcept
                  : buckets(other.buckets),
                    bucket(other.bucket), item(other.item) {
    }

    SWC_CAN_INLINE
    ConstIterator& operator=(const ConstIterator& other) noexcept {
      buckets = other.buckets;
      bucket = other.bucket;
      item = other.item;
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return bucket != buckets->cend() && item != (*bucket)->cend();
    }

    SWC_CAN_INLINE
    void operator++() noexcept {
      if(++item == (*bucket)->cend() && ++bucket != buckets->cend())
        item = (*bucket)->cbegin();
    }

  };


  struct Iterator final {
    Buckets*            buckets;
    Buckets::iterator   bucket;
    Bucket::iterator    item;

    SWC_CAN_INLINE
    Iterator() noexcept : buckets(nullptr) { }

    SWC_CAN_INLINE
    Iterator(Buckets* buckets, size_t offset = 0) noexcept
             : buckets(buckets), bucket(buckets->begin()) {
      if(offset) {
        for(; bucket != buckets->end(); ++bucket) {
          if(offset < (*bucket)->size()) {
            item = (*bucket)->begin() + offset;
            break;
          }
          offset -= (*bucket)->size();
        }
      } else if(bucket != buckets->end()) {
        item = (*bucket)->begin();
      }
    }

    SWC_CAN_INLINE
    Iterator(const Iterator& other) noexcept
            : buckets(other.buckets), bucket(other.bucket), item(other.item) {
    }

    SWC_CAN_INLINE
    Iterator& operator=(const Iterator& other) noexcept {
      buckets = other.buckets;
      bucket = other.bucket;
      item = other.item;
      return *this;
    }

    SWC_CAN_INLINE
    operator bool() const noexcept {
      return bucket != buckets->end() && item != (*bucket)->end();
    }

    SWC_CAN_INLINE
    void operator++() noexcept {
      if(++item == (*bucket)->end() && ++bucket != buckets->end())
        item = (*bucket)->begin();
    }

    void insert(Cell* value) {
      item = (*bucket)->insert(item, value);

      if((*bucket)->size() >= bucket_max) {
        auto offset = item - (*bucket)->begin();

        auto nbucket = buckets->insert(++bucket, make_bucket());

        auto it_b = (*(bucket = nbucket-1))->begin() + bucket_split;
        auto it_e = (*bucket)->end();
        (*nbucket)->assign(it_b, it_e);
        (*bucket)->erase  (it_b, it_e);

        if(offset >= bucket_split)
          item = (*(bucket = nbucket))->begin() + offset - bucket_split;
      }
    }

    void remove() {
      (*bucket)->erase(item);
      if((*bucket)->empty() && buckets->size() > 1) {
        delete *bucket;
        if((bucket = buckets->erase(bucket)) != buckets->end())
          item = (*bucket)->begin();
      } else if(item == (*bucket)->end() && ++bucket != buckets->end()) {
        item = (*bucket)->begin();
      }
    }

    void remove(size_t number) {
      while(number) {
        if(item == (*bucket)->begin() && number >= (*bucket)->size()) {
          if(buckets->size() == 1) {
            (*bucket)->clear();
            item = (*bucket)->end();
            return;
          }
          number -= (*bucket)->size();
          delete *bucket;
          bucket = buckets->erase(bucket);

        } else {
          size_t avail = (*bucket)->end() - item;
          if(avail > number) {
            item = (*bucket)->erase(item, item + number);
            return;
          }
          number -= avail;
          (*bucket)->erase(item, (*bucket)->end());
          ++bucket;
        }

        if(bucket == buckets->end())
          return;
        item = (*bucket)->begin();
      }
    }

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
                 const Types::Column typ=Types::Column::PLAIN) noexcept;

  ConstIterator ConstIt(size_t offset = 0) const noexcept;

  Iterator It(size_t offset = 0) noexcept;

  size_t size() const noexcept;

  size_t size_bytes() const noexcept;

  size_t size_of_internal() const noexcept;

  bool empty() const noexcept;

  Cell& front() noexcept;

  Cell& back() noexcept;

  Cell& front() const noexcept;

  Cell& back() const noexcept;

  Cell* operator[](size_t idx) noexcept;

  bool has_one_key() const noexcept;


  void add_sorted(const Cell& cell, bool no_value=false);

  void add_sorted_no_cpy(Cell* cell);

  size_t add_sorted(const uint8_t* ptr, size_t remain);


  void add_raw(const DynamicBuffer& cells);

  void add_raw(const DynamicBuffer& cells, const DB::Cell::Key& upto_key,
                                           const DB::Cell::Key& from_key,
                                           uint32_t skip, bool malformed);

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


  void print(std::ostream& out, bool with_cells=false) const;


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


  bool split(Mutable& cells, bool loaded);

  bool can_split() const noexcept;

  void split(Mutable& cells);

  private:

  void _add_remove(const Cell& e_cell, size_t* offsetp);

  void _add_plain(const Cell& e_cell, size_t* offsetp);

  void _add_counter(const Cell& e_cell, size_t* offsetp);


  size_t _narrow(const DB::Cell::Key& key, size_t offset_hint = 0) const;

  size_t _narrow(const Specs::Interval& specs) const;


  void _add(Cell* cell) noexcept;

  void _remove(Cell* cell) noexcept;

  void _push_back(Cell* cell);

  Cell* _insert(Iterator& it, const Cell& cell);

  void _remove(Iterator& it);

  void _remove(Iterator& it, size_t number, bool wdel = true);

  void _remove_overhead(Iterator& it, const DB::Cell::Key& key,
                        uint32_t revs);


  Buckets               buckets;
  size_t                _bytes;
  size_t                _size;

};



}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Cells/Mutable.cc"
#endif

#endif // swcdb_db_Cells_Mutable_h
