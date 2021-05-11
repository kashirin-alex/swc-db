/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_queries_update_MetricsReporting_h
#define swcdb_common_queries_update_MetricsReporting_h


#include "swcdb/common/sys/Resources.h"
#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Query/Update/Handlers/Metrics.h"


namespace SWC { namespace Common { namespace Query { namespace Update {
namespace Metric {

using namespace client::Query::Update::Handlers::Metric;



namespace { // local namespace

static size_t encoded_length(
        const Common::Stats::MinMaxAvgCount<uint64_t>& value,
        bool with_count) noexcept {
  size_t sz = 0;
  if(value.count && value.total) {
    uint64_t avg = value.avg();
    if(avg != value.min)
      sz += 2 + Serialization::encoded_length_vi64(value.min);
    if(avg != value.max)
      sz += 2 + Serialization::encoded_length_vi64(value.max);
    sz += 2 + Serialization::encoded_length_vi64(avg);
    if(with_count)
      sz += 2 + Serialization::encoded_length_vi64(value.count);
  }
  return sz;
}

static void add_value(
        const Common::Stats::MinMaxAvgCount<uint64_t>& value,
        uint8_t field_start, bool with_count,
        DB::Cell::Serial::Value::FieldsWriter& wfields) noexcept {
  if(value.count && value.total) {
    uint64_t avg = value.avg();
    if(avg != value.min)
      wfields.add(field_start, int64_t(value.min));
    if(avg != value.max)
      wfields.add(field_start + 1, int64_t(value.max));
    wfields.add(field_start + 2, int64_t(avg));
    if(with_count)
      wfields.add(field_start + 3, int64_t(value.count));
  }
}

}


template<typename CommandsT>
class Item_Net : public Base {
  /* Cell-Serialization:
      key:    [levels, "net", "{address}", is_secure()?"SECURE":"PLAIN"]
      value:  {FIELD_ID}:I:{value}, ...
  */

  public:
  typedef std::unique_ptr<Item_CountVolume> Ptr;

  static const uint8_t FIELD_CONN_OPEN        = 0;
  static const uint8_t FIELD_CONN_ACC         = 1;
  static const uint8_t FIELD_CONN_EST         = 2;

  static const uint8_t FIELD_BYTES_SENT_MIN   = 3;
  static const uint8_t FIELD_BYTES_SENT_MAX   = 4;
  static const uint8_t FIELD_BYTES_SENT_AVG   = 5;
  static const uint8_t FIELD_BYTES_SENT_TRX   = 6;

  static const uint8_t FIELD_BYTES_RECV_MIN   = 7;
  static const uint8_t FIELD_BYTES_RECV_MAX   = 8;
  static const uint8_t FIELD_BYTES_RECV_AVG   = 9;
  static const uint8_t FIELD_BYTES_RECV_TRX   = 10;

  static const uint8_t FIELD_EV_COMMAND_START = 100; // {100 + CMD}

  struct Addr {
    const asio::ip::address                      addr;
    Core::Atomic<uint64_t>                       conn_open;
    Core::Atomic<uint64_t>                       conn_accept;
    Core::Atomic<uint64_t>                       conn_est;
    Common::Stats::MinMaxAvgCount_Safe<uint64_t> bytes_sent;
    Common::Stats::MinMaxAvgCount_Safe<uint64_t> bytes_recv;
    std::vector<Core::Atomic<uint64_t>>          commands;

    Addr(const Comm::EndPoint& endpoint) noexcept
        : addr(endpoint.address()),
          conn_open(0), conn_accept(0), conn_est(0),
          commands(CommandsT::max_command()) {
    }
  };

  Item_Net(const Comm::EndPoints& endpoints, bool using_secure) {
    for(uint8_t secure = 0; secure <= using_secure; ++secure) {
      m_addresses[secure].reserve(endpoints.size());
      for(auto& endpoint : endpoints)
        m_addresses[secure].emplace_back(new Addr(endpoint));
    }
  }

  virtual ~Item_Net() { }

  Addr* get(const asio::ip::address& for_addr, bool secure) const noexcept {
    for(auto& addr : m_addresses[secure]) {
      if(addr->addr.is_unspecified() || addr->addr == for_addr)
        return addr.get();
    }
    return nullptr;
  }

