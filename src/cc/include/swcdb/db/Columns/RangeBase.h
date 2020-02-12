/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_RangeBase_h
#define swcdb_lib_db_Columns_RangeBase_h

#include <mutex>
#include <shared_mutex>

#include "swcdb/db/Files/RgrData.h"
#include "swcdb/db/Cells/Interval.h"


namespace SWC { namespace DB {


class RangeBase : public std::enable_shared_from_this<RangeBase> {
  public:
  typedef std::shared_ptr<RangeBase> Ptr;
  
  inline static const std::string column_dir = "C"; 
  inline static const std::string range_dir = "/R"; 
  // (swc.fs.path.data)+column_dir+/+{cid}+/range_dir+/+{rid}+/+(types)
  inline static const std::string ranger_data_file = "ranger.data";
  inline static const std::string range_data_file = "range.data";

  inline static const std::string cellstores_dir = "cs";
  inline static const std::string cellstores_bak_dir = "cs_bak";
  inline static const std::string cellstores_tmp_dir = "cs_tmp";
  
  inline static const std::string log_dir = "log"; 

  inline static const std::string 
  get_column_path() {
    std::string s(column_dir);
    return s;
  }

  inline static const std::string 
  get_column_path(const int64_t cid) {
    std::string s(get_column_path());
    //if(!s.empty())
    s.append("/");
    FS::set_structured_id(std::to_string(cid), s);
    return s;
  }

  inline static const std::string 
  get_path(const int64_t cid) {
    std::string s(get_column_path(cid));
    s.append(range_dir);
    return s;
  }
  
  inline static const std::string 
  get_path(const int64_t cid, const int64_t rid) {
    std::string s(get_path(cid));
    s.append("/");
    FS::set_structured_id(std::to_string(rid), s);
    s.append("/");
    return s;
  }

  const ColumnCfg*  cfg;
  const int64_t     rid;
            
  RangeBase(const ColumnCfg* cfg, const int64_t rid)
            : cfg(cfg), rid(rid), 
              m_path(get_path(cfg->cid, rid)) { 
  }

  RangeBase(const ColumnCfg* cfg, const int64_t rid, 
            const Cells::Interval& intval)
            : cfg(cfg), rid(rid), m_interval(intval),
              m_path(get_path(cfg->cid, rid)) {
  }

  virtual ~RangeBase() { }
  
  Ptr shared() {
    return shared_from_this();
  }

  const std::string get_path(const std::string suff) const {
    std::string s(m_path);
    s.append(suff);
    return s;
  }

  const std::string get_path_cs(const int64_t cs_id) const {
    std::string s(m_path);
    s.append(cellstores_dir);
    s.append("/");
    s.append(std::to_string(cs_id));
    s.append(".cs");
    return s;
  }

  const std::string get_path_cs_on(const std::string folder, 
                                  const int64_t cs_id) const {
    std::string s(m_path);
    s.append(folder);
    s.append("/");
    s.append(std::to_string(cs_id));
    s.append(".cs");
    return s;
  }
  

  Files::RgrData::Ptr get_last_rgr(int &err) {
    return Files::RgrData::get_rgr(err, get_path(ranger_data_file));
  }

  void set(const Cells::Interval& interval){
    std::scoped_lock lock(m_mutex);
    m_interval.copy(interval);
  }

  bool after(const Ptr& range) {
    std::shared_lock lock(m_mutex);
    return range->before(m_interval);
  }

  bool const before(const Cells::Interval& intval) {
    std::shared_lock lock(m_mutex);
    return (!intval.was_set && m_interval.was_set) 
            ||
           (intval.was_set && m_interval.was_set 
            && intval.is_in_end(m_interval.key_end));
  }

  bool const equal(const Cells::Interval& intval) {
    std::shared_lock lock(m_mutex);
    return m_interval.equal(intval);
  }

  bool includes(const DB::Cell::Key& range_begin, 
                const DB::Cell::Key& range_end, uint32_t any_is=0) {
    std::shared_lock lock(m_mutex);
    return (
        m_interval.key_begin.empty() || 
        m_interval.key_begin.count == any_is ||
        range_end.empty() || 
        range_end.compare(m_interval.key_begin) != Condition::GT
      ) && (
        m_interval.key_end.empty() || 
        m_interval.key_end.count == any_is ||
        range_begin.empty() || 
        range_begin.compare(m_interval.key_end) != Condition::LT
      );
  }

  void get_interval(Cells::Interval& interval) {
    std::shared_lock lock(m_mutex);
    interval.copy(m_interval);
  }

  void get_interval(Cell::Key& key_begin, Cell::Key& key_end) {
    std::shared_lock lock(m_mutex);
    key_begin.copy(m_interval.key_begin);
    key_end.copy(m_interval.key_end);
  }

  const bool is_any_begin() {
    std::shared_lock lock(m_mutex);
    return m_interval.key_begin.empty();
  }

  void get_key_begin(Cell::Key& key_begin) {
    std::shared_lock lock(m_mutex);
    key_begin.copy(m_interval.key_begin);
  }

  const bool is_any_end() {
    std::shared_lock lock(m_mutex);
    return m_interval.key_end.empty();
  }

  void get_key_end(Cell::Key& key_end) {
    std::shared_lock lock(m_mutex);
    key_end.copy(m_interval.key_end);
  }

 void get_interval(Specs::Key& key_start, Specs::Key& key_end) {
    std::shared_lock lock(m_mutex);
    key_start.set(m_interval.key_begin, Condition::GE);
    key_end.set(m_interval.key_end, Condition::LE);
  }

  const bool align(const Cells::Interval& interval) {
    std::scoped_lock lock(m_mutex);
    return m_interval.align(interval);
  }

  const std::string to_string() {
    std::shared_lock lock(m_mutex);
    std::string s(cfg->to_string());
    s.append(" rid=");
    s.append(std::to_string(rid));
    s.append(" ");
    s.append(m_interval.to_string());
    return s;
  }

  
  private:
  const std::string         m_path;

  protected:
  std::shared_mutex         m_mutex;
  Cells::Interval           m_interval;
};

}}
#endif