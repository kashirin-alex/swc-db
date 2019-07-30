/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_lib_db_Columns_Column_h
#define swcdb_lib_db_Columns_Column_h

#include "swcdb/lib/db/Files/RsData.h"

#include "Schema.h"
#include "Range.h"

#include <mutex>
#include <memory>
#include <unordered_map>

#include "swcdb/lib/db/Protocol/req/UnloadRange.h"

namespace SWC {

typedef std::unordered_map<int64_t, RangePtr> RangesMap;
typedef std::pair<int64_t, RangePtr> RangesMapPair;


class Column : public std::enable_shared_from_this<Column> {
  
  public:

  Column(int64_t id)
        : cid(id),
          ranges(std::make_shared<RangesMap>()) {
    
    std::string col_range_path(Range::get_path(cid));
    m_err = 0;
    if(!EnvFsInterface::fs()->exists(m_err, col_range_path))
      EnvFsInterface::fs()->mkdirs(m_err, col_range_path);
      if(m_err == 17)
        m_err = 0;
  }

  virtual ~Column(){}

  int has_err(){
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_err;
  }

  void ranges_by_fs(int &err, FS::IdEntries_t &entries){
    EnvFsInterface::interface()->get_structured_ids(err, Range::get_path(cid), entries);
  }

  RangePtr get_range(int64_t rid, bool initialize=false, bool is_rs=false){
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = ranges->find(rid);
    if (it != ranges->end())
      return it->second;

    if(initialize) {
      RangePtr range = std::make_shared<Range>(cid, rid, schema);
      if(!is_rs || load_range(range)) {
        ranges->insert(RangesMapPair(rid, range));
      }
      return range;
    }
    return nullptr;
  }

  private:

  bool load_range(RangePtr range){
    HT_DEBUGF("LOADING RANGE cid=%d rid=%d", range->cid, range->rid);
    if(!range->set_dirs())
      return false;
    Files::RsDataPtr rs_data = EnvRsData::get();

    Files::RsDataPtr rs_last = range->get_last_rs();
    std::cout << " RS-LAST=" << rs_last->to_string() << "\n"
              << " RS-NEW =" << rs_data->to_string() << "\n";
    if(!has_endpoint(rs_data->endpoints, rs_last->endpoints)){
      // if online, (means rs-mngr had comm issues with the RS-LAST )
      //   req. unload (sync) 

      client::ClientConPtr old_conn = EnvClients::get()->rs_service->get_connection(
          rs_last->endpoints, std::chrono::milliseconds(10000), 1);
      if(old_conn != nullptr)
        Protocol::Req::UnloadRange(old_conn, range);
   
    }

    if(!range->set_last_rs())
      return false;
    
    if(range->load())
      HT_DEBUGF("LOADED RANGE cid=%d rid=%d", range->cid, range->rid);
      return true;
    
    HT_WARNF("LOAD RANGE FAILED cid=%d rid=%d", range->cid, range->rid);
    return false;
  }

  std::mutex        m_mutex;
  int               m_err;
  int64_t           cid;
  SchemaPtr         schema;
  std::shared_ptr<RangesMap> ranges;

};
typedef std::shared_ptr<Column> ColumnPtr;

}

#endif