  void accepted(const Comm::EndPoint& endpoint, bool secure) noexcept {
    if(Addr* addr = get(endpoint.address(), secure))
      addr->conn_accept.fetch_add(1);
  }

  void connected() noexcept {
    Addr* addr = m_addresses[0].front().get();
    addr->conn_open.fetch_add(1);
    addr->conn_est.fetch_add(1);
  }

  void disconnected() noexcept {
    Addr* addr = m_addresses[0].front().get();
    addr->conn_open.fetch_sub(1);
  }

  void connected(const Comm::ConnHandlerPtr& conn) noexcept {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure())) {
      addr->conn_open.fetch_add(1);
      addr->conn_est.fetch_add(1);
    }
  }

  void disconnected(const Comm::ConnHandlerPtr& conn) noexcept {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->conn_open.fetch_sub(1);
  }

  void command(const Comm::ConnHandlerPtr& conn, uint8_t cmd) noexcept {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->commands[cmd].fetch_add(1);
  }

  void error(const Comm::ConnHandlerPtr& conn) noexcept {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->commands.back().fetch_add(1);
  }

  void sent(const Comm::ConnHandlerPtr& conn, size_t bytes) noexcept {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->bytes_sent.add(bytes);
  }

  void received(const Comm::ConnHandlerPtr& conn, size_t bytes) noexcept {
    if(Addr* addr = get(conn->endpoint_local.address(), conn->is_secure()))
      addr->bytes_recv.add(bytes);
  }

  virtual void definitions(client::Query::Update::Handlers::Base::Column* colp,
                           const DB::Cell::KeyVec& parent_key) override {
    bool secure = false;
    _do_layer:
    for(auto& addr : m_addresses[secure]) {
      DB::Cell::KeyVec key;
      key.reserve(parent_key.size() + 3);
      key.copy(parent_key);
      key.add("net");
      key.add(addr->addr.to_string());
      key.add(secure ? "SECURE" : "PLAIN");

      DB::Cells::Cell cell;
      cell.flag = DB::Cells::INSERT;
      cell.set_time_order_desc(true);
      cell.key.add(key);

      DB::Cell::Serial::Value::FieldsWriter wfields;
      wfields.ensure(214 + addr->commands.size() * 11);
      wfields.add(FIELD_CONN_OPEN, "Open Connections", 16);
      wfields.add(FIELD_CONN_ACC, "Accepted Connections", 20);
      wfields.add(FIELD_CONN_EST, "Connections Established", 23);

      wfields.add(FIELD_BYTES_SENT_MIN,  "Send Bytes Minimum", 18);
      wfields.add(FIELD_BYTES_SENT_MAX,  "Send Bytes Maximum", 18);
      wfields.add(FIELD_BYTES_SENT_AVG,  "Send Bytes Average", 18);
      wfields.add(FIELD_BYTES_SENT_TRX,  "Send Transactions", 17);

      wfields.add(FIELD_BYTES_RECV_MIN,  "Receive Bytes Minimum", 21);
      wfields.add(FIELD_BYTES_RECV_MAX,  "Receive Bytes Maximum", 21);
      wfields.add(FIELD_BYTES_RECV_AVG,  "Receive Bytes Average", 21);
      wfields.add(FIELD_BYTES_RECV_TRX,  "Receive Transactions", 21);

      for(size_t i=0; i < addr->commands.size(); ++i) {
        std::string cmd(CommandsT::to_string(i));
        wfields.add(i + FIELD_EV_COMMAND_START, cmd + " Requests");
      }
      cell.set_value(DB::Types::Encoder::ZSTD, wfields.base, wfields.fill());

      colp->add(cell);
    }

    if(!secure) {
      secure = true;
      if(!m_addresses[secure].empty())
        goto _do_layer;
    }
  }

  virtual void report(uint64_t for_ns,
                      client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {
    int64_t conn_open = 0;
    int64_t conn_accept = 0;
    int64_t conn_est = 0;
    Common::Stats::MinMaxAvgCount<uint64_t> bytes_sent;
    Common::Stats::MinMaxAvgCount<uint64_t> bytes_recv;

    bool secure = false;
    _do_layer:
    for(auto& addr : m_addresses[secure]) {
      DB::Cell::KeyVec key;
      key.reserve(parent_key.size() + 3);
      key.copy(parent_key);
      key.add("net");
      key.add(addr->addr.to_string());
      key.add(secure ? "SECURE" : "PLAIN");

      DB::Cells::Cell cell;
      cell.flag = DB::Cells::INSERT;
      cell.set_time_order_desc(true);
      cell.set_timestamp(for_ns);
      cell.key.add(key);

      conn_open = addr->conn_open.load();
      size_t sz = 2 + Serialization::encoded_length_vi64(conn_open);
      if((conn_accept = addr->conn_accept.exchange(0)))
        sz += 2 + Serialization::encoded_length_vi64(conn_accept);
      if((conn_est = addr->conn_est.exchange(0)))
        sz += 2 + Serialization::encoded_length_vi64(conn_est);

      addr->bytes_sent.gather(bytes_sent);
      sz += encoded_length(bytes_sent, true);
      addr->bytes_recv.gather(bytes_recv);
      sz += encoded_length(bytes_recv, true);

      std::vector<int64_t> counts(addr->commands.size());
      auto it = counts.begin();
      for(auto& c : addr->commands) {
        if((*it = c.exchange(0)))
          sz += 2 + Serialization::encoded_length_vi64(*it);
        ++it;
      }

      DB::Cell::Serial::Value::FieldsWriter wfields;
      wfields.ensure(sz);

      wfields.add(FIELD_CONN_OPEN,  conn_open);
      if(conn_accept) {
        wfields.add(FIELD_CONN_ACC, conn_accept);
      }
      if(conn_est) {
        wfields.add(FIELD_CONN_EST, conn_est);
      }
      add_value(bytes_sent, FIELD_BYTES_SENT_MIN, true, wfields);
      add_value(bytes_recv, FIELD_BYTES_RECV_MIN, true, wfields);
      for(size_t i=0; i < counts.size(); ++i) {
        if(counts[i])
          wfields.add(i + FIELD_EV_COMMAND_START, counts[i]);
      }
      cell.set_value(wfields.base, wfields.fill(), false);
      colp->add(cell);
    }

    if(!secure) {
      secure = true;
      if(!m_addresses[secure].empty())
        goto _do_layer;
    }
  }

  virtual void reset() override {
    bool secure = false;
    _do_layer:
      for(auto& addr : m_addresses[secure]) {
        addr->conn_open.store(0);
        addr->conn_accept.store(0);
        addr->conn_est.store(0);
        addr->bytes_sent.reset();
        addr->bytes_recv.reset();
        for(auto& c : addr->commands) {
          c.store(0);
        }
      }
    if(!secure) {
      secure = true;
      if(!m_addresses[secure].empty())
        goto _do_layer;
    }
  }


  protected:
  std::vector<std::unique_ptr<Addr>> m_addresses[2];
};



class Item_Mem : public Base {
  /* Cell-Serialization:
      key:    [levels, "mem"]
      value:  {FIELD_ID}:I:{value}, ...
  */

  public:
  typedef std::unique_ptr<Item_Mem> Ptr;

  static const uint8_t FIELD_RSS_FREE_MIN = 0;
  static const uint8_t FIELD_RSS_FREE_MAX = 1;
  static const uint8_t FIELD_RSS_FREE_AVG = 2;

  static const uint8_t FIELD_RSS_USED_MIN = 3;
  static const uint8_t FIELD_RSS_USED_MAX = 4;
  static const uint8_t FIELD_RSS_USED_AVG = 5;

  static const uint8_t FIELD_RSS_USED_REG_MIN = 6;
  static const uint8_t FIELD_RSS_USED_REG_MAX = 7;
  static const uint8_t FIELD_RSS_USED_REG_AVG = 8;

  Common::Stats::MinMaxAvgCount_Safe<uint64_t>  rss_free;
  Common::Stats::MinMaxAvgCount_Safe<uint64_t>  rss_used;
  Common::Stats::MinMaxAvgCount_Safe<uint64_t>  rss_used_reg;

  Item_Mem() noexcept { }

  virtual ~Item_Mem() { }

  virtual void definitions(client::Query::Update::Handlers::Base::Column* colp,
                           const DB::Cell::KeyVec& parent_key) override {
    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add("mem");

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.key.add(key);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(177);

    wfields.add(FIELD_RSS_FREE_MIN, "RSS Free Minimal", 16);
    wfields.add(FIELD_RSS_FREE_MAX, "RSS Free Maximal", 16);
    wfields.add(FIELD_RSS_FREE_AVG, "RSS Free Average", 16);

    wfields.add(FIELD_RSS_USED_MIN, "RSS Used Minimal", 16);
    wfields.add(FIELD_RSS_USED_MAX, "RSS Used Maximal", 16);
    wfields.add(FIELD_RSS_USED_AVG, "RSS Used Average", 16);

    wfields.add(FIELD_RSS_USED_REG_MIN, "RSS Registred Minimal", 21);
    wfields.add(FIELD_RSS_USED_REG_MAX, "RSS Registred Maximal", 21);
    wfields.add(FIELD_RSS_USED_REG_AVG, "RSS Registred Average", 21);

    cell.set_value(DB::Types::Encoder::ZSTD ,wfields.base, wfields.fill());
    colp->add(cell);
  }

  virtual void report(uint64_t for_ns, client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {
    Common::Stats::MinMaxAvgCount<uint64_t> _rss_free;
    rss_free.gather(_rss_free);

    Common::Stats::MinMaxAvgCount<uint64_t> _rss_used;
    rss_used.gather(_rss_used);

    Common::Stats::MinMaxAvgCount<uint64_t> _rss_used_reg;
    rss_used_reg.gather(_rss_used_reg);

    size_t sz = encoded_length(_rss_free, false);
    sz += encoded_length(_rss_used, false);
    sz += encoded_length(_rss_used_reg, false);
    if(!sz)
      return;

    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add("mem");

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.set_timestamp(for_ns);
    cell.key.add(key);


    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(sz);
    add_value(_rss_free,      FIELD_RSS_FREE_MIN,     false, wfields);
    add_value(_rss_used,      FIELD_RSS_USED_MIN,     false, wfields);
    add_value(_rss_used_reg,  FIELD_RSS_USED_REG_MIN, false, wfields);

    cell.set_value(wfields.base, wfields.fill(), false);
    colp->add(cell);
  }

  virtual void reset() override {
    rss_free.reset();
    rss_used.reset();
    rss_used_reg.reset();
  }

};



class Item_CPU : public Base {
  /* Cell-Serialization:
      key:    [levels, "cpu"]
      value:  {FIELD_ID}:I:{value}, ...
  */

  public:
  typedef std::unique_ptr<Item_CPU> Ptr;

  static const uint8_t FIELD_CPU_U_PERC_MIN = 0;
  static const uint8_t FIELD_CPU_U_PERC_MAX = 1;
  static const uint8_t FIELD_CPU_U_PERC_AVG = 2;

  static const uint8_t FIELD_CPU_S_PERC_MIN = 3;
  static const uint8_t FIELD_CPU_S_PERC_MAX = 4;
  static const uint8_t FIELD_CPU_S_PERC_AVG = 5;

  static const uint8_t FIELD_NTHREADS_MIN   = 6;
  static const uint8_t FIELD_NTHREADS_MAX   = 7;
  static const uint8_t FIELD_NTHREADS_AVG   = 8;

  Common::Stats::MinMaxAvgCount_Safe<uint64_t>  percent_user;
  Common::Stats::MinMaxAvgCount_Safe<uint64_t>  percent_sys;
  Common::Stats::MinMaxAvgCount_Safe<uint64_t>  nthreads;

  Item_CPU() noexcept { }

  virtual ~Item_CPU() { }

  virtual void definitions(client::Query::Update::Handlers::Base::Column* colp,
                           const DB::Cell::KeyVec& parent_key) override {
    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add("cpu");

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.key.add(key);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(150);

    wfields.add(FIELD_CPU_U_PERC_MIN, "User %m Minimal", 15);
    wfields.add(FIELD_CPU_U_PERC_MAX, "User %m Maximal", 15);
    wfields.add(FIELD_CPU_U_PERC_AVG, "User %m Average", 15);

    wfields.add(FIELD_CPU_S_PERC_MIN, "Sys %m Minimal", 14);
    wfields.add(FIELD_CPU_S_PERC_MAX, "Sys %m Maximal", 14);
    wfields.add(FIELD_CPU_S_PERC_AVG, "Sys %m Average", 14);

    wfields.add(FIELD_NTHREADS_MIN, "Threads Minimal", 15);
    wfields.add(FIELD_NTHREADS_MAX, "Threads Maximal", 15);
    wfields.add(FIELD_NTHREADS_AVG, "Threads Average", 15);

    cell.set_value(DB::Types::Encoder::ZSTD ,wfields.base, wfields.fill());
    colp->add(cell);
  }

  virtual void report(uint64_t for_ns,
                      client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {

    Common::Stats::MinMaxAvgCount<uint64_t> _percent_user;
    percent_user.gather(_percent_user);

    Common::Stats::MinMaxAvgCount<uint64_t> _percent_sys;
    percent_sys.gather(_percent_sys);

    Common::Stats::MinMaxAvgCount<uint64_t> _nthreads;
    nthreads.gather(_nthreads);

    size_t sz = encoded_length(_percent_user, false);
    sz += encoded_length(_percent_sys, false);
    sz += encoded_length(_nthreads, false);
    if(!sz)
      return;

    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 1);
    key.copy(parent_key);
    key.add("cpu");

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.set_timestamp(for_ns);
    cell.key.add(key);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(sz);
    add_value(_percent_user,  FIELD_CPU_U_PERC_MIN, false, wfields);
    add_value(_percent_sys,   FIELD_CPU_S_PERC_MIN, false, wfields);
    add_value(_nthreads,      FIELD_NTHREADS_MIN,   false, wfields);

    cell.set_value(wfields.base, wfields.fill(), false);
    colp->add(cell);
  }

  virtual void reset() override {
    percent_user.reset();
    percent_sys.reset();
    nthreads.reset();
  }

};



class Item_FS : public Base {
  /* Cell-Serialization:
      key:    [levels, "fs", {type}]
      value:  (FIELD_{type} + FS_{cmd}):I:{value}, ...
  */

  public:

  static const uint8_t FIELD_FDS    = 0;
  static const uint8_t FIELD_MIN    = 1;
  static const uint8_t FIELD_MAX    = 1 + FS::Statistics::Command::MAX * 1;
  static const uint8_t FIELD_AVG    = 1 + FS::Statistics::Command::MAX * 2;
  static const uint8_t FIELD_ERROR  = 1 + FS::Statistics::Command::MAX * 3;
  static const uint8_t FIELD_COUNT  = 1 + FS::Statistics::Command::MAX * 4;

  typedef std::unique_ptr<Item_FS> Ptr;
  FS::FileSystem::Ptr              fs;

  Item_FS(const FS::FileSystem::Ptr& fs) noexcept : fs(fs) { }

  virtual ~Item_FS() { }

  virtual void definitions(client::Query::Update::Handlers::Base::Column* colp,
                           const DB::Cell::KeyVec& parent_key) override {
    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 2);
    key.copy(parent_key);
    key.add("fs");
    key.add(FS::to_string(fs->get_type()));

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.key.add(key);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(FS::Statistics::Command::MAX * 200);

    wfields.add(FIELD_FDS, "Open FDs", 8);
    for(uint8_t i=0; i < FS::Statistics::Command::MAX; ++i) {
      std::string cmd(FS::Statistics::to_string(FS::Statistics::Command(i)));
      wfields.add(FIELD_MIN + i, cmd + " Latency ns Minimum");
      wfields.add(FIELD_MAX + i, cmd + " Latency ns Maximum");
      wfields.add(FIELD_AVG + i, cmd + " Latency ns Average");
      wfields.add(FIELD_ERROR + i, cmd + " Errors Count");
      wfields.add(FIELD_COUNT + i, cmd + " Requests Count");
    }
    cell.set_value(DB::Types::Encoder::ZSTD ,wfields.base, wfields.fill());
    colp->add(cell);
  }

  virtual void report(uint64_t for_ns,
                      client::Query::Update::Handlers::Base::Column* colp,
                      const DB::Cell::KeyVec& parent_key) override {
    FS::Statistics stats(true);
    fs->statistics.gather(stats);
    uint64_t open_fds = fs->statistics.fds_count.load();

    size_t sz = open_fds
      ? (2 + Serialization::encoded_length_vi64(open_fds))
      : 0;
    for(uint8_t cmd=0; cmd < FS::Statistics::Command::MAX; ++cmd) {
      auto& m = stats.metrics[cmd];
      if(m.m_count) {
        uint64_t avg = m.m_total/m.m_count;
        if(avg != m.m_min)
          sz += 2 + Serialization::encoded_length_vi64(m.m_min);
        if(avg != m.m_max)
          sz += 2 + Serialization::encoded_length_vi64(m.m_max);
        sz += 2 + Serialization::encoded_length_vi64(avg);
        if(m.m_error)
          sz += 2 + Serialization::encoded_length_vi64(m.m_error);
        sz += 2 + Serialization::encoded_length_vi64(m.m_count);
      }
    }
    if(!sz)
      return;

    DB::Cell::KeyVec key;
    key.reserve(parent_key.size() + 2);
    key.copy(parent_key);
    key.add("fs");
    key.add(FS::to_string(fs->get_type()));

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);
    cell.set_timestamp(for_ns);
    cell.key.add(key);

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(sz);

    if(open_fds)
      wfields.add(FIELD_FDS, int64_t(open_fds));

    for(uint8_t cmd=0; cmd < FS::Statistics::Command::MAX; ++cmd) {
      auto& m = stats.metrics[cmd];
      if(m.m_count) {
        uint64_t avg = m.m_total/m.m_count;
        if(avg != m.m_min)
          wfields.add(FIELD_MIN + cmd,  int64_t(m.m_min));
        if(avg != m.m_max)
          wfields.add(FIELD_MAX + cmd,  int64_t(m.m_max));
        wfields.add(FIELD_AVG + cmd,  int64_t(avg));
        if(m.m_error)
          wfields.add(FIELD_ERROR + cmd,  int64_t(m.m_error));
        wfields.add(FIELD_COUNT + cmd,  int64_t(m.m_count));
      }
    }

    cell.set_value(wfields.base, wfields.fill(), false);
    colp->add(cell);
  }

  virtual void reset() override {
    fs->statistics.reset();
  }

};



class Reporting : public client::Query::Update::Handlers::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval)
      : client::Query::Update::Handlers::Metric::Reporting(io, cfg_intval),
        system(this), cpu(nullptr), mem(nullptr) {
  }

  virtual Level* configure(const char* group_name, const char* inst_name,
                           const char* host, const Comm::EndPoints& endpoints) {
    auto level = host ? get_level(host) : nullptr;
    if(group_name)
      level = level ? level->get_level(group_name) : get_level(group_name);
    if(inst_name)
      level = level ? level->get_level(inst_name) : get_level(inst_name);
    if(!endpoints.empty()) {
      std::string port(std::to_string(endpoints.front().port()));
      level = level ? level->get_level(port.c_str()) : get_level(port.c_str());
    }
    SWC_ASSERT(level);

    level->metrics.emplace_back(cpu = system.cpu);
    level->metrics.emplace_back(mem = system.mem);
    return level;
  }

  virtual ~Reporting() { }


  struct System final : SWC::System::Notifier {
    Reporting* reporting;
    Item_CPU*  cpu;
    Item_Mem*  mem;

    System(Reporting* reporting)
          : reporting(reporting), cpu(new Item_CPU()), mem(new Item_Mem()) { }

    void rss_used_reg(size_t bytes) noexcept override {
      mem->rss_used_reg.add(bytes / 1048576);
    }

    void rss_free(size_t bytes) noexcept override {
      mem->rss_free.add(bytes / 1048576);
    }

    void rss_used(size_t bytes) noexcept override {
      mem->rss_used.add(bytes / 1048576);
    }

    void cpu_user(size_t perc_milli) noexcept override {
      cpu->percent_user.add(perc_milli);
    }

    void cpu_sys(size_t perc_milli) noexcept override {
      cpu->percent_sys.add(perc_milli);
    }

    void cpu_threads(size_t threads) noexcept override {
      cpu->nthreads.add(threads);
    }

    uint64_t get_cpu_ms_interval() const noexcept override {
      return reporting->cfg_intval->get() * 1000;
    }

  } system;

  Item_CPU* cpu;
  Item_Mem* mem;

};



}}}}} // namespace SWC::Common::Query::Update::Metric


#endif // swcdb_common_queries_update_MetricsReporting_h